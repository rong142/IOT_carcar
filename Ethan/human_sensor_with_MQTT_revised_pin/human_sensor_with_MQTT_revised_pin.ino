#include <WiFi.h>
#include <PubSubClient.h>
#include <Adafruit_NeoPixel.h>

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
const long delayStartLEDandUltrasonic = 3000;
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
