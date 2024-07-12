int Trig = 12; //發出聲波腳位
int Echo = 14; //接收聲波腳位
int buzzer = 17;//蜂鳴器

void setup() {
  Serial.begin(115200);
  pinMode(Trig, OUTPUT);
  pinMode(Echo, INPUT);
  pinMode(buzzer, OUTPUT);
  pinMode(4, OUTPUT);//GPIO 4輸出RGB LED的紅色
  pinMode(2, OUTPUT);//GPIO 2輸出RGB LED的綠色
  pinMode(15, OUTPUT);//GPIO 15輸出RGB LED的藍色
}

void loop() {
  digitalWrite(Trig, LOW); //先關閉
  delayMicroseconds(5);
  digitalWrite(Trig, HIGH);//啟動超音波
  delayMicroseconds(10);
  digitalWrite(Trig, LOW); //關閉
  float EchoTime = pulseIn(Echo, HIGH); //計算傳回時間
  float CMValue = EchoTime / 29.4 / 2; //將時間轉換成距離
  Serial.println(CMValue);
  Serial.println(buzzer);
  delay(50);

  if (CMValue <= 5) {
    //碰撞距離紫燈
    analogWrite(15, 255);
    analogWrite(2, 0);
    analogWrite(4, 255);
    tone(buzzer, 4186, 100);
  }
  else if (CMValue <= 15 && CMValue > 5) {
    //警告距離紅燈
    analogWrite(15, 0);
    analogWrite(2, 0);
    analogWrite(4, 255);
    tone(buzzer, 523, 100);
  }
  else if (CMValue <= 30 && CMValue > 15) {
    //注意距離黃燈
    analogWrite(15, 0);
    analogWrite(2, 255);
    analogWrite(4, 255);
    tone(buzzer, 253, 100);
  }
  else if (CMValue <= 50 && CMValue > 30) {

    //安全距離綠燈
    analogWrite(15, 0);
    analogWrite(2, 255);
    analogWrite(4, 0);
  }
  else
  { analogWrite(15, 0);
    analogWrite(2, 0);
    analogWrite(4, 0);
  }
  delay(100);
}
