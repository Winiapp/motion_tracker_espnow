#include <esp_now.h>
#include <WiFi.h>

// MAC address of the receiver device
static const uint8_t kReceiverAddress[6] = { 0x00, 0x4B, 0x12, 0xEF, 0x1B, 0x44 };

// Identifier for this sender (included in outgoing messages)
static const char* kSenderId = "Sender3";

// Message counter
static uint32_t messageCounter = 1;

/**
 * Callback invoked after a message is transmitted.
 */
void onMessageSent(const wifi_tx_info_t* info, esp_now_send_status_t status) {
  const char* result = (status == ESP_NOW_SEND_SUCCESS) ? "Success" : "Failed";
  Serial.printf("[ESP-NOW] Transmission status: %s\n", result);
}

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);

  // Initialize ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("[Error] ESP-NOW initialization failed");
    return;
  }

  esp_now_register_send_cb(onMessageSent);

  // Register receiver peer
  esp_now_peer_info_t peer{};
  memcpy(peer.peer_addr, kReceiverAddress, sizeof(kReceiverAddress));
  peer.channel = 0;
  peer.encrypt = false;

  if (esp_now_add_peer(&peer) != ESP_OK) {
    Serial.println("[Error] Failed to register peer");
    return;
  }

  Serial.println("[System] Sender initialized and ready");
}

void loop() {
  if (messageCounter > 10000) {
    return;
  }

  // Build message in the format: SenderID:Counter
  char payload[32];
  snprintf(payload, sizeof(payload), "%s:%lu", kSenderId, messageCounter);

  // Send the message
  esp_err_t result = esp_now_send(kReceiverAddress, (uint8_t*)payload, strlen(payload));

  if (result == ESP_OK) {
    Serial.printf("[TX] Sent: %s\n", payload);
  } else {
    Serial.printf("[TX Error] Failed to send message: %s\n", payload);
  }

  messageCounter++;
  delay(50);
}
