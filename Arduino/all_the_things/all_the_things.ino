#include "esp_camera.h"
#include <WiFi.h>
#include <PubSubClient.h>
char cmd;

#define CAMERA_MODEL_AI_THINKER
#include "camera_pins.h"

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
byte motorSpeed = 200;

char ssid[] = "Uokio";
char password[] = "00000000";

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

void startCameraServer();
void setupLedFlash(int pin);


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
  Serial.setDebugOutput(true);
  pinMode(LEFT1, OUTPUT);
  pinMode(LEFT2, OUTPUT);
  pinMode(LEFT_PWM, OUTPUT);
  pinMode(RIGHT1, OUTPUT);
  pinMode(RIGHT2, OUTPUT);
  pinMode(RIGHT_PWM, OUTPUT);

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
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.frame_size = FRAMESIZE_UXGA;
  config.pixel_format = PIXFORMAT_JPEG;  // for streaming
  //config.pixel_format = PIXFORMAT_RGB565; // for face detection/recognition
  config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
  config.fb_location = CAMERA_FB_IN_PSRAM;
  config.jpeg_quality = 12;
  config.fb_count = 1;

  // if PSRAM IC present, init with UXGA resolution and higher JPEG quality
  //                      for larger pre-allocated frame buffer.
  if (config.pixel_format == PIXFORMAT_JPEG) {
    if (psramFound()) {
      config.jpeg_quality = 10;
      config.fb_count = 2;
      config.grab_mode = CAMERA_GRAB_LATEST;
    } else {
      // Limit the frame size when PSRAM is not available
      config.frame_size = FRAMESIZE_SVGA;
      config.fb_location = CAMERA_FB_IN_DRAM;
    }
  } else {
    // Best option for face detection/recognition
    config.frame_size = FRAMESIZE_240X240;
#if CONFIG_IDF_TARGET_ESP32S3
    config.fb_count = 2;
#endif
  }

#if defined(CAMERA_MODEL_ESP_EYE)
  pinMode(13, INPUT_PULLUP);
  pinMode(14, INPUT_PULLUP);
#endif

  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

  sensor_t *s = esp_camera_sensor_get();
  // initial sensors are flipped vertically and colors are a bit saturated
  if (s->id.PID == OV3660_PID) {
    s->set_vflip(s, 1);        // flip it back
    s->set_brightness(s, 1);   // up the brightness just a bit
    s->set_saturation(s, -2);  // lower the saturation
  }
  // drop down frame size for higher initial frame rate
  if (config.pixel_format == PIXFORMAT_JPEG) {
    s->set_framesize(s, FRAMESIZE_QVGA);
  }

#if defined(CAMERA_MODEL_M5STACK_WIDE) || defined(CAMERA_MODEL_M5STACK_ESP32CAM)
  s->set_vflip(s, 1);
  s->set_hmirror(s, 1);
#endif

#if defined(CAMERA_MODEL_ESP32S3_EYE)
  s->set_vflip(s, 1);
#endif

// Setup LED FLash if LED pin is defined in camera_pins.h
#if defined(LED_GPIO_NUM)
  setupLedFlash(LED_GPIO_NUM);
#endif

  WiFi.begin(ssid, password);
  WiFi.setSleep(false);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  startCameraServer();

  Serial.print("Camera Ready! Use 'http://");
  Serial.print(WiFi.localIP());
  Serial.println("' to connect");
  
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

  MQTTClient.loop();//更新訂閱狀態
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
    //if (payloadString == "0") stop();
  }
  if (strcmp(topic, MQTTSubTopic2) == 0) {
    Serial.println("往後：" + payloadString);
    if (payloadString == "1") backward();
    //if (payloadString == "0") stop();
  }
  if (strcmp(topic, MQTTSubTopic3) == 0) {
    Serial.println("左轉：" + payloadString);
    if (payloadString == "1") turnLeft();
    //if (payloadString == "0") stop();
  }
  if (strcmp(topic, MQTTSubTopic4) == 0) {
    Serial.println("右轉：" + payloadString);
    if (payloadString == "1") turnRight();
    //if (payloadString == "0") stop();
  }
  if (strcmp(topic, MQTTSubTopic5) == 0) {
    Serial.println("停止：" + payloadString);
    if (payloadString == "1") stop();
    //if (payloadString == "0") stop();
  }
}

void backward() {  // 馬達轉向：後退
  Serial.println("往前");
  digitalWrite(LEFT1, HIGH);
  digitalWrite(LEFT2, LOW);
  digitalWrite(RIGHT1, HIGH);
  digitalWrite(RIGHT2, LOW);

  analogWrite(LEFT_PWM, motorSpeed);
  analogWrite(RIGHT_PWM, motorSpeed);

  MQTTClient.publish(MQTTSubTopic2, String(0).c_str());
  MQTTClient.publish(MQTTSubTopic3, String(0).c_str());
  MQTTClient.publish(MQTTSubTopic4, String(0).c_str());
  MQTTClient.publish(MQTTSubTopic5, String(0).c_str());
}

void forward() {  // 馬達轉向：前進
  Serial.println("後退");
  digitalWrite(LEFT1, LOW);
  digitalWrite(LEFT2, HIGH);
  digitalWrite(RIGHT1, LOW);
  digitalWrite(RIGHT2, HIGH);

  analogWrite(LEFT_PWM, motorSpeed);
  analogWrite(RIGHT_PWM, motorSpeed);

  MQTTClient.publish(MQTTSubTopic1, String(0).c_str());
  MQTTClient.publish(MQTTSubTopic3, String(0).c_str());
  MQTTClient.publish(MQTTSubTopic4, String(0).c_str());
  MQTTClient.publish(MQTTSubTopic5, String(0).c_str());
}

void turnRight() {  // 馬達轉向：右轉
  Serial.println("左轉");
  digitalWrite(LEFT1, LOW);
  digitalWrite(LEFT2, HIGH);
  digitalWrite(RIGHT1, HIGH);
  digitalWrite(RIGHT2, LOW);

  analogWrite(LEFT_PWM, motorSpeed);
  analogWrite(RIGHT_PWM, 0);

  MQTTClient.publish(MQTTSubTopic1, String(0).c_str());
  MQTTClient.publish(MQTTSubTopic2, String(0).c_str());
  MQTTClient.publish(MQTTSubTopic4, String(0).c_str());
  MQTTClient.publish(MQTTSubTopic5, String(0).c_str());
}

void turnLeft() {  // 馬達轉向：左轉
  Serial.println("右轉");
  digitalWrite(LEFT1, HIGH);
  digitalWrite(LEFT2, LOW);
  digitalWrite(RIGHT1, LOW);
  digitalWrite(RIGHT2, HIGH);

  analogWrite(LEFT_PWM, 0);
  analogWrite(RIGHT_PWM, motorSpeed);

  MQTTClient.publish(MQTTSubTopic1, String(0).c_str());
  MQTTClient.publish(MQTTSubTopic2, String(0).c_str());
  MQTTClient.publish(MQTTSubTopic3, String(0).c_str());
  MQTTClient.publish(MQTTSubTopic5, String(0).c_str());
}

void stop() {  // 停止
  Serial.println("停止");
  digitalWrite(LEFT1, LOW);
  digitalWrite(LEFT2, LOW);
  digitalWrite(RIGHT1, LOW);
  digitalWrite(RIGHT2, LOW);

  analogWrite(LEFT_PWM, 0);
  analogWrite(RIGHT_PWM, 0);

  MQTTClient.publish(MQTTSubTopic1, String(0).c_str());
  MQTTClient.publish(MQTTSubTopic2, String(0).c_str());
  MQTTClient.publish(MQTTSubTopic3, String(0).c_str());
  MQTTClient.publish(MQTTSubTopic4, String(0).c_str());


}
