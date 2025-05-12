#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <SPI.h>
#include <LoRa.h>
#include "esp_system.h"

// LoRa Pins
#define SCK  5
#define MISO 19
#define MOSI 27
#define SS   18
#define RST  14
#define DI0  26
#define ONBOARD_LED 2  
// Wi-Fi credentials
const char* ssid = "ssid";
const char* password = "pass";

// LoRa settings
#define LORA_BAND 915E6
#define SYNC_WORD 0x12

// Web server
AsyncWebServer server(80);

// HTML page
const char index_html[] PROGMEM = R"rawlite(
<!DOCTYPE html>
<html lang="en">
<head><meta charset="UTF-8"><meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>ESP32 LoRa Messenger</title>
<style>
body { font-family: Arial; text-align: center; background: #f2f4f8; margin-top: 50px; }
.container { max-width: 500px; margin: auto; padding: 20px; background: white; box-shadow: 0 2px 5px rgba(0,0,0,0.3); border-radius: 8px; }
textarea { width: 90%; height: 100px; font-size: 16px; padding: 8px; resize: vertical; border-radius: 4px; border: 1px solid #ccc; }
button { padding: 10px 20px; font-size: 16px; background-color: #4CAF50; color: white; border: none; border-radius: 4px; cursor: pointer; margin-top: 10px; }
button:hover { background-color: #45a049; }
#status { margin-top: 15px; font-weight: bold; color: #333; }
</style>
</head>
<body>
<div class="container">
  <h2>ESP32 LoRa Messenger</h2>
  <textarea id="message" placeholder="Enter your message here..."></textarea><br>
  <button onclick="sendMessage()">Send Message</button>
  <div id="status"></div>
</div>
<script>
function sendMessage() {
  const message = document.getElementById('message').value;
  if (message.trim() === "") { alert("Please enter a message before sending."); return; }
  fetch('/send', { method: 'POST', body: message })
    .then(res => res.text()).then(data => {
      document.getElementById('status').innerText = data;
      setTimeout(() => { document.getElementById('status').innerText = ""; }, 3000);
      document.getElementById('message').value = "";
    }).catch(err => {
      document.getElementById('status').innerText = "Error sending message!";
    });
}
</script>
</body>
</html>
)rawlite";

QueueHandle_t loraQueue;
TaskHandle_t loraTaskHandle;
int msgCount = 1;

void sendLoRaTask(void *pvParameters) {
  String msg;
  for (;;) {
    if (xQueueReceive(loraQueue, &msg, portMAX_DELAY)) {
      Serial.println("Sending via LoRa: " + msg);

      // Turn ON LED
      digitalWrite(ONBOARD_LED, HIGH);

      LoRa.beginPacket();
      LoRa.print("RTOS" + msg);
      int res = LoRa.endPacket(true);

      Serial.println(res == 1 ? "LoRa send successful" : "LoRa send failed");

      // Wait and turn OFF LED after 500ms
      vTaskDelay(500 / portTICK_PERIOD_MS);
      digitalWrite(ONBOARD_LED, LOW);
    }
  }
}


void setup() {
  Serial.begin(115200);
  delay(500);
  pinMode(ONBOARD_LED, OUTPUT);
  digitalWrite(ONBOARD_LED, LOW);
  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print('.');
  }
  Serial.println("\nConnected. IP: " + WiFi.localIP().toString());

  // Initialize LoRa
  SPI.begin(SCK, MISO, MOSI, SS);
  LoRa.setPins(SS, RST, DI0);
  if (!LoRa.begin(LORA_BAND)) {
    Serial.println("LoRa init failed!");
    while (1);
  }
  LoRa.setSyncWord(SYNC_WORD);
  Serial.println("LoRa initialized.");
  
  // Create LoRa message queue
  loraQueue = xQueueCreate(5, sizeof(String));
  xTaskCreate(sendLoRaTask, "LoRaSend", 4096, NULL, 1, &loraTaskHandle);

  // Serve HTML
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *req){
    req->send_P(200, "text/html", index_html);
  });

  // Handle send
  server.on("/send", HTTP_POST, [](AsyncWebServerRequest *req){}, NULL,
    [](AsyncWebServerRequest *req, uint8_t *data, size_t len, size_t idx, size_t total){
      String msg = "";
      for (size_t i = 0; i < len; i++) msg += (char)data[i];
      msg = "#" + String(msgCount++) + ": " + msg;
      if (xQueueSend(loraQueue, &msg, 100 / portTICK_PERIOD_MS)) {
        req->send(200, "text/plain", "Message sent!");
      } else {
        req->send(200, "text/plain", "Try again.");
      }
  });

  server.begin();
  
}

void loop() {
  // Nothing here
}
