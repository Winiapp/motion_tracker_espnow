#include <esp_now.h>
#include <WiFi.h>

// آدرس MAC گیرنده
uint8_t receiverAddress[] = {0x00, 0x4B, 0x12, 0xEF, 0x1B, 0x44};

// نام فرستنده (برای تشخیص در گیرنده)
const char* senderName = "Sender3";
int counter = 1;

void onSent(const wifi_tx_info_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("وضعیت ارسال: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "✅ موفق" : "❌ ناموفق");
}

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK) {
    Serial.println("❌ خطا در مقداردهی ESP-NOW");
    return;
  }

  esp_now_register_send_cb(onSent);

  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, receiverAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("❌ خطا در افزودن گیرنده");
    return;
  }

  Serial.println("✅ فرستنده 1 آماده است...");
}

void loop() {
  if (counter <= 10000) {
    String msg = String(senderName) + ":" + String(counter);
    esp_now_send(receiverAddress, (uint8_t *)msg.c_str(), msg.length());
    Serial.print("📤 ارسال شد: ");
    Serial.println(msg);
    counter++;
    delay(50);
  }
}
