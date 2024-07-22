// 接收序列埠值的變數
char cmd;

#include <WiFi.h>
#include <PubSubClient.h> //請先安裝PubSubClient程式庫

// Ultrasonic sensor pins
int Trig = 16; // 發出聲波腳位
int Echo = 17; // 接收聲波腳位
float minValidDistance = 2.0; // 最小有效距離（單位：cm）
float maxValidDistance = 400.0; // 最大有效距離（單位：cm）

void setup() {
  Serial.begin(115200);
  pinMode(Trig, OUTPUT);
  pinMode(Echo, INPUT);
}

void loop() {
  digitalWrite(Trig, LOW); // 先關閉
  delayMicroseconds(5);
  digitalWrite(Trig, HIGH); // 啟動超音波
  delayMicroseconds(10);
  digitalWrite(Trig, LOW); // 關閉
  float EchoTime = pulseIn(Echo, HIGH); // 計算傳回時間
  float CMValue = EchoTime / 29.4 / 2; // 將時間轉換成距離

  if (CMValue >= minValidDistance && CMValue <= maxValidDistance) {
    Serial.println(CMValue); // 打印距離
  } else {
    Serial.println("Out of range"); // 當距離不在有效範圍內，打印"Out of range"
  }

  delay(50);
}
