#include <esp_now.h>
#include <WiFi.h>
#include "BluetoothSerial.h"

BluetoothSerial SerialBT;

// تابعی برای دریافت داده از فرستنده (طبق ESP-IDF v5.5)
void onReceive(const esp_now_recv_info *recvInfo, const uint8_t *data, int len) {
  char msg[len + 1];
  memcpy(msg, data, len);
  msg[len] = '\0';

  // نمایش اطلاعات فرستنده و پیام
  Serial.print("📩 پیام از ");
  for (int i = 0; i < 6; i++) {
    Serial.print(recvInfo->src_addr[i], HEX);
    if (i < 5) Serial.print(":");
  }
  Serial.print(" → ");
  Serial.println(msg);

  // ارسال پیام دریافتی به بلوتوث سریال
  SerialBT.print("📩 از ESP: ");
  SerialBT.println(msg);
}

void setup() {
  Serial.begin(115200);
  SerialBT.begin("Receiver_BT");  // اسم بلوتوث

  WiFi.mode(WIFI_STA);

  // مقداردهی ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("❌ خطا در مقداردهی ESP-NOW");
    return;
  }

  // ثبت تابع دریافت
  esp_now_register_recv_cb(onReceive);

  Serial.println("✅ گیرنده آماده است...");
  SerialBT.println("✅ گیرنده بلوتوث آماده است...");
}

void loop() {
  // چیزی برای انجام نیست، فقط منتظر دریافت پیام می‌ماند
}
