int Trig = 12; //發出聲波腳位
int Echo = 14; //接收聲波腳位
int buzzer = 17; //蜂鳴器
int relay = 23; //繼電器
bool relayOff = false; // 繼電器狀態標誌

unsigned long previousMillisPurple = 0;
unsigned long previousMillisRed = 0;
unsigned long previousMillisYellow = 0;

const long intervalPurple = 100;
const long intervalRed = 250;
const long intervalYellow = 500;

bool purpleState = false;
bool redState = false;
bool yellowState = false;

void setup() {
  Serial.begin(115200);
  pinMode(Trig, OUTPUT);
  pinMode(Echo, INPUT);
  pinMode(buzzer, OUTPUT);
  pinMode(4, OUTPUT);//GPIO 4輸出RGB LED的紅色
  pinMode(2, OUTPUT);//GPIO 2輸出RGB LED的綠色
  pinMode(15, OUTPUT);//GPIO 15輸出RGB LED的藍色
  pinMode(23, OUTPUT);//GPIO 23輸出繼電器訊號源
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
  delay(50);

  unsigned long currentMillis = millis();

  if (CMValue <= 5) {
    //碰撞距離紫燈
    if (currentMillis - previousMillisPurple >= intervalPurple) {
      previousMillisPurple = currentMillis;
      purpleState = !purpleState;
      analogWrite(15, purpleState ? 255 : 0);
      analogWrite(2, 0);
      analogWrite(4, purpleState ? 255 : 0);
    }
    tone(buzzer, 4186, 100);
    digitalWrite(relay, LOW);
    relayOff = true; // 將繼電器狀態標誌設為關閉
  } 
  else if (CMValue <= 15 && CMValue > 5) {
    //警告距離紅燈
    if (currentMillis - previousMillisRed >= intervalRed) {
      previousMillisRed = currentMillis;
      redState = !redState;
      analogWrite(15, 0);
      analogWrite(2, 0);
      analogWrite(4, redState ? 255 : 0);
    }
    tone(buzzer, 523, 100);
    if (!relayOff) { // 如果繼電器狀態標誌未設為關閉，則關閉繼電器
      digitalWrite(relay, LOW);
      relayOff = true; // 將繼電器狀態標誌設為關閉
    }
  } 
  else if (CMValue <= 30 && CMValue > 15) {
    //注意距離黃燈
    if (currentMillis - previousMillisYellow >= intervalYellow) {
      previousMillisYellow = currentMillis;
      yellowState = !yellowState;
      analogWrite(15, 0);
      analogWrite(2, yellowState ? 255 : 0);
      analogWrite(4, yellowState ? 255 : 0);
    }
    tone(buzzer, 253, 100);
    if (relayOff) { // 如果繼電器狀態標誌設為關閉，則打開繼電器
      digitalWrite(relay, HIGH);
      relayOff = false; // 將繼電器狀態標誌設為開啟
    }
  } 
  else if (CMValue <= 50 && CMValue > 30) {
    //安全距離綠燈
    analogWrite(15, 0);
    analogWrite(2, 255);
    analogWrite(4, 0);
    if (relayOff) { // 如果繼電器狀態標誌設為關閉，則打開繼電器
      digitalWrite(relay, HIGH);
      relayOff = false; // 將繼電器狀態標誌設為開啟
    }
  } 
  else {
    analogWrite(15, 0);
    analogWrite(2, 0);
    analogWrite(4, 0);
    if (relayOff) { // 如果繼電器狀態標誌設為關閉，則打開繼電器
      digitalWrite(relay, HIGH);
      relayOff = false; // 將繼電器狀態標誌設為開啟
    }
  }
  delay(100);
  Serial.print("relayOff= ");
  Serial.println(relayOff);
}
