// 接收序列埠值的變數
char cmd;

#include <WiFi.h>
#include <PubSubClient.h> //請先安裝PubSubClient程式庫
#include <Adafruit_NeoPixel.h>
int NEOPin = 32; // 腳位編號
int NEONumber = 1; // 燈組的數量（本例只有一顆）
Adafruit_NeoPixel pixels(NEONumber, NEOPin, NEO_GRB + NEO_KHZ800);
// 設定規格
int Trig = 12; //發出聲波腳位
int Echo = 14; //接收聲波腳位
int buzzer = 21; //蜂鳴器
int relay = 23; //繼電器
bool relayOff = false; // 繼電器狀態標誌

unsigned long previousMillisPurple = 0;
unsigned long previousMillisRed = 0;
unsigned long previousMillisYellow = 0;

const long intervalPurple = 100;
const long intervalRed = 250;
const long intervalYellow = 500;

bool purpleState = false;
bool redState = false;
bool yellowState = false;

void setup() {
  Serial.begin(115200);
  pinMode(Trig, OUTPUT);
  pinMode(Echo, INPUT);
  pinMode(buzzer, OUTPUT);
  pinMode(18, OUTPUT);//GPIO 4輸出RGB LED的紅色(顯示用)
  pinMode(17, OUTPUT);//GPIO 2輸出RGB LED的綠色(顯示用)
  pinMode(16, OUTPUT);//GPIO 15輸出RGB LED的藍色(顯示用)
  pinMode(23, OUTPUT);//GPIO 23輸出繼電器訊號源
  pixels.begin(); // 初始化NeoPixel
}

void loop() {
  digitalWrite(Trig, LOW); //先關閉
  delayMicroseconds(5);
  digitalWrite(Trig, HIGH);//啟動超音波
  delayMicroseconds(10);
  digitalWrite(Trig, LOW); //關閉
  float EchoTime = pulseIn(Echo, HIGH); //計算傳回時間
  float CMValue = EchoTime / 29.4 / 2; //將時間轉換成距離
  Serial.println(CMValue);
  delay(50);

  unsigned long currentMillis = millis();

  if (CMValue <= 5) {
    //碰撞距離紫燈
    if (currentMillis - previousMillisPurple >= intervalPurple) {
      previousMillisPurple = currentMillis;
      purpleState = !purpleState;
      if (purpleState) {
        pixels.setPixelColor(0, pixels.Color(200, 0, 255)); // Purple
      } else {
        pixels.setPixelColor(0, pixels.Color(0, 0, 0)); // Off
      }
      pixels.show();

      // Update the RGB LED
      if (purpleState) {
        analogWrite(16, 240);
        analogWrite(17, 32);
        analogWrite(18, 160);
      } else {
        analogWrite(16, 0);
        analogWrite(17, 0);
        analogWrite(18, 0);
      }
    }
    tone(buzzer, 4186, 100);
    digitalWrite(relay, LOW);
    relayOff = true; // 將繼電器狀態標誌設為關閉
  }
  else if (CMValue <= 15 && CMValue > 5) {
    //警告距離紅燈
    if (currentMillis - previousMillisRed >= intervalRed) {
      previousMillisRed = currentMillis;
      redState = !redState;
      if (redState) {
        pixels.setPixelColor(0, pixels.Color(255, 0, 0)); // Red
      } else {
        pixels.setPixelColor(0, pixels.Color(0, 0, 0)); // Off
      }
      pixels.show();

      // Update the RGB LED
      if (redState) {
        analogWrite(16, 0);
        analogWrite(17, 0);
        analogWrite(18, 255);
      } else {
        analogWrite(16, 0);
        analogWrite(17, 0);
        analogWrite(18, 0);
      }
    }
    tone(buzzer, 523, 100);
    if (!relayOff) { // 如果繼電器狀態標誌未設為關閉，則關閉繼電器
      digitalWrite(relay, LOW); 
      relayOff = true; // 將繼電器狀態標誌設為關閉
    }
  }
  else if (CMValue <= 30 && CMValue > 15) {
    //注意距離黃燈
    if (currentMillis - previousMillisYellow >= intervalYellow) {
      previousMillisYellow = currentMillis;
      yellowState = !yellowState;
      if (yellowState) {
        pixels.setPixelColor(0, pixels.Color(175, 255, 0)); // Yellow
      } else {
        pixels.setPixelColor(0, pixels.Color(0, 0, 0)); // Off
      }
      pixels.show();

      // Update the RGB LED
      if (yellowState) {
        analogWrite(16, 0);
        analogWrite(17, 192);
        analogWrite(18, 255);
      } else {
        analogWrite(16, 0);
        analogWrite(17, 0);
        analogWrite(18, 0);
      }
    }
    tone(buzzer, 253, 100);
    if (relayOff) { // 如果繼電器狀態標誌設為關閉，則打開繼電器
      digitalWrite(relay, HIGH);
      relayOff = false; // 將繼電器狀態標誌設為開啟
    }
  }
  else if (CMValue <= 50 && CMValue > 30) {
    //安全距離綠燈
    pixels.setPixelColor(0, pixels.Color(0, 255, 0)); // 綠色
    pixels.show();
    analogWrite(16, 0);
    analogWrite(17, 255);
    analogWrite(18, 0);
    if (relayOff) { // 如果繼電器狀態標誌設為關閉，則打開繼電器
      digitalWrite(relay, HIGH);
      relayOff = false; // 將繼電器狀態標誌設為開啟
    }
  }
  else {
    // 當距離大於50時關閉NEO
    pixels.setPixelColor(0, pixels.Color(0, 0, 0)); // 關閉
    pixels.show();
    analogWrite(16, 0);
    analogWrite(17, 0);
    analogWrite(18, 0);
    if (relayOff) { // 如果繼電器狀態標誌設為關閉，則打開繼電器
      digitalWrite(relay, HIGH);
      relayOff = false; // 將繼電器狀態標誌設為開啟
    }
  }
  delay(100);
  Serial.print("relayOff= ");
  Serial.println(relayOff);
}
