
#include <esp_now.h>
#include <WiFi.h>
#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

const char* deviceName = "leftLeg"; // device identifier
uint8_t receiverAddress[] = {0x00, 0x4B, 0x12, 0xEF, 0x1B, 0x44}; // receiver MAC (replace)
#define I2C_SDA 21
#define I2C_SCL 22
const unsigned long SEND_INTERVAL_MS = 50; // sending interval

// ---- MPU object & calibration ----
Adafruit_MPU6050 mpu;

// Continuous calibration offsets
float calibAccX = 0.0f, calibAccY = 0.0f, calibAccZ = 0.0f;
float calibGyroX = 0.0f, calibGyroY = 0.0f, calibGyroZ = 0.0f;

// Low-pass coefficient for adaptive calibration (tune for responsiveness vs stability)
const float alpha = 0.01f; // smaller => slower adaptation

// Control flags
volatile bool startSending = false;
unsigned long counterValue = 1;
unsigned long lastSendMs = 0;

// ---- Utility: format MAC ----
static void macToStr(const uint8_t *mac, char *out, size_t outLen) {
  snprintf(out, outLen, "%02X:%02X:%02X:%02X:%02X:%02X",
           mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}

// ---- ESP-NOW callbacks (Corrected for your version) ----
void onSent(const wifi_tx_info_t *info, esp_now_send_status_t status) {
  char macStr[18];
  
  // استفاده از des_addr به جای dest_addr
  macToStr(info->des_addr, macStr, sizeof(macStr)); 

  Serial.printf("SEND status to %s -> %s\n", macStr,
                status == ESP_NOW_SEND_SUCCESS ? "SUCCESS" : "FAILED");
}

// When we receive messages (e.g., "start") from receiver
void onReceive(const esp_now_recv_info *info, const uint8_t *data, int len) {
  // copy into C-string safely
  char msg[len + 1];
  memcpy(msg, data, len);
  msg[len] = '\0';

  if (strcmp(msg, "start") == 0) {
    startSending = true;
    Serial.println("Received 'start' command. Beginning data transmit.");
  } else {
    Serial.printf("Received unknown command: %s\n", msg);
  }
}

// ---- Setup ----
void setup() {
  Serial.begin(115200);
  delay(100);

  WiFi.mode(WIFI_STA); // required for ESP-NOW

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init failed!");
    while (true) delay(1000);
  }

  esp_now_register_send_cb(onSent);
  esp_now_register_recv_cb(onReceive);

  // add receiver as peer
  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, receiverAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  esp_err_t r = esp_now_add_peer(&peerInfo);
  if (r != ESP_OK && r != ESP_ERR_ESPNOW_EXIST) {
    Serial.printf("Warning: esp_now_add_peer returned %d\n", r);
  }

  // I2C & MPU init
  Wire.begin(I2C_SDA, I2C_SCL);
  if (!mpu.begin(0x68, &Wire)) {
    Serial.println("MPU6050 not found. Check wiring.");
    while (true) delay(1000);
  }

  // Configure ranges/filter to match sender expectations
  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

  // initialize calibration offsets to first sample to avoid large spikes
  sensors_event_t a0, g0, t0;
  mpu.getEvent(&a0, &g0, &t0);
  calibAccX = a0.acceleration.x;
  calibAccY = a0.acceleration.y;
  calibAccZ = a0.acceleration.z;
  calibGyroX = g0.gyro.x;
  calibGyroY = g0.gyro.y;
  calibGyroZ = g0.gyro.z;

  Serial.println("Sender ready. Waiting for 'start' via ESP-NOW.");
}

// ---- Main loop ----
void loop() {
  // We only send after receiving "start"
  if (!startSending) {
    delay(10);
    return;
  }

  unsigned long now = millis();
  if (now - lastSendMs < SEND_INTERVAL_MS) return;
  lastSendMs = now;

  sensors_event_t a, g, t;
  mpu.getEvent(&a, &g, &t);

  // Update adaptive offsets (low-pass) — this allows offsets to slowly follow long-term bias
  calibAccX += alpha * (a.acceleration.x - calibAccX);
  calibAccY += alpha * (a.acceleration.y - calibAccY);
  calibAccZ += alpha * (a.acceleration.z - calibAccZ);

  calibGyroX += alpha * (g.gyro.x - calibGyroX);
  calibGyroY += alpha * (g.gyro.y - calibGyroY);
  calibGyroZ += alpha * (g.gyro.z - calibGyroZ);


// Calibrated readings (raw - offset)
  float ax = a.acceleration.x - calibAccX;
  float ay = a.acceleration.y - calibAccY;
  float az = a.acceleration.z - calibAccZ;

  float gx = g.gyro.x - calibGyroX;
  float gy = g.gyro.y - calibGyroY;
  float gz = g.gyro.z - calibGyroZ;

  // Build JSON payload (compact)
  char payload[256];
  int n = snprintf(payload, sizeof(payload),
                   "{\"device\":\"%s\",\"count\":%lu,"
                   "\"acc\":{\"x\":%.6f,\"y\":%.6f,\"z\":%.6f},"
                   "\"gyro\":{\"x\":%.6f,\"y\":%.6f,\"z\":%.6f}}",
                   deviceName, counterValue,
                   ax, ay, az,
                   gx, gy, gz);

  if (n <= 0 || n >= (int)sizeof(payload)) {
    Serial.println("Payload formatting error.");
    return;
  }

  // Send via ESP-NOW
  esp_err_t res = esp_now_send(receiverAddress, (uint8_t *)payload, strlen(payload));
  if (res != ESP_OK) {
    Serial.printf("esp_now_send returned error %d\n", res);
  }

  // Print local log
  Serial.print("SENT: ");
  Serial.println(payload);

  counterValue++;
}
