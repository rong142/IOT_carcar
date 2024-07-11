// 接收序列埠值的變數
char cmd;
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

void setup() {
  Serial.begin(115200);
  pinMode(LEFT1, OUTPUT);
  pinMode(LEFT2, OUTPUT);
  pinMode(LEFT_PWM, OUTPUT);
  pinMode(RIGHT1, OUTPUT);
  pinMode(RIGHT2, OUTPUT);
  pinMode(RIGHT_PWM, OUTPUT);
}

void loop() {
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
}
