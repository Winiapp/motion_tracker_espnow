#include <esp_now.h>
#include <WiFi.h>
#include "BluetoothSerial.h"
#include <ArduinoJson.h>

BluetoothSerial SerialBT;


uint8_t sender1[] = {0x00, 0x4B, 0x12, 0xEE, 0xA6, 0xC8};
uint8_t sender2[] = {0x08, 0x3A, 0xF2, 0xB6, 0xD1, 0xD0};

// Utility: convert MAC to string
static void macToStr(const uint8_t *mac, char *out, size_t outLen) {
  snprintf(out, outLen, "%02X:%02X:%02X:%02X:%02X:%02X",
           mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}


void onSent(const wifi_tx_info_t *info, esp_now_send_status_t status) {
  char macStr[18];

  macToStr(info->des_addr, macStr, sizeof(macStr));
  
  Serial.printf("Message sent to %s -> %s\n", macStr,
                status == ESP_NOW_SEND_SUCCESS ? "SUCCESS" : "FAILED");
  SerialBT.printf("Message sent to %s -> %s\n", macStr,
                   status == ESP_NOW_SEND_SUCCESS ? "SUCCESS" : "FAILED");
}

// ESP-NOW receive callback
void onReceive(const esp_now_recv_info *info, const uint8_t *data, int len) {
  // Build mac string
  char macStr[18];
  macToStr(info->src_addr, macStr, sizeof(macStr));

  // Copy payload to String
  String payload;
  payload.reserve(len + 1);
  for (int i = 0; i < len; ++i) payload += (char)data[i];

  // Log raw payload
  Serial.printf("Received from %s -> %s\n", macStr, payload.c_str());
  SerialBT.printf("Received from %s -> %s\n", macStr, payload.c_str());

  // Parse JSON (tune capacity if your payloads grow)
  StaticJsonDocument<512> doc;
  DeserializationError err = deserializeJson(doc, payload);
  if (err) {
    Serial.printf("JSON parse error: %s\n", err.c_str());
    SerialBT.printf("JSON parse error: %s\n", err.c_str());
    return;
  }

  // Extract fields robustly
  const char *device = doc["device"] | "N/A";

  // count may be numeric or string
  String countStr = "N/A";
  if (doc.containsKey("count")) {
    if (doc["count"].is<const char*>()) countStr = String((const char*)doc["count"]);
    // اصلاح شده: اضافه کردن || (OR) بین شرط‌ها
    else if (doc["count"].is<long>() || doc["count"].is<int>()) countStr = String(doc["count"].as<long>());
    else if (doc["count"].is<double>() || doc["count"].is<float>()) countStr = String(doc["count"].as<double>(), 0);
  }

  float accx = 0, accy = 0, accz = 0;
  if (doc.containsKey("acc")) {
    accx = doc["acc"]["x"] | 0.0f;
    accy = doc["acc"]["y"] | 0.0f;
    accz = doc["acc"]["z"] | 0.0f;
  }

  float gyx = 0, gyy = 0, gyz = 0;
  if (doc.containsKey("gyro")) {
    gyx = doc["gyro"]["x"] | 0.0f;
    gyy = doc["gyro"]["y"] | 0.0f;
    gyz = doc["gyro"]["z"] | 0.0f;
  }

  // Pretty print parsed values
  Serial.printf("Parsed -> device: %s | count: %s\n", device, countStr.c_str());
  Serial.printf("  acc:  x=%0.3f, y=%0.3f, z=%0.3f\n", accx, accy, accz);
  Serial.printf("  gyro: x=%0.3f, y=%0.3f, z=%0.3f\n", gyx, gyy, gyz);

  SerialBT.printf("Parsed -> device: %s | count: %s\n", device, countStr.c_str());
  SerialBT.printf("  acc:  x=%0.3f, y=%0.3f, z=%0.3f\n", accx, accy, accz);
  SerialBT.printf("  gyro: x=%0.3f, y=%0.3f, z=%0.3f\n", gyx, gyy, gyz);
}

// Helper: add peer and log outcome
void addPeer(const uint8_t *mac) {
  esp_now_peer_info_t peer = {};
  peer.channel = 0;
  peer.encrypt = false;
  memcpy(peer.peer_addr, mac, 6);
  esp_err_t res = esp_now_add_peer(&peer);
  if (res != ESP_OK && res != ESP_ERR_ESPNOW_EXIST) {
    Serial.printf("Warning: add_peer failed code=%d\n", res);
    SerialBT.printf("Warning: add_peer failed code=%d\n", res);
  }
}

void setup() {
  Serial.begin(115200);
  delay(50);
  SerialBT.begin("Receiver_ESP");
  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init failed.");
    while (true) delay(1000);
  }

  esp_now_register_send_cb(onSent);
  esp_now_register_recv_cb(onReceive);

  // Add known senders so we can send control messages
  addPeer(sender1);
  addPeer(sender2);

  Serial.println("Receiver ready. Send 'm1' via Bluetooth to trigger senders.");
  SerialBT.println("Receiver ready. Send 'm1' to trigger senders.");
}

void loop() {
  if (SerialBT.available()) {
    String command = SerialBT.readStringUntil('\n');
    command.trim();
    if (command == "m1") {
      Serial.println("Command 'm1' received. Sending 'start' to senders.");
      SerialBT.println("Command 'm1' received. Sending 'start' to senders.");
      const char *msg = "start";
      esp_now_send(sender1, (uint8_t*)msg, strlen(msg));
      esp_now_send(sender2, (uint8_t*)msg, strlen(msg));
    } else {
      Serial.print("Unknown Bluetooth command: ");
      Serial.println(command);
      SerialBT.print("Unknown Bluetooth command: ");
      SerialBT.println(command);
    }
  }
  delay(10);
}