#include <ArduinoWebsockets.h>
#include <WiFi.h>

const char* ssid = "Uokio"; // 輸入你的WiFi網路名稱
const char* password = "00000000"; // 輸入你的WiFi密碼
const char* websockets_server_host = "websocket.icelike.info"; // 輸入WebSocket伺服器位址
const uint16_t websockets_server_port = 8080; // 輸入WebSocket伺服器埠號

using namespace websockets;

WebsocketsClient client;

void setup() {
    Serial.begin(115200);
    // 連接WiFi
    WiFi.begin(ssid, password);

    // 等待連接WiFi
    for(int i = 0; i < 10 && WiFi.status() != WL_CONNECTED; i++) {
        Serial.print(".");
        delay(1000);
    }

    // 檢查WiFi連接狀態
    if(WiFi.status() != WL_CONNECTED) {
        Serial.println("未連接到WiFi！");
        return;
    }

    Serial.println("已連接到WiFi，正在連接到伺服器。");
    // 嘗試連接到Websockets伺服器
    bool connected = client.connect(websockets_server_host, websockets_server_port, "/");
    if(connected) {
        Serial.println("已連接！");
        client.send("Hello Server"); // 向伺服器發送訊息
    } else {
        Serial.println("未連接！");
    }
    
    // 當接收到訊息時執行回調函式
    client.onMessage([&](WebsocketsMessage message){
        Serial.print("收到訊息: ");
        Serial.println(message.data());
    });
}

void loop() {
    // 讓Websockets客戶端檢查是否有新的訊息
    if(client.available()) {
        client.poll();
    }
}
