// 接收序列埠值的變數
char cmd;

#include <WiFi.h>
#include <PubSubClient.h> //請先安裝PubSubClient程式庫

// 設定啟動或停止馬達的參數
// 一開始先設定成「停止」
boolean run = false;

// 左馬達控制設定
const byte LEFT1 = 2;
const byte LEFT2 = 15;
const byte LEFT_PWM = 4;

// 右馬達控制設定
const byte RIGHT1 = 13;
const byte RIGHT2 = 14;
const byte RIGHT_PWM = 12;

// ------ 以下修改成你自己的WiFi帳號密碼 ------
char ssid[] = "C220MIS";
char password[] = "misc220c220";

// &取紙器=取得變數在記憶體中的位置 *指向氣=指向記憶體開始的位置------ 以下修改成你MQTT設定 ------
char* MQTTServer = "mqttgo.io";//免註冊MQTT伺服器
int MQTTPort = 1883;//MQTT Port
char* MQTTUser = "";//不須帳密
char* MQTTPassword = "";//不須帳密
//推播主題1:往前
char* MQTTSubTopic1 = "car/carcar/forward";
//推播主題2:往後
char* MQTTSubTopic2 = "car/carcar/back";
//推播主題3:往左
char* MQTTSubTopic3 = "car/carcar/left";
//推播主題4:往右
char* MQTTSubTopic4 = "car/carcar/right";
//推播主題5:停止
char* MQTTSubTopic5 = "car/carcar/stop";

//long MQTTLastPublishTime;//此變數用來記錄推播時間
//long MQTTPublishInterval = 10000;//每10秒推撥一次
WiFiClient WifiClient;
PubSubClient MQTTClient(WifiClient);

// 設定PWM輸出值（註：FA-130馬達供電不要超過3v）
byte motorSpeed = 200;

void forward() {  // 馬達轉向：前進
  Serial.println("往前");
  digitalWrite(LEFT1, HIGH);
  digitalWrite(LEFT2, LOW);
  digitalWrite(RIGHT1, HIGH);
  digitalWrite(RIGHT2, LOW);
}

void backward() {  // 馬達轉向：後退
  Serial.println("後退");
  digitalWrite(LEFT1, LOW);
  digitalWrite(LEFT2, HIGH);
  digitalWrite(RIGHT1, LOW);
  digitalWrite(RIGHT2, HIGH);
}

void turnLeft() {  // 馬達轉向：左轉
  Serial.println("左轉");
  digitalWrite(LEFT1, LOW);
  digitalWrite(LEFT2, HIGH);
  digitalWrite(RIGHT1, HIGH);
  digitalWrite(RIGHT2, LOW);
}

void turnRight() {  // 馬達轉向：右轉
  Serial.println("右轉");
  digitalWrite(LEFT1, HIGH);
  digitalWrite(LEFT2, LOW);
  digitalWrite(RIGHT1, LOW);
  digitalWrite(RIGHT2, HIGH);
}

void stop() {  // 停止
  Serial.println("停止");
  digitalWrite(LEFT1, LOW);
  digitalWrite(LEFT2, LOW);
  digitalWrite(RIGHT1, LOW);
  digitalWrite(RIGHT2, LOW);
}

//開始WiFi連線
void WifiConnecte() {
  //開始WiFi連線
  WiFi.begin(ssid, password);
 int tryCount=0;
 
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    if (tryCount ++ >=20){
      ESP.restart();
    }
  }
  Serial.println("WiFi連線成功");
  Serial.print("IP Address:");
  Serial.println(WiFi.localIP());
}

//開始MQTT連線
void MQTTConnecte() {
  MQTTClient.setServer(MQTTServer, MQTTPort);
  MQTTClient.setCallback(MQTTCallback);
  while (!MQTTClient.connected()) {
    //以亂數為ClietID
    String  MQTTClientid = "esp32-" + String(random(1000000, 9999999));
    if (MQTTClient.connect(MQTTClientid.c_str(), MQTTUser, MQTTPassword)) {
      //連結成功，顯示「已連線」。
      Serial.println("MQTT已連線");
      //訂閱SubTopic1主題
      MQTTClient.subscribe(MQTTSubTopic1);
      MQTTClient.subscribe(MQTTSubTopic2);
      MQTTClient.subscribe(MQTTSubTopic3);
      MQTTClient.subscribe(MQTTSubTopic4);
      MQTTClient.subscribe(MQTTSubTopic5);
     

    } else {
      //若連線不成功，則顯示錯誤訊息，並重新連線
      Serial.print("MQTT連線失敗,狀態碼=");
      Serial.println(MQTTClient.state());
      Serial.println("五秒後重新連線");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(LEFT1, OUTPUT);
  pinMode(LEFT2, OUTPUT);
  pinMode(LEFT_PWM, OUTPUT);
  pinMode(RIGHT1, OUTPUT);
  pinMode(RIGHT2, OUTPUT);
  pinMode(RIGHT_PWM, OUTPUT);
   //開始WiFi連線
  WifiConnecte();

  //開始MQTT連線
  MQTTConnecte();
}

void loop() {
    //如果WiFi連線中斷，則重啟WiFi連線
  if (WiFi.status() != WL_CONNECTED) WifiConnecte();

  //如果MQTT連線中斷，則重啟MQTT連線
  if (!MQTTClient.connected())  MQTTConnecte();
 
  /*
  if (Serial.available() > 0) {
    cmd = Serial.read();

    switch (cmd) {
      case 'w':  // 接收到'w'，前進。
        forward();
        run = true;  // 啟動馬達
        break;
      case 'x':  // 接收到'x'，後退。
        backward();
        run = true;  // 啟動馬達
        break;
      case 'a':  // 接收到'a'，左轉。
        turnLeft();
        run = true;  // 啟動馬達
        break;
      case 'd':  // 接收到'd'，右轉。
        turnRight();
        run = true;  // 啟動馬達
        break;
      case 's':
        stop();
        run = false;  // 停止馬達
        break;
    }
  }

  if (run) {
    // 如果要啟動馬達…
    // 向馬達輸出指定的類比電壓值
    analogWrite(LEFT_PWM, motorSpeed);
    analogWrite(RIGHT_PWM, motorSpeed);
    Serial.println("100");
  } else {
    // 否則…
    // 將馬達的電壓值設定成0
    analogWrite(LEFT_PWM, 0);
    analogWrite(RIGHT_PWM, 0);
    Serial.println("0");
  }
  */
}


//接收到訂閱時
void MQTTCallback(char* topic, byte* payload, unsigned int length) {
  Serial.print(topic); Serial.print("訂閱通知:");
  String payloadString;//將接收的payload轉成字串
  //顯示訂閱內容
  for (int i = 0; i < length; i++) {
    payloadString = payloadString + (char)payload[i];
  }
  Serial.println(payloadString);
  //比對主題是否為訂閱主題1
  if (strcmp(topic, MQTTSubTopic1) == 0) {
    Serial.println("往前：" + payloadString);
    if (payloadString == "1") forward();
    if (payloadString == "0") stop();
  }
    if (strcmp(topic, MQTTSubTopic2) == 0) {
    Serial.println("往後：" + payloadString);
    if (payloadString == "1") backward();
    if (payloadString == "0") stop();
  }
    if (strcmp(topic, MQTTSubTopic3) == 0) {
    Serial.println("左轉：" + payloadString);
    if (payloadString == "1") turnLeft();
    if (payloadString == "0") stop();
  }
  if (strcmp(topic, MQTTSubTopic4) == 0) {
    Serial.println("右轉：" + payloadString);
    if (payloadString == "1") turnRight();
    if (payloadString == "0") stop();
  }
  if (strcmp(topic, MQTTSubTopic5) == 0) {
    Serial.println("停止：" + payloadString);
    if (payloadString == "1") stop();
    if (payloadString == "0") stop();
  }
}
