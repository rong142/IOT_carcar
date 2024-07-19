

// 接收序列埠值的變數
char cmd;

#include <WiFi.h>
#include <PubSubClient.h> //請先安裝PubSubClient程式庫
#include "esp_camera.h"
#include <ArduinoWebsockets.h>
#include "esp_timer.h"
#include "img_converters.h"
#include "fb_gfx.h"
#include "soc/soc.h" //disable brownout problems
#include "soc/rtc_cntl_reg.h" //disable brownout problems
#include "driver/gpio.h"

// configuration for AI Thinker Camera board
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

//WiFi配置
//motor寫法
//char ssid[] = "C220MIS";
//char password[] = "misc220c220";
//webcam寫法
const char* ssid     = "Uokio"; // CHANGE HERE
const char* password = "00000000"; // CHANGE HERE
//WebSocket配置
const char* websockets_server_host = "192.168.137.1"; //CHANGE HERE
const uint16_t websockets_server_port = 3001; // OPTIONAL CHANGE
camera_fb_t * fb = NULL;
size_t _jpg_buf_len = 0;
uint8_t * _jpg_buf = NULL;
uint8_t state = 0;

using namespace websockets;
WebsocketsClient client;

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

// 設定PWM輸出值（註：FA-130馬達供電不要超過3v）
byte motorSpeed = 255;



// &取紙器=取得變數在記憶體中的位置 *指向氣=指向記憶體開始的位置------ 以下修改成你MQTT設定 ------
char* MQTTServer = "mqttgo.io";//免註冊MQTT伺服器
int MQTTPort = 1883;//MQTT Port
char* MQTTUser = "";//不須帳密
char* MQTTPassword = "";//不須帳密

//推播主題1:往前
char* MQTTSubTopic1_001 = "car/carcar/forward_001";
char* MQTTSubTopic1_002 = "car/carcar/forward_002";
//推播主題2:往後
char* MQTTSubTopic2_001 = "car/carcar/back_001";
char* MQTTSubTopic2_002 = "car/carcar/back_002";
//推播主題3:往左
char* MQTTSubTopic3_001 = "car/carcar/left_001";
char* MQTTSubTopic3_002 = "car/carcar/left_002";
//推播主題4:往右
char* MQTTSubTopic4_001 = "car/carcar/right_001";
char* MQTTSubTopic4_002 = "car/carcar/right_002";
//推播主題5:停止
char* MQTTSubTopic5_001 = "car/carcar/stop_001";
char* MQTTSubTopic5_002 = "car/carcar/stop_002";

WiFiClient WifiClient;
PubSubClient MQTTClient(WifiClient);

void backward() {  // 馬達轉向：後退
  //Serial.println("往前");
  Serial.println("倒退");
  digitalWrite(LEFT1, HIGH);
  digitalWrite(LEFT2, LOW);
  digitalWrite(RIGHT1, HIGH);
  digitalWrite(RIGHT2, LOW);

  analogWrite(LEFT_PWM, motorSpeed);
  analogWrite(RIGHT_PWM, motorSpeed);

  MQTTClient.publish(MQTTSubTopic2_001, String(0).c_str());
  MQTTClient.publish(MQTTSubTopic3_001, String(0).c_str());
  MQTTClient.publish(MQTTSubTopic4_001, String(0).c_str());
  MQTTClient.publish(MQTTSubTopic5_001, String(0).c_str());
}

void forward() {  // 馬達轉向：前進
  //Serial.println("後退");
  Serial.println("向前");
  digitalWrite(LEFT1, LOW);
  digitalWrite(LEFT2, HIGH);
  digitalWrite(RIGHT1, LOW);
  digitalWrite(RIGHT2, HIGH);

  analogWrite(LEFT_PWM, motorSpeed);
  analogWrite(RIGHT_PWM, motorSpeed);

  MQTTClient.publish(MQTTSubTopic1_001, String(0).c_str());
  MQTTClient.publish(MQTTSubTopic3_001, String(0).c_str());
  MQTTClient.publish(MQTTSubTopic4_001, String(0).c_str());
  MQTTClient.publish(MQTTSubTopic5_001, String(0).c_str());

}

void turnRight() {  // 馬達轉向：右轉
  //Serial.println("左轉");
  Serial.println("右轉");
  digitalWrite(LEFT1, LOW);
  digitalWrite(LEFT2, HIGH);
  digitalWrite(RIGHT1, HIGH);
  digitalWrite(RIGHT2, LOW);

  analogWrite(LEFT_PWM, motorSpeed);
  analogWrite(RIGHT_PWM, 0);

  MQTTClient.publish(MQTTSubTopic1_001, String(0).c_str());
  MQTTClient.publish(MQTTSubTopic2_001, String(0).c_str());
  MQTTClient.publish(MQTTSubTopic4_001, String(0).c_str());
  MQTTClient.publish(MQTTSubTopic5_001, String(0).c_str());
}

void turnLeft() {  // 馬達轉向：左轉
  //Serial.println("右轉");
  Serial.println("左轉");
  digitalWrite(LEFT1, HIGH);
  digitalWrite(LEFT2, LOW);
  digitalWrite(RIGHT1, LOW);
  digitalWrite(RIGHT2, HIGH);

  analogWrite(LEFT_PWM, 0);
  analogWrite(RIGHT_PWM, motorSpeed);

  MQTTClient.publish(MQTTSubTopic1_001, String(0).c_str());
  MQTTClient.publish(MQTTSubTopic2_001, String(0).c_str());
  MQTTClient.publish(MQTTSubTopic3_001, String(0).c_str());
  MQTTClient.publish(MQTTSubTopic5_001, String(0).c_str());
}

void stop() {  // 停止
  Serial.println("停止");
  digitalWrite(LEFT1, LOW);
  digitalWrite(LEFT2, LOW);
  digitalWrite(RIGHT1, LOW);
  digitalWrite(RIGHT2, LOW);

  analogWrite(LEFT_PWM, 0);
  analogWrite(RIGHT_PWM, 0);

  MQTTClient.publish(MQTTSubTopic1_001, String(0).c_str());
  MQTTClient.publish(MQTTSubTopic2_001, String(0).c_str());
  MQTTClient.publish(MQTTSubTopic3_001, String(0).c_str());
  MQTTClient.publish(MQTTSubTopic4_001, String(0).c_str());


}

//開始WiFi連線
void WifiConnecte() {
  //開始WiFi連線
  WiFi.begin(ssid, password);
  int tryCount = 0;

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    if (tryCount ++ >= 20) {
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
      MQTTClient.subscribe(MQTTSubTopic1_002);
      MQTTClient.subscribe(MQTTSubTopic2_002);
      MQTTClient.subscribe(MQTTSubTopic3_002);
      MQTTClient.subscribe(MQTTSubTopic4_002);
      MQTTClient.subscribe(MQTTSubTopic5_002);
      MQTTClient.subscribe(MQTTSubTopic1_001);
      MQTTClient.subscribe(MQTTSubTopic2_001);
      MQTTClient.subscribe(MQTTSubTopic3_001);
      MQTTClient.subscribe(MQTTSubTopic4_001);
      MQTTClient.subscribe(MQTTSubTopic5_001);

    } else {
      //若連線不成功，則顯示錯誤訊息，並重新連線
      Serial.print("MQTT連線失敗,狀態碼=");
      Serial.println(MQTTClient.state());
      Serial.println("五秒後重新連線");
      delay(5000);
    }
  }
}

void onMessageCallback(WebsocketsMessage message) {
  Serial.print("Got Message: ");
  Serial.println(message.data());
}

esp_err_t init_camera() {
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  // parameters for image quality and size
  //config.frame_size = FRAMESIZE_VGA; // FRAMESIZE_ + QVGA|CIF|VGA|SVGA|XGA|SXGA|UXGA
  //config.jpeg_quality = 15; //10-63 lower number means higher quality
  //config.fb_count = 2;
  //GPT寫法:
  if (psramFound()) {
    config.frame_size = FRAMESIZE_UXGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_VGA;
    config.jpeg_quality = 15;
    config.fb_count = 2;
  }
  
  
  // Camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("camera init FAIL: 0x%x", err);
    return err;
  }
  sensor_t * s = esp_camera_sensor_get();
  s->set_framesize(s, FRAMESIZE_VGA);
  Serial.println("camera init OK");
  return ESP_OK;
}

esp_err_t init_wifi() {
  WiFi.begin(ssid, password);
  Serial.println("Wifi init ");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi OK");
  Serial.println("connecting to WS: ");
  client.onMessage(onMessageCallback);
  bool connected = client.connect(websockets_server_host, websockets_server_port, "/");
  if (!connected) {
    Serial.println("WS connect failed!");
    Serial.println(WiFi.localIP());
    state = 3;
    return ESP_FAIL;
  }
  if (state == 3) {
    return ESP_FAIL;
  }

  Serial.println("WS OK");
  client.send("hello from ESP32 camera stream!");
  return ESP_OK;
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
  if (strcmp(topic, MQTTSubTopic1_001) == 0) {
    Serial.println("往前：" + payloadString);
    if (payloadString == "1") forward();
    //if (payloadString == "0") stop();
  }
  if (strcmp(topic, MQTTSubTopic2_001) == 0) {
    Serial.println("往後：" + payloadString);
    if (payloadString == "1") backward();
    //if (payloadString == "0") stop();
  }
  if (strcmp(topic, MQTTSubTopic3_001) == 0) {
    Serial.println("左轉：" + payloadString);
    if (payloadString == "1") turnLeft();
    //if (payloadString == "0") stop();
  }
  if (strcmp(topic, MQTTSubTopic4_001) == 0) {
    Serial.println("右轉：" + payloadString);
    if (payloadString == "1") turnRight();
    //if (payloadString == "0") stop();
  }
  if (strcmp(topic, MQTTSubTopic5_001) == 0) {
    Serial.println("停止：" + payloadString);
    if (payloadString == "1") stop();
    //if (payloadString == "0") stop();
  }
  if (strcmp(topic, MQTTSubTopic1_002) == 0) {
    Serial.println("往前：" + payloadString);
    if (payloadString == "1") backward();
    //if (payloadString == "0") stop();
  }
  if (strcmp(topic, MQTTSubTopic2_002) == 0) {
    Serial.println("MQTTSubTopic2_001：" + payloadString);
    if (payloadString == "1") backward();
    //if (payloadString == "0") stop();
  }
  if (strcmp(topic, MQTTSubTopic3_002) == 0) {
    Serial.println("MQTTSubTopic3_001：" + payloadString);
    if (payloadString == "1") backward();
    //if (payloadString == "0") stop();
  }
  if (strcmp(topic, MQTTSubTopic4_002) == 0) {
    Serial.println("MQTTSubTopic4_001：" + payloadString);
    if (payloadString == "1") backward();
    //if (payloadString == "0") stop();
  }
  if (strcmp(topic, MQTTSubTopic5_002) == 0) {
    Serial.println("MQTTSubTopic5_001：" + payloadString);
    if (payloadString == "1") stop();
    //if (payloadString == "0") stop();
  }
}

void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  
  init_camera();
  init_wifi();
  
  pinMode(LEFT1, OUTPUT);
  pinMode(LEFT2, OUTPUT);
  pinMode(LEFT_PWM, OUTPUT);
  pinMode(RIGHT1, OUTPUT);
  pinMode(RIGHT2, OUTPUT);
  pinMode(RIGHT_PWM, OUTPUT);
  
  MQTTConnecte();
  
}

void loop() {
  MQTTClient.loop();//更新訂閱狀態
  client.poll();
  
  //如果WiFi連線中斷，則重啟WiFi連線
  if (WiFi.status() != WL_CONNECTED) WifiConnecte();
  //如果MQTT連線中斷，則重啟MQTT連線
  if (!MQTTClient.connected())  MQTTConnecte();

  if (client.available()) {
    camera_fb_t *fb = esp_camera_fb_get();
    if (!fb) {
      Serial.println("img capture failed");
      esp_camera_fb_return(fb);
      ESP.restart();
    }
    client.sendBinary((const char*) fb->buf, fb->len);
    Serial.println("image sent");
    esp_camera_fb_return(fb);
    
  }

  
}
