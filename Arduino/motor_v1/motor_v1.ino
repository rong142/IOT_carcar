int in1 = 2;  //in1
int in2 = 15; //in2
int ena = 4;  //ena

// Motor B
int in3 = 13; //in3
int in4 = 14; //in4
int enb = 12; //enb

// Setting PWM properties
const int freq = 30000;
const int pwmChannel = 0;
const int resolution = 8;
int dutyCycle = 200;

void setup() {
  // sets the pins as outputs:
  // Motor A
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  pinMode(ena, OUTPUT);
  // Motor B
  pinMode(in3, OUTPUT);
  pinMode(in4, OUTPUT);
  pinMode(enb, OUTPUT);
 
  // configure LEDC PWM
//  ledcAttachChannel(ena, freq, resolution, pwmChannel);

  Serial.begin(115200);

  // testing
  Serial.print("Testing DC Motor...");
}

void loop() {
  // Move the DC motor forward at maximum speed

 while (dutyCycle <= 255){
   ledcWrite(ena, dutyCycle);  
   Serial.print("Forward with duty cycle: ");
   Serial.println(dutyCycle);
   dutyCycle = dutyCycle + 5;

  Serial.println("往前");
  digitalWrite(in1, LOW);
  digitalWrite(in2, HIGH);
  digitalWrite(in3, LOW);
  digitalWrite(in4, HIGH);
  delay(500);

  // Stop the DC motor
  Serial.println("往前_停");
  digitalWrite(in1, LOW);
  digitalWrite(in2, LOW);
  digitalWrite(in3, LOW);
  digitalWrite(in4, LOW);
  delay(1000);

  // Move DC motor backwards at maximum speed
  Serial.println("倒退");
  digitalWrite(in1, HIGH);
  digitalWrite(in2, LOW);
  digitalWrite(in3, HIGH);
  digitalWrite(in4, LOW);
  delay(500);

  // Stop the DC motor
  Serial.println("倒退_停");
  digitalWrite(in1, LOW);
  digitalWrite(in2, LOW);
  digitalWrite(in3, LOW);
  digitalWrite(in4, LOW);
  delay(1000);
 
  //速度
 //Move DC motor forward with increasing speed
  digitalWrite(in1, HIGH);
  digitalWrite(in2, LOW);
  digitalWrite(in3, HIGH);
  digitalWrite(in4, LOW);

   delay(500);
 }
//   while (dutyCycle <= 255){
//    ledcWrite(enb, dutyCycle);  
//    Serial.print("Forward with duty cycle: ");
//    Serial.println(dutyCycle);
//    dutyCycle = dutyCycle + 5;
//    delay(500);
//  }
  dutyCycle = 200;
}
