#include <WiFi.h>
#include <PubSubClient.h>
#include <Adafruit_NeoPixel.h>
#include "esp_camera.h"
#include <WiFiClientSecure.h>
#include <PubSubClient.h>//請先安裝PubSubClient程式庫
#define CAMERA_MODEL_AI_THINKER // Has PSRAM


char ssid[] = "C220MIS";
char password[] = "misc220c220";
int Trig = 12;
int Echo = 14;
int buzzer = 2;
int buttonPin = 33;
int NEOPin = 32;
int NEONumber = 1;
Adafruit_NeoPixel pixels(NEONumber, NEOPin, NEO_GRB + NEO_KHZ800);

char* MQTTServer = "mqttgo.io";
int MQTTPort = 1883;
char* MQTTUser = "";
char* MQTTPassword = "";
char* MQTTPubTopic1 = "3th4n/road222/ped";
char* MQTTPubTopic2 = "3th4n/road222/car";
char* MQTTPubTopic3 = "3th4n/road222/image";
char* MQTTSubTopic1 = "3th4n/road222/button";

long MQTTLastPublishTime;
long MQTTPublishInterval = 2000;
WiFiClient WifiClient;
PubSubClient MQTTClient(WifiClient);

unsigned long previousMillisMotion = 0;
unsigned long previousMillisLED = 0;
unsigned long previousMillisUltrasonic = 0;
unsigned long previousMillisBuzzer = 0;
unsigned long previousMillisPrint = 0;
unsigned long motionDetectedMillis = 0;
unsigned long motionEndedMillis = 0;
unsigned long buttonPressedMillis = 0;

const long intervalMotion = 1000;
const long intervalLED = 250;
const long intervalUltrasonic = 50;
const long intervalBuzzer = 250;
const long intervalPrint = 250;
const long delayStartLEDandUltrasonic = 4000;
const long durationLEDandUltrasonic = 6000;
const long durationButtonLEDandUltrasonic = 10000;

bool motionDetected = false;
bool buttonPressed = false;
bool ledState = false;
float CMValue = 0;
bool ultrasonicActive = false;

void setup() {
  Serial.begin(115200);
  pinMode(4, INPUT); // PIR Sensor
  pinMode(15, OUTPUT); // Red LED
  pinMode(13, OUTPUT);// Blue & Yellow LED
  pinMode(Trig, OUTPUT);
  pinMode(Echo, INPUT);
  pinMode(buzzer, OUTPUT);
  pinMode(buttonPin, INPUT);

  // Connect to WiFi
  WifiConnect();

  // Connect to MQTT
  MQTTConnect();

  //初始化相機結束
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = 5;
  config.pin_d1 = 18;
  config.pin_d2 = 19;
  config.pin_d3 = 21;
  config.pin_d4 = 36;
  config.pin_d5 = 39;
  config.pin_d6 = 34;
  config.pin_d7 = 35;
  config.pin_xclk = 0;
  config.pin_pclk = 22;
  config.pin_vsync = 25;
  config.pin_href = 23;
  config.pin_sscb_sda = 26;
  config.pin_sscb_scl = 27;
  config.pin_pwdn = 32;
  config.pin_reset = -1;
  config.xclk_freq_hz = 20000000;
  config.fb_count = 2;
  config.pixel_format = PIXFORMAT_JPEG;
  config.jpeg_quality = 12;
  //設定解析度：FRAMESIZE_ + QVGA|CIF|VGA|SVGA|XGA|SXGA|UXGA
  config.frame_size = FRAMESIZE_QVGA;//VGA=640*480(VGA格式較穩定)
  //Line notify don't accept bigger than XGA
  esp_err_t err = esp_camera_init(&config);
  if (err == ESP_OK) {
    Serial.println("鏡頭啟動成功");
    // setup stream ------------------------
    sensor_t * s = esp_camera_sensor_get();
    int res = 0;
    res = s->set_brightness(s, 1); //亮度:(-2~2)
    res = s->set_contrast(s, 1); //對比度:(-2~2)
    res = s->set_saturation(s, 1); //色彩飽和度:(-2~2)
    //res = s->set_special_effect(s, 0);//特殊效果:(0~6)
    //res = s->set_whitebal(s, 1);//啟動白平衡:(0或1)
    //res = s->set_awb_gain(s, 1);//自動白平衡增益:(0或1)
    //res = s->set_wb_mode(s, 0);//白平衡模式:(0~4)
    //res = set_exposure_ctrl(s, 1);;//曝光控制:(0或1)
    //res = set_aec2(s, 0);//自動曝光校正:(0或1)
    //res = set_ae_level(s, 0);//自動曝光校正程度:(-2~2)
    //res = set_aec_value(s, 300);//自動曝光校正值：(0~1200)
    //res = set_gain_ctrl(s, 1);//增益控制:(0或1)
    //res = set_agc_gain(s, 0);//自動增益:(0~30)
    //res = set_gainceiling(s, (gainceiling_t)0); //增益上限:(0~6)
    //res = set_bpc(s, 1);//bpc開啟:(0或1)
    //res = set_wpc(s, 1);//wpc開啟:(0或1)
    //res = set_raw_gma(s, 1);//影像GMA:(0或1)
    //res = s->set_lenc(s, 1);//鏡頭校正:(0或1)
    //res = s->set_hmirror(s, 1);//水平翻轉:(0或1)
    //res = s->set_vflip(s, 1);//垂直翻轉:(0或1)
    //res = set_dcw(s, 1);//dcw開啟:(0或1)
  } else {
    Serial.printf("鏡頭設定失敗，5秒後重新啟動");
    delay(5000);
    ESP.restart();
  }

}



void loop() {

  unsigned long currentMillis = millis();
  int value = digitalRead(4);
  int buttonState = digitalRead(buttonPin);

  if (currentMillis - previousMillisMotion >= intervalMotion) {
    previousMillisMotion = currentMillis;
    if (value == HIGH) {
      if (!motionDetected) {
        motionDetected = true;
        motionDetectedMillis = currentMillis;
      }
      pixels.setPixelColor(0, pixels.Color(0, 255, 0)); // Green
      pixels.show();
      Serial.println("有人經過");
    } else {
      if (motionDetected) {
        if (currentMillis - motionDetectedMillis < delayStartLEDandUltrasonic) {
          motionDetected = false;
          Serial.println("無人經過");
        } else {
          motionEndedMillis = currentMillis;
          Serial.println("無人經過");
          motionDetected = false;
          pixels.setPixelColor(0, pixels.Color(0, 0, 0)); // Off
          pixels.show();
        }
      }
    }
  }

  if (buttonState == HIGH && !buttonPressed) {
    buttonPressed = true;
    pixels.setPixelColor(0, pixels.Color(0, 255, 0)); // Green
    pixels.show();
    buttonPressedMillis = currentMillis;
    Serial.println("按鈕按下");

    // Publish the button press message
    String payload = "按下按鈕, 1, " + String(CMValue);
    MQTTClient.publish(MQTTPubTopic1, payload.c_str());
    Serial.println("按鈕資訊已推播到MQTT Broker");
  } else if (buttonState == LOW && buttonPressed) {
    buttonPressed = false;
  }

  bool shouldOperateLEDsAndUltrasonic = false;

  if (buttonPressed ||
      (motionDetected && (currentMillis - motionDetectedMillis >= delayStartLEDandUltrasonic)) ||
      (currentMillis - motionEndedMillis < durationLEDandUltrasonic) ||
      (currentMillis - buttonPressedMillis < durationButtonLEDandUltrasonic)) {
    shouldOperateLEDsAndUltrasonic = true;
  }

  if (shouldOperateLEDsAndUltrasonic) {
    ultrasonicActive = true;

    if (currentMillis - previousMillisLED >= intervalLED) {
      previousMillisLED = currentMillis;
      ledState = !ledState;
      Serial.print("LED State: ");
      Serial.println(ledState);
      digitalWrite(15, ledState ? HIGH : LOW);
      digitalWrite(13, ledState ? LOW : HIGH);
    }

    if (currentMillis - previousMillisUltrasonic >= intervalUltrasonic) {
      previousMillisUltrasonic = currentMillis;
      digitalWrite(Trig, LOW);
      delayMicroseconds(5);
      digitalWrite(Trig, HIGH);
      delayMicroseconds(10);
      digitalWrite(Trig, LOW);
      float EchoTime = pulseIn(Echo, HIGH);
      CMValue = EchoTime / 29.4 / 2;
    }

    if (currentMillis - previousMillisBuzzer >= intervalBuzzer) {
      previousMillisBuzzer = currentMillis;
      if (CMValue > 200) {
        tone(buzzer, 0, 0);
      } else if (CMValue <= 200 && CMValue > 100) {
        tone(buzzer, 523, 100);
      } else {
        tone(buzzer, 988, 100);
      }
    }

    if (currentMillis - previousMillisPrint >= intervalPrint) {
      previousMillisPrint = currentMillis;
      Serial.println(CMValue);
    }
  } else {
    if (!motionDetected &&
        (currentMillis - motionEndedMillis >= durationLEDandUltrasonic) ||
        (currentMillis - buttonPressedMillis >= durationButtonLEDandUltrasonic)) {
      digitalWrite(15, LOW);
      digitalWrite(13, LOW);
      pixels.setPixelColor(0, pixels.Color(0, 0, 0)); // Off
      pixels.show();
      ledState = false;
      ultrasonicActive = false;
    }
  }

  delay(10);

  if (WiFi.status() != WL_CONNECTED) WifiConnect();
  if (!MQTTClient.connected()) MQTTConnect();

  if ((millis() - MQTTLastPublishTime) >= MQTTPublishInterval) {
    byte ped = motionDetected ? 1 : 0;
    String payloadPed = String(ped);
    String payloadCar;

    if (ultrasonicActive) {
      payloadCar = String(CMValue);
    } else {
      payloadCar = "off";
    }

    // Publish to respective topics
    MQTTClient.publish(MQTTPubTopic1, payloadPed.c_str());
    MQTTClient.publish(MQTTPubTopic2, payloadCar.c_str());

    Serial.println("人車資訊已推播到MQTT Broker");
    MQTTLastPublishTime = millis();
  }
  MQTTClient.loop();
  delay(50);

    //如果MQTT連線中斷，則重啟MQTT連線
  if (!MQTTClient.connected())   MQTTConnect(); 
  if ((millis() - MQTTLastPublishTime) >= MQTTPublishInterval ) {
    String result = SendImageMQTT();
    Serial.println(result);
    MQTTLastPublishTime = millis(); //更新最後傳輸時間
  }
  delay(100);

}

// WiFi Connection
void WifiConnect() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi連線成功");
  Serial.print("IP Address:");
  Serial.println(WiFi.localIP());
}

// MQTT Connection
void MQTTConnect() {
  MQTTClient.setServer(MQTTServer, MQTTPort);
  MQTTClient.setCallback(MQTTCallback);
  while (!MQTTClient.connected()) {
    String MQTTClientid = "esp32-" + String(random(1000000, 9999999));
    if (MQTTClient.connect(MQTTClientid.c_str(), MQTTUser, MQTTPassword)) {
      Serial.println("MQTT已連線");
      MQTTClient.subscribe(MQTTSubTopic1);
    } else {
      Serial.print("MQTT連線失敗,狀態碼=");
      Serial.println(MQTTClient.state());
      Serial.println("五秒後重新連線");
      delay(5000);
    }
  }
}

// MQTT Callback
void MQTTCallback(char* topic, byte* payload, unsigned int length) {
  Serial.print(topic); Serial.print("訂閱通知:");
  String payloadString;
  for (int i = 0; i < length; i++) {
    payloadString = payloadString + (char)payload[i];
  }
  Serial.println(payloadString);
  if (strcmp(topic, MQTTSubTopic1) == 0) {
    Serial.println("改變燈號：" + payloadString);
    if (payloadString == "1") {
      buttonPressed = true;
      buttonPressedMillis = millis(); // Simulate button press
    }
    if (payloadString == "0") {
      buttonPressed = false;
    }
  }
}

//拍照傳送到MQTT
String SendImageMQTT() {
  camera_fb_t * fb =  esp_camera_fb_get();
  size_t fbLen = fb->len;
  int ps = 512;
  //開始傳遞影像檔
  MQTTClient.beginPublish(MQTTPubTopic3, fbLen, false);
  uint8_t *fbBuf = fb->buf;
  for (size_t n = 0; n < fbLen; n = n + 2048) {
    if (n + 2048 < fbLen) {
      MQTTClient.write(fbBuf, 2048);
      fbBuf += 2048;
    } else if (fbLen % 2048 > 0) {
      size_t remainder = fbLen % 2048;
      MQTTClient.write(fbBuf, remainder);
    }
  }
  boolean isPublished = MQTTClient.endPublish();
  esp_camera_fb_return(fb);//清除緩衝區
  if (isPublished) {
    return "MQTT傳輸成功";
  }
  else {
    return "MQTT傳輸失敗，請檢查網路設定";
  }
}
