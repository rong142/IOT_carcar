int Trig = 12; //發出聲波腳位
int Echo = 14; //接收聲波腳位
int buzzer = 5; //蜂鳴器
int buttonPin = 23; //按鈕腳位

unsigned long previousMillisMotion = 0;
unsigned long previousMillisLED = 0;
unsigned long previousMillisUltrasonic = 0;
unsigned long previousMillisBuzzer = 0;
unsigned long motionDetectedMillis = 0; // Time when motion was first detected
unsigned long motionEndedMillis = 0; // Time when motion was last detected

const long intervalMotion = 1000; // Interval for motion detection
const long intervalLED = 250; // Interval for LEDs
const long intervalUltrasonic = 50; // Interval for ultrasonic sensor
const long intervalBuzzer = 100; // Interval for buzzer
const long delayStartLEDandUltrasonic = 3000; // Delay before starting LEDs and ultrasonic sensor (3 seconds)
const long durationLEDandUltrasonic = 10000; // Duration to keep LEDs and ultrasonic sensor on (10 seconds)

bool motionDetected = false;
bool buttonPressed = false;
bool ledState = false;
float CMValue = 0; // Store the distance value globally

void setup() {
  Serial.begin(115200);
  pinMode(17, INPUT); // SR-501
  pinMode(15, OUTPUT); // 紅色LED
  pinMode(4, OUTPUT); // 黃色LED
  pinMode(Trig, OUTPUT);
  pinMode(Echo, INPUT);
  pinMode(buzzer, OUTPUT);
  pinMode(buttonPin, INPUT); // 按鈕
}

void loop() {
  unsigned long currentMillis = millis();
  int value = digitalRead(17);
  int buttonState = digitalRead(buttonPin);

  // Motion detection or button press every 1000 ms
  if (currentMillis - previousMillisMotion >= intervalMotion) {
    previousMillisMotion = currentMillis;
    if (value == HIGH) {
      if (!motionDetected) {
        motionDetected = true;
        motionDetectedMillis = currentMillis; // Record the time when motion is first detected
      } Serial.println("有人經過");
    } else {
      if (motionDetected) {
        motionDetected = false;
        motionEndedMillis = currentMillis; // Record the time when motion is no longer detected
      } Serial.println("無人經過");
    }

    if (buttonState == HIGH) {
      buttonPressed = true;
      Serial.println("按鈕按下");
    } else {
      buttonPressed = false;
    }
  }

  if (motionDetected || buttonPressed || (currentMillis - motionEndedMillis < durationLEDandUltrasonic)) {
    unsigned long elapsedMillis = currentMillis - motionDetectedMillis;

    if (buttonPressed || (motionDetected && elapsedMillis >= delayStartLEDandUltrasonic) || (currentMillis - motionEndedMillis < durationLEDandUltrasonic)) {
      // Toggle LEDs every 250 ms
      if (currentMillis - previousMillisLED >= intervalLED) {
        previousMillisLED = currentMillis;
        ledState = !ledState;
        Serial.print("LED State: ");
        Serial.println(ledState);
        digitalWrite(15, ledState ? HIGH : LOW);
        digitalWrite(4, ledState ? LOW : HIGH);
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

      // Sound buzzer based on distance every 100 ms
      if (currentMillis - previousMillisBuzzer >= intervalBuzzer) {
        previousMillisBuzzer = currentMillis;
        if (CMValue > 100) {
          tone(buzzer, 523, 100); //警告距離紅燈
        } else if (CMValue <= 200 && CMValue > 100) {
          tone(buzzer, 253, 100); //注意距離黃燈
        } else {
          tone(buzzer, 0, 0);
        }
      }
    }
  } else {
    // Ensure LEDs are turned off after the duration ends
    if (currentMillis - motionEndedMillis >= durationLEDandUltrasonic) {
      digitalWrite(15, LOW);
      digitalWrite(4, LOW);
    }
  }

  // Small delay to reduce CPU usage
  delay(10);
}
