// 左馬達
const byte LEFT1 = 2;
const byte LEFT2 = 15;
const byte LEFT_PWM = 4;
// 右馬達
const byte RIGHT1 = 13;
const byte RIGHT2 = 14;
const byte RIGHT_PWM = 12;
// 設定PWM輸出值
byte motorSpeed = 0;

void setup() {
  Serial.begin(115200);
  pinMode(LEFT1, OUTPUT);
  pinMode(LEFT2, OUTPUT);
  pinMode(LEFT_PWM, OUTPUT);
  pinMode(RIGHT1, OUTPUT);
  pinMode(RIGHT2, OUTPUT);
  pinMode(RIGHT_PWM, OUTPUT);
  digitalWrite(LEFT1,HIGH);
  digitalWrite(LEFT2,LOW);
  digitalWrite(RIGHT1,HIGH);
  digitalWrite(RIGHT2,LOW);
}

void loop() {
  for(int i=1 ; i<=1024; i++){
    motorSpeed = i;
    analogWrite(LEFT_PWM,i);
    analogWrite(RIGHT_PWM,i);
    Serial.println(i);
    delay(30);
  }
}
