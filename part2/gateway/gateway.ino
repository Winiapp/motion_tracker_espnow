#include <esp_now.h>
#include <WiFi.h>
#include "BluetoothSerial.h"

BluetoothSerial SerialBT;

// list of sender MAC addresses
uint8_t sender1[] = {0x00, 0x4B, 0x12, 0xEE, 0xA6, 0xC8};
uint8_t sender2[] = {0x08, 0x3A, 0xF2, 0xB6, 0xD1, 0xD0};
uint8_t sender3[] = {0x8C, 0x4F, 0x00, 0x2F, 0xC1, 0x8C};

// called whenever data is received via ESP-NOW
void onReceive(const esp_now_recv_info *info, const uint8_t *data, int len) {
  char msg[len + 1];
  memcpy(msg, data, len);
  msg[len] = '\0';

  char macStr[18];
  snprintf(macStr, sizeof(macStr),
           "%02X:%02X:%02X:%02X:%02X:%02X",
           info->src_addr[0], info->src_addr[1], info->src_addr[2],
           info->src_addr[3], info->src_addr[4], info->src_addr[5]);

  // print to USB serial
  Serial.print("📩 from ");
  Serial.print(macStr);
  Serial.print(" -> ");
  Serial.println(msg);

  // forward data to Bluetooth serial
  SerialBT.print("📩 from ");
  SerialBT.print(macStr);
  SerialBT.print(" -> ");
  SerialBT.println(msg);
}

void setup() {
  Serial.begin(115200);
  SerialBT.begin("Receiver_ESP");  // Bluetooth device name
  WiFi.mode(WIFI_STA);

  // init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init failed");
    return;
  }

  esp_now_register_recv_cb(onReceive);

  // common peer settings
  esp_now_peer_info_t peerInfo = {};
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  // add sender 1
  memcpy(peerInfo.peer_addr, sender1, 6);
  esp_now_add_peer(&peerInfo);

  // add sender 2
  memcpy(peerInfo.peer_addr, sender2, 6);
  esp_now_add_peer(&peerInfo);

  // add sender 3
  memcpy(peerInfo.peer_addr, sender3, 6);
  esp_now_add_peer(&peerInfo);

  Serial.println("Receiver ready, waiting for Bluetooth commands...");
}

void loop() {
  if (SerialBT.available()) {
    String command = SerialBT.readStringUntil('\n');
    command.trim();

    if (command == "START") {
      Serial.println("START received -> sending START to all senders");
      SerialBT.println("START received -> starting data transfer");

      const char *msg = "start";
      esp_now_send(sender1, (uint8_t *)msg, strlen(msg));
      esp_now_send(sender2, (uint8_t *)msg, strlen(msg));
      esp_now_send(sender3, (uint8_t *)msg, strlen(msg));
    }
  }
}
