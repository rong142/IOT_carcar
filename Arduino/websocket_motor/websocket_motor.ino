#include <ArduinoWebsockets.h>
#include <WiFi.h>
//--------------Setting------------------------
// 左馬達
const byte LEFT1 = 2;
const byte LEFT2 = 15;
const byte LEFT_PWM = 4;
// 右馬達
const byte RIGHT1 = 13;
const byte RIGHT2 = 14;
const byte RIGHT_PWM = 12;
// 馬達轉速
byte motorSpeed = 255;
const char* ssid = "Uokio"; // 輸入你的WiFi網路名稱
const char* password = "00000000"; // 輸入你的WiFi密碼
const char* websockets_server_host = "websocket.icelike.info"; // 輸入WebSocket伺服器位址
const uint16_t websockets_server_port = 8080; // 輸入WebSocket伺服器埠號
using namespace websockets;
WebsocketsClient client;
String message; // 定義 message 變數
bool connected;
//---------------------------------------------

void setup() {
  Serial.begin(115200);

  // 設定馬達腳位為輸出
  pinMode(LEFT1, OUTPUT);
  pinMode(LEFT2, OUTPUT);
  pinMode(LEFT_PWM, OUTPUT);
  pinMode(RIGHT1, OUTPUT);
  pinMode(RIGHT2, OUTPUT);
  pinMode(RIGHT_PWM, OUTPUT);

  digitalWrite(LEFT1, HIGH);
  digitalWrite(LEFT2, LOW);
  digitalWrite(RIGHT1, HIGH);
  digitalWrite(RIGHT2, LOW);
  analogWrite(LEFT_PWM, 0);
  analogWrite(RIGHT_PWM, 0);

  websocket_connet();
}

void loop() {
  // 讓Websockets客戶端檢查是否有新的訊息
  if (client.available()) {
    client.poll();
  }
  if (connected) { //判斷是否成功連上WS伺服器
    // 根據接收到的訊息來控制馬達
    if (message == "up") forward();
    if (message == "down") backward();
    if (message == "left") turnLeft();
    if (message == "right") turnRight();
    if (message == "pause") stop();
  }
  else {
    websocket_connet(); //若是沒有連接到WS嘗試重新連線
  }

}

void websocket_connet() {
  // 連接WiFi
  WiFi.begin(ssid, password);

  // 等待連接WiFi
  for (int i = 0; i < 10 && WiFi.status() != WL_CONNECTED; i++) {
    Serial.print(".");
    delay(1000);
  }

  // 檢查WiFi連接狀態
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("未連接到WiFi！");
    ESP.restart(); //ESP32重啟
    return;
  }

  Serial.println("已連接到WiFi，正在連接到伺服器。");
  // 嘗試連接到Websockets伺服器
  ::connected = client.connect(websockets_server_host, websockets_server_port, "/");
  if (connected) {
    Serial.println("已連接！");
    client.send("Hello Server"); // 向伺服器發送訊息
  } else {
    Serial.println("未連接！");
  }

  // 當接收到訊息時執行回調函式
  client.onMessage([&](WebsocketsMessage message) {
    Serial.print("收到訊息: ");
    Serial.println(message.data());
    ::message = message.data(); // 儲存接收到的訊息到全域變數
  });
}

void backward() {  // 馬達轉向：後退
  digitalWrite(LEFT1, HIGH);
  digitalWrite(LEFT2, LOW);
  digitalWrite(RIGHT1, HIGH);
  digitalWrite(RIGHT2, LOW);
  analogWrite(LEFT_PWM, motorSpeed);
  analogWrite(RIGHT_PWM, motorSpeed);
}

void forward() {  // 馬達轉向：前進
  digitalWrite(LEFT1, LOW);
  digitalWrite(LEFT2, HIGH);
  digitalWrite(RIGHT1, LOW);
  digitalWrite(RIGHT2, HIGH);
  analogWrite(LEFT_PWM, motorSpeed);
  analogWrite(RIGHT_PWM, motorSpeed);
}

void turnLeft() {  // 馬達轉向：左轉
  Serial.println("左轉");
  digitalWrite(LEFT1, HIGH);
  digitalWrite(LEFT2, LOW);
  digitalWrite(RIGHT1, LOW);
  digitalWrite(RIGHT2, HIGH);
  analogWrite(LEFT_PWM, 0);
  analogWrite(RIGHT_PWM, motorSpeed);
}

void turnRight() {  // 馬達轉向：右轉
  Serial.println("右轉");
  digitalWrite(LEFT1, LOW);
  digitalWrite(LEFT2, HIGH);
  digitalWrite(RIGHT1, HIGH);
  digitalWrite(RIGHT2, LOW);
  analogWrite(LEFT_PWM, motorSpeed);
  analogWrite(RIGHT_PWM, 0);
}

void stop() {  // 停止
  Serial.println("停止");
  digitalWrite(LEFT1, LOW);
  digitalWrite(LEFT2, LOW);
  digitalWrite(RIGHT1, LOW);
  digitalWrite(RIGHT2, LOW);
  analogWrite(LEFT_PWM, 0);
  analogWrite(RIGHT_PWM, 0);
}
