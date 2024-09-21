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

//const char* ssid = "NKUST"; // WiFi credentials
//const char* password = "s0973901050"; // WiFi password
const char* ssid = "Uokio"; // WiFi credentials
const char* password = "00000000"; // WiFi password
const char* camera_server_host = "192.168.3.9"; // WebSocket server address for camera
const uint16_t camera_server_port = 8090; // WebSocket server port for camera
const char* control_server_host = "192.168.3.9"; // WebSocket server address for motor control
const uint16_t control_server_port = 8080; // WebSocket server port for motor control

using namespace websockets;
WebsocketsClient camera_client; // Client for sending camera data
WebsocketsClient control_client; // Client for receiving motor control commands
String message; // To store received WebSocket messages
bool camera_connected = false; // To keep track of camera WebSocket connection status
bool control_connected = false; // To keep track of control WebSocket connection status

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
  config.jpeg_quality = 5; // Lower quality
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

// Function to initialize WiFi and WebSocket connections
esp_err_t init_wifi() {
  WiFi.begin(ssid, password);
  Serial.println("Wifi init ");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi OK");

  // Initialize WebSocket clients
  camera_client.onMessage([](WebsocketsMessage msg) {
    Serial.print("Camera WS Message: ");
    Serial.println(msg.data());
  });
  camera_connected = camera_client.connect(camera_server_host, camera_server_port, "/");
  if (!camera_connected) {
    Serial.println("Camera WS connect failed!");
    return ESP_FAIL;
  }
  Serial.println("Camera WS OK");
  camera_client.send("hello from ESP32 camera stream!");

  control_client.onMessage([&](WebsocketsMessage msg) {
    Serial.print("Control WS Message: ");
    Serial.println(msg.data());
    message = msg.data(); // Store the received message
  });
  control_connected = control_client.connect(control_server_host, control_server_port, "/");
  if (!control_connected) {
    Serial.println("Control WS connect failed!");
    return ESP_FAIL;
  }
  Serial.println("Control WS OK");
  control_client.send("Hello Server");

  return ESP_OK;
}

// Function to reconnect to WebSocket if needed
void websocket_connect() {
  if (!camera_connected) {
    camera_connected = camera_client.connect(camera_server_host, camera_server_port, "/");
    if (camera_connected) {
      Serial.println("Camera WS Reconnected!");
      camera_client.send("Hello Server");
    } else {
      Serial.println("Camera WS Reconnect failed!");
    }
  }

  if (!control_connected) {
    control_connected = control_client.connect(control_server_host, control_server_port, "/");
    if (control_connected) {
      Serial.println("Control WS Reconnected!");
      control_client.send("Hello Server");
    } else {
      Serial.println("Control WS Reconnect failed!");
    }
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
  if (camera_connected) {
    // Capture and send image
    camera_fb_t *fb = esp_camera_fb_get();
    if (!fb) {
      Serial.println("img capture failed");
      esp_camera_fb_return(fb);
      ESP.restart();
    } else {
      camera_client.sendBinary((const char*)fb->buf, fb->len);
      Serial.println("image sent");
      esp_camera_fb_return(fb);
    }
  } else {
    websocket_connect(); // Reconnect if not connected
  }

  if (control_connected) {
    // Process received WebSocket messages
    if (message == "up") forward();
    if (message == "down") backward();
    if (message == "left") turnLeft();
    if (message == "right") turnRight();
    if (message == "pause") stop();

    control_client.poll(); // Poll WebSocket
  } else {
    websocket_connect(); // Reconnect if not connected
  }
}
