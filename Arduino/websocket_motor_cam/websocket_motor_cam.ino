#include "esp_camera.h"
#include <WiFi.h>
#include <ArduinoWebsockets.h>
#include "esp_timer.h"
#include "img_converters.h"
#include "fb_gfx.h"
#include "soc/soc.h" // disable brownout problems
#include "soc/rtc_cntl_reg.h" // disable brownout problems
#include "driver/gpio.h"

// Camera configuration for AI Thinker Camera board
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

// Motor pins
const byte LEFT1 = 2;
const byte LEFT2 = 15;
const byte LEFT_PWM = 4;
const byte RIGHT1 = 13;
const byte RIGHT2 = 14;
const byte RIGHT_PWM = 12;

// Motor speed
byte motorSpeed = 255;

const char* ssid = "Uokio"; // WiFi credentials
const char* password = "00000000"; // WiFi password
const char* websockets_server_host = "websocket.icelike.info"; // WebSocket server address
const uint16_t websockets_server_port = 8080; // WebSocket server port

using namespace websockets;
WebsocketsClient client;
String message; // To store received WebSocket messages
bool connected = false; // To keep track of WebSocket connection status

// Function to initialize the camera
esp_err_t init_camera() {
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
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  config.frame_size = FRAMESIZE_QVGA; // Lower resolution
  config.jpeg_quality = 12; // Lower quality
  config.fb_count = 2;

  // Initialize the camera
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("camera init FAIL: 0x%x", err);
    return err;
  }
  sensor_t * s = esp_camera_sensor_get();
  s->set_framesize(s, FRAMESIZE_QVGA); // Set frame size
  Serial.println("camera init OK");
  return ESP_OK;
}

// Function to initialize WiFi and WebSocket connection
esp_err_t init_wifi() {
  WiFi.begin(ssid, password);
  Serial.println("Wifi init ");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi OK");
  Serial.println("connecting to WS: ");
  client.onMessage([&](WebsocketsMessage msg) {
    Serial.print("Got Message: ");
    Serial.println(msg.data());
    message = msg.data(); // Store the received message
  });
  connected = client.connect(websockets_server_host, websockets_server_port, "/");
  if (!connected) {
    Serial.println("WS connect failed!");
    Serial.println(WiFi.localIP());
    return ESP_FAIL;
  }
  Serial.println("WS OK");
  client.send("hello from ESP32 camera stream!");
  return ESP_OK;
}

// Function to reconnect to WebSocket if needed
void websocket_connect() {
  WiFi.begin(ssid, password);
  for (int i = 0; i < 10 && WiFi.status() != WL_CONNECTED; i++) {
    Serial.print(".");
    delay(1000);
  }
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("未連接到WiFi！");
    ESP.restart(); // ESP32 reboot
    return;
  }
  Serial.println("已連接到WiFi，正在連接到伺服器。");
  connected = client.connect(websockets_server_host, websockets_server_port, "/");
  if (connected) {
    Serial.println("已連接！");
    client.send("Hello Server");
  } else {
    Serial.println("未連接！");
  }
}

// Motor control functions
void forward() {
  digitalWrite(LEFT1, LOW);
  digitalWrite(LEFT2, HIGH);
  digitalWrite(RIGHT1, LOW);
  digitalWrite(RIGHT2, HIGH);
  analogWrite(LEFT_PWM, motorSpeed);
  analogWrite(RIGHT_PWM, motorSpeed);
}

void backward() {
  digitalWrite(LEFT1, HIGH);
  digitalWrite(LEFT2, LOW);
  digitalWrite(RIGHT1, HIGH);
  digitalWrite(RIGHT2, LOW);
  analogWrite(LEFT_PWM, motorSpeed);
  analogWrite(RIGHT_PWM, motorSpeed);
}

void turnLeft() {
  Serial.println("左轉");
  digitalWrite(LEFT1, HIGH);
  digitalWrite(LEFT2, LOW);
  digitalWrite(RIGHT1, LOW);
  digitalWrite(RIGHT2, HIGH);
  analogWrite(LEFT_PWM, 0);
  analogWrite(RIGHT_PWM, motorSpeed);
}

void turnRight() {
  Serial.println("右轉");
  digitalWrite(LEFT1, LOW);
  digitalWrite(LEFT2, HIGH);
  digitalWrite(RIGHT1, HIGH);
  digitalWrite(RIGHT2, LOW);
  analogWrite(LEFT_PWM, motorSpeed);
  analogWrite(RIGHT_PWM, 0);
}

void stop() {
  Serial.println("停止");
  digitalWrite(LEFT1, LOW);
  digitalWrite(LEFT2, LOW);
  digitalWrite(RIGHT1, LOW);
  digitalWrite(RIGHT2, LOW);
  analogWrite(LEFT_PWM, 0);
  analogWrite(RIGHT_PWM, 0);
}

void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);

  Serial.begin(115200);
  Serial.setDebugOutput(true);

  // Initialize camera
  init_camera();
  
  // Initialize motor pins
  pinMode(LEFT1, OUTPUT);
  pinMode(LEFT2, OUTPUT);
  pinMode(LEFT_PWM, OUTPUT);
  pinMode(RIGHT1, OUTPUT);
  pinMode(RIGHT2, OUTPUT);
  pinMode(RIGHT_PWM, OUTPUT);

  digitalWrite(LEFT1, HIGH);
  digitalWrite(LEFT2, LOW);
  digitalWrite(RIGHT1, HIGH);
  digitalWrite(RIGHT2, LOW);
  analogWrite(LEFT_PWM, 0);
  analogWrite(RIGHT_PWM, 0);

  // Initialize WiFi and WebSocket
  init_wifi();
}

void loop() {
  if (connected) {
    // Capture and send image
    camera_fb_t *fb = esp_camera_fb_get();
    if (!fb) {
      Serial.println("img capture failed");
      esp_camera_fb_return(fb);
      ESP.restart();
    } else {
      client.sendBinary((const char*)fb->buf, fb->len);
      Serial.println("image sent");
      esp_camera_fb_return(fb);
    }

    // Process received WebSocket messages
    if (message == "up") forward();
    if (message == "down") backward();
    if (message == "left") turnLeft();
    if (message == "right") turnRight();
    if (message == "pause") stop();

    client.poll(); // Poll WebSocket
  } else {
    websocket_connect(); // Reconnect if not connected
  }
}
