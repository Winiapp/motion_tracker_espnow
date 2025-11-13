#include <esp_now.h>
#include <WiFi.h>
#include "BluetoothSerial.h"

BluetoothSerial SerialBT;

// Callback function to receive data from sender (ESP-IDF v5.5)
void onReceive(const esp_now_recv_info *recvInfo, const uint8_t *data, int len) {
  char msg[len + 1];
  memcpy(msg, data, len);
  msg[len] = '\0';

  // Print sender MAC address and message
  Serial.print("Message from ");
  for (int i = 0; i < 6; i++) {
    Serial.print(recvInfo->src_addr[i], HEX);
    if (i < 5) Serial.print(":");
  }
  Serial.print(" -> ");
  Serial.println(msg);

  // Send received message to Bluetooth Serial
  SerialBT.print("From ESP: ");
  SerialBT.println(msg);
}

void setup() {
  Serial.begin(115200);
  SerialBT.begin("Receiver_BT");  // Bluetooth name

  WiFi.mode(WIFI_STA);

  // Initialize ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Register receive callback
  esp_now_register_recv_cb(onReceive);

  Serial.println("Receiver is ready");
  SerialBT.println("Bluetooth receiver is ready");
}

void loop() {
  // No action needed, waiting for messages
}
