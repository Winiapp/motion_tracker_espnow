#include <esp_now.h>
#include <WiFi.h>
#include "BluetoothSerial.h"

BluetoothSerial SerialBT;

// لیست فرستنده‌ها
uint8_t sender1[] = {0x00, 0x4B, 0x12, 0xEE, 0xA6, 0xC8};
uint8_t sender2[] = {0x08, 0x3A, 0xF2, 0xB6, 0xD1, 0xD0};

bool sendStartSignal = false;

// وقتی از فرستنده داده می‌رسد
void onReceive(const esp_now_recv_info *info, const uint8_t *data, int len) {
  char msg[len + 1];
  memcpy(msg, data, len);
  msg[len] = '\0';

  char macStr[18];
  snprintf(macStr, sizeof(macStr),
           "%02X:%02X:%02X:%02X:%02X:%02X",
           info->src_addr[0], info->src_addr[1], info->src_addr[2],
           info->src_addr[3], info->src_addr[4], info->src_addr[5]);

  Serial.print("📩 از ");
  Serial.print(macStr);
  Serial.print(" → ");
  Serial.println(msg);

  // ارسال به بلوتوث
  SerialBT.print("📩 از ");
  SerialBT.print(macStr);
  SerialBT.print(" → ");
  SerialBT.println(msg);
}

void setup() {
  Serial.begin(115200);
  SerialBT.begin("Receiver_ESP");  // اسم بلوتوث
  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK) {
    Serial.println("❌ خطا در مقداردهی ESP-NOW");
    return;
  }

  esp_now_register_recv_cb(onReceive);

  // افزودن فرستنده‌ها
  esp_now_peer_info_t peerInfo = {};
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  memcpy(peerInfo.peer_addr, sender1, 6);
  esp_now_add_peer(&peerInfo);

  memcpy(peerInfo.peer_addr, sender2, 6);
  esp_now_add_peer(&peerInfo);

  Serial.println("✅ گیرنده آماده است. از طریق بلوتوث دستور بده...");
}

void loop() {
  if (SerialBT.available()) {
    String command = SerialBT.readStringUntil('\n');
    command.trim();

    if (command == "m1") {
      Serial.println("🚀 فرمان 'm1' دریافت شد → ارسال پیام start به فرستنده‌ها");
      SerialBT.println("🚀 فرمان 'm1' دریافت شد → شروع ارسال داده");

      const char *msg = "start";
      esp_now_send(sender1, (uint8_t *)msg, strlen(msg));
      esp_now_send(sender2, (uint8_t *)msg, strlen(msg));
    }
  }
}
