int Trig = 12; //發出聲波腳位
int Echo = 14; //接收聲波腳位
int buzzer = 5; //蜂鳴器
int buttonPin = 18; //按鈕腳位
#include <Adafruit_NeoPixel.h>
int NEOPin = 32; // 腳位編號
int NEONumber = 1; // 燈組的數量（本例只有一顆）
Adafruit_NeoPixel pixels(NEONumber, NEOPin, NEO_GRB + NEO_KHZ800);

unsigned long previousMillisMotion = 0;
unsigned long previousMillisLED = 0;
unsigned long previousMillisUltrasonic = 0;
unsigned long previousMillisBuzzer = 0;
unsigned long previousMillisPrint = 0; // Time for printing CMValue
unsigned long motionDetectedMillis = 0; // Time when motion was first detected
unsigned long motionEndedMillis = 0; // Time when motion was last detected
unsigned long buttonPressedMillis = 0; // Time when button was pressed

const long intervalMotion = 1000; // Interval for motion detection
const long intervalLED = 250; // Interval for LEDs
const long intervalUltrasonic = 50; // Interval for ultrasonic sensor
const long intervalBuzzer = 250; // Interval for buzzer
const long intervalPrint = 250; // Interval for printing CMValue
const long delayStartLEDandUltrasonic = 3000; // Delay before starting LEDs and ultrasonic sensor (3 seconds)
const long durationLEDandUltrasonic = 6000; // Duration to keep LEDs and ultrasonic sensor on (6 seconds)
const long durationButtonLEDandUltrasonic = 10000; // Duration to keep LEDs and ultrasonic sensor on after button press (10 seconds)

bool motionDetected = false;
bool buttonPressed = false;
bool ledState = false;
float CMValue = 0; // Store the distance value globally

void setup() {
  Serial.begin(115200);
  pinMode(17, INPUT); // SR-501
  pinMode(15, OUTPUT); // 紅色LED
  pinMode(4, OUTPUT); // 黃色or藍色LED
  pinMode(22, OUTPUT);
  pinMode(Trig, OUTPUT);
  pinMode(Echo, INPUT);
  pinMode(buzzer, OUTPUT);
  pinMode(buttonPin, INPUT); // 按鈕
}

void loop() {
  unsigned long currentMillis = millis();
  int value = digitalRead(17);
  int buttonState = digitalRead(buttonPin);

  // Motion detection every 1000 ms
  if (currentMillis - previousMillisMotion >= intervalMotion) {
    previousMillisMotion = currentMillis;
    if (value == HIGH) {
      if (!motionDetected) {
        motionDetected = true;
        motionDetectedMillis = currentMillis; // Record the time when motion is first detected
      }
      pixels.setPixelColor(0, pixels.Color(0, 255, 0)); // 綠色
      pixels.show();
      Serial.println("有人經過");
    } else {
      if (motionDetected) {
        // Only consider the motion ended if it lasted for less than 3 seconds
        if (currentMillis - motionDetectedMillis < delayStartLEDandUltrasonic) {
          motionDetected = false;
          Serial.println("無人經過");
        } else {
          motionEndedMillis = currentMillis; // Record the time when motion is no longer detected
          Serial.println("無人經過");
          motionDetected = false; // Added this line to reset the motionDetected flag
          pixels.setPixelColor(0, pixels.Color(0, 0, 0)); // 關閉
          pixels.show();
        }
      }
    }
  }

  // Button press detection
  if (buttonState == HIGH && !buttonPressed) {
    buttonPressed = true;
    pixels.setPixelColor(0, pixels.Color(0, 255, 0)); // 綠色
    pixels.show();
    buttonPressedMillis = currentMillis; // Record the time when button is pressed
    Serial.println("按鈕按下");
  } else if (buttonState == LOW && buttonPressed) {
    buttonPressed = false;

  }

  // Check if motion was detected for over 3 seconds, or if button was pressed within the last 10 seconds
  bool shouldOperateLEDsAndUltrasonic = false;

  if (buttonPressed ||
      (motionDetected && (currentMillis - motionDetectedMillis >= delayStartLEDandUltrasonic)) ||
      (currentMillis - motionEndedMillis < durationLEDandUltrasonic) ||
      (currentMillis - buttonPressedMillis < durationButtonLEDandUltrasonic)) {
    shouldOperateLEDsAndUltrasonic = true;
  }

  if (shouldOperateLEDsAndUltrasonic) {
    // Toggle LEDs every 250 ms
    if (currentMillis - previousMillisLED >= intervalLED) {
      previousMillisLED = currentMillis;
      ledState = !ledState;
      Serial.print("LED State: ");
      Serial.println(ledState);
      digitalWrite(15, ledState ? HIGH : LOW);
      digitalWrite(4, ledState ? LOW : HIGH);
      digitalWrite(22, ledState ? LOW : HIGH);

    }

    // Measure distance every 50 ms
    if (currentMillis - previousMillisUltrasonic >= intervalUltrasonic) {
      previousMillisUltrasonic = currentMillis;
      digitalWrite(Trig, LOW);
      delayMicroseconds(5);
      digitalWrite(Trig, HIGH);
      delayMicroseconds(10);
      digitalWrite(Trig, LOW);
      float EchoTime = pulseIn(Echo, HIGH); //計算傳回時間
      CMValue = EchoTime / 29.4 / 2; //將時間轉換成距離
    }

    // Sound buzzer based on distance every 250 ms
    if (currentMillis - previousMillisBuzzer >= intervalBuzzer) {
      previousMillisBuzzer = currentMillis;
      if (CMValue > 200) {
        tone(buzzer, 0, 0); // No sound if distance is greater than 200 cm
      } else if (CMValue <= 200 && CMValue > 100) {
        tone(buzzer, 523, 100); //注意距離
      } else {
        tone(buzzer, 988, 100); //警告距離
      }
    }

    // Print CMValue every 250 ms
    if (currentMillis - previousMillisPrint >= intervalPrint) {
      previousMillisPrint = currentMillis;
      Serial.println(CMValue);
    }
  } else {
    // Ensure LEDs are turned off after the duration ends
    if (!motionDetected &&
        (currentMillis - motionEndedMillis >= durationLEDandUltrasonic) ||
        (currentMillis - buttonPressedMillis >= durationButtonLEDandUltrasonic)) {
      digitalWrite(15, LOW);
      digitalWrite(4, LOW);
      digitalWrite(22, LOW);
      pixels.setPixelColor(0, pixels.Color(0, 0, 0)); // 關閉
      pixels.show();
      ledState = false;
    }
  }

  // Small delay to reduce CPU usage
  delay(10);
}
