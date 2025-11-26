#include <esp_now.h>
#include <WiFi.h>

// Receiver MAC address
uint8_t receiverAddress[] = {0x00, 0x4B, 0x12, 0xEF, 0x1B, 0x44};

bool startSending = false;
int counter = 1;
const char* senderName = "Sender1";  // For the second device, set "Sender2"

// Callback: runs when a message is received from the receiver
void onReceive(const esp_now_recv_info *info, const uint8_t *data, int len) {
  char msg[len + 1];
  memcpy(msg, data, len);
  msg[len] = '\0';

  if (strcmp(msg, "start") == 0) {
    startSending = true;
    Serial.println("✅ START command received → Beginning data transmission");
  }
}

// Send status callback
void onSent(const wifi_tx_info_t *mac_addr, esp_now_send_status_t status) {
  // You can print send status for debugging, but it's not needed now
}

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK) {
    Serial.println("❌ ESP-NOW initialization error");
    return;
  }

  esp_now_register_send_cb(onSent);
  esp_now_register_recv_cb(onReceive);

  // Add receiver peer
  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, receiverAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  esp_now_add_peer(&peerInfo);

  Serial.println("✅ Sender is ready and waiting for the START command...");
}

void loop() {
  if (startSending && counter <= 10000) {
    String msg = String(senderName) + ":" + String(counter);
    esp_now_send(receiverAddress, (uint8_t *)msg.c_str(), msg.length());
    Serial.println("📤 " + msg);
    counter++;
    delay(50);
  }
}
