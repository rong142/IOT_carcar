#include "esp_camera.h"
#include <WiFi.h>
#include <ArduinoWebsockets.h>
#include "esp_timer.h"
#include "img_converters.h"
#include "fb_gfx.h"
#include "soc/soc.h" //disable brownout problems
#include "soc/rtc_cntl_reg.h" //disable brownout problems
#include "driver/gpio.h"

// configuration for AI Thinker Camera board
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

const char* ssid     = "Uokio"; // CHANGE HERE
const char* password = "00000000"; // CHANGE HERE

const char* websockets_server_host = "websocket.icelike.info"; // WebSocket server address
const uint16_t websockets_server_port = 8080; // WebSocket server port
bool connected = false;

camera_fb_t * fb = NULL;

using namespace websockets;
WebsocketsClient client;

void onMessageCallback(WebsocketsMessage message) {
  Serial.print("Got Message: ");
  Serial.println(message.data());
}

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

  // parameters for image quality and size
  config.frame_size = FRAMESIZE_QVGA; // Reduced frame size
  config.jpeg_quality = 20; // Reduced quality
  config.fb_count = 2;

  // Camera init
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
  client.onMessage(onMessageCallback);
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

void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);

  Serial.begin(115200);
  Serial.setDebugOutput(true);

  init_camera();
  init_wifi();
}

void loop() {
  if (connected) {
    camera_fb_t *fb = esp_camera_fb_get();
    if (!fb) {
      Serial.println("img capture failed");
      esp_camera_fb_return(fb);
      ESP.restart();
    }

    // Send raw image data
    client.sendBinary((const char*)fb->buf, fb->len);
    Serial.println("image sent");
    esp_camera_fb_return(fb);
    client.poll();
  } else {
    init_wifi(); // Attempt to reconnect if not connected
  }
}

void websocket_connect() {
  // 連接WiFi
  WiFi.begin(ssid, password);

  // 等待連接WiFi
  for (int i = 0; i < 10 && WiFi.status() != WL_CONNECTED; i++) {
    Serial.print(".");
    delay(1000);
  }

  // 檢查WiFi連接狀態
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("未連接到WiFi！");
    ESP.restart(); // ESP32重啟
    return;
  }

  Serial.println("已連接到WiFi，正在連接到伺服器。");
  // 嘗試連接到Websockets伺服器
  connected = client.connect(websockets_server_host, websockets_server_port, "/");
  if (connected) {
    Serial.println("已連接！");
    client.send("Hello Server"); // 向伺服器發送訊息
  } else {
    Serial.println("未連接！");
  }

  // 當接收到訊息時執行回調函式
  client.onMessage([&](WebsocketsMessage message) {
    Serial.print("收到訊息: ");
    Serial.println(message.data());
    ::message = message.data(); // 儲存接收到的訊息到全域變數
  });
}
