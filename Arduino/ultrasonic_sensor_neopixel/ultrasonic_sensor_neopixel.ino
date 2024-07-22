// 接收序列埠值的變數
char cmd;

#include <WiFi.h>
#include <PubSubClient.h> //請先安裝PubSubClient程式庫
#include <Adafruit_NeoPixel.h> // 請先安裝 Adafruit NeoPixel 程式庫

// Ultrasonic sensor pins
int Trig = 16; // 發出聲波腳位
int Echo = 17; // 接收聲波腳位
float minValidDistance = 2.0; // 最小有效距離（單位：cm）
float maxValidDistance = 400.0; // 最大有效距離（單位：cm）

// NeoPixel 設置
int NEOPin = 32; // 腳位編號
int NEONumber = 1; // 燈組的數量（本例只有一顆）
Adafruit_NeoPixel pixels(NEONumber, NEOPin, NEO_GRB + NEO_KHZ800);

// Time variables
unsigned long previousMillis = 0; // 儲存上一次偵測的時間
const long interval = 1000; // 間隔時間（單位：毫秒）

void setup() {
  Serial.begin(115200);
  pinMode(Trig, OUTPUT);
  pinMode(Echo, INPUT);
  pixels.begin(); // 初始化NeoPixel
  pixels.setPixelColor(0, pixels.Color(0, 0, 0)); // 關閉 NeoPixel
  pixels.show(); // 顯示顏色
}

void loop() {
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    digitalWrite(Trig, LOW); // 先關閉
    delayMicroseconds(5);
    digitalWrite(Trig, HIGH); // 啟動超音波
    delayMicroseconds(10);
    digitalWrite(Trig, LOW); // 關閉
    float EchoTime = pulseIn(Echo, HIGH); // 計算傳回時間
    float CMValue = EchoTime / 29.4 / 2; // 將時間轉換成距離

    if (CMValue >= minValidDistance && CMValue <= maxValidDistance) {
      Serial.println(CMValue); // 打印距離
      
      if (CMValue <= 10) {
        pixels.setPixelColor(0, pixels.Color(25, 0, 0)); // 調低亮度的紅色
      } else if (CMValue <= 20) {
        pixels.setPixelColor(0, pixels.Color(25, 16, 0)); // 調低亮度的橘色
      } else {
        pixels.setPixelColor(0, pixels.Color(0, 25, 0)); // 調低亮度的綠色
      }
      pixels.show(); // 顯示顏色

    } else {
      Serial.println("Out of range"); // 當距離不在有效範圍內，打印"Out of range"
      pixels.setPixelColor(0, pixels.Color(0, 0, 0)); // 關閉 NeoPixel
      pixels.show(); // 顯示顏色
    }
  }
}
