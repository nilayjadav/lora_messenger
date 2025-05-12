#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Arduino.h>

// LoRa pins
#define LORA_SCK  5
#define LORA_MISO 19
#define LORA_MOSI 23
#define LORA_SS   18
#define LORA_RST  25
#define LORA_DIO0 26

// OLED I2C pins
#define OLED_SDA  21
#define OLED_SCL  22

// Output pins
#define ONBOARD_LED 13     
#define BUZZER      4

// OLED settings
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

QueueHandle_t msgQueue;

void handleReceivedMessage(void *param) {
  String msg;
  for (;;) {
    if (xQueueReceive(msgQueue, &msg, portMAX_DELAY)) {
      // Blink LED and buzz
      for (int i = 0; i < 2; i++) {
        digitalWrite(ONBOARD_LED, LOW);
        digitalWrite(BUZZER, HIGH);
        vTaskDelay(300 / portTICK_PERIOD_MS);
        digitalWrite(ONBOARD_LED, HIGH);
        digitalWrite(BUZZER, LOW);
        vTaskDelay(300 / portTICK_PERIOD_MS);
      }

      int rssi = LoRa.packetRssi();

      display.clearDisplay();
      display.setCursor(0, 0);
      display.println("Message Received");
      display.setCursor(0, 10);
      display.println(msg);
      display.setCursor(0, 20);
      display.print("RSSI: ");
      display.print(rssi);
      display.display();
      vTaskDelay(15000 / portTICK_PERIOD_MS);

      display.clearDisplay();
      display.display();
    }
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("ESP32 LoRa + OLED Receiver");

  pinMode(ONBOARD_LED, OUTPUT);
  digitalWrite(ONBOARD_LED, HIGH);
  pinMode(BUZZER, OUTPUT);
  digitalWrite(BUZZER, LOW);

  Wire.begin(OLED_SDA, OLED_SCL);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED init failed!");
    while (1);
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("OLED Ready");
  display.display();
  delay(1000);

  SPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_SS);
  LoRa.setPins(LORA_SS, -1, LORA_DIO0);
  if (!LoRa.begin(915E6)) {
    Serial.println("LoRa init failed");
    while (1);
  }
  LoRa.setSyncWord(0x12);
  Serial.println("LoRa initialized");

  msgQueue = xQueueCreate(5, sizeof(String));
  xTaskCreate(handleReceivedMessage, "HandleMsg", 4096, NULL, 1, NULL);
}

void loop() {
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    String msg = "";
    while (LoRa.available()) {
      msg += (char)LoRa.read();
    }

    msg.trim();
    Serial.println("Received LoRa: " + msg);

    if (msg.startsWith("RTOS")) {
      msg = msg.substring(4);
    } else {
      msg = "RANDOM MESSAGE";
    }

    if (msgQueue) {
      xQueueSend(msgQueue, &msg, 0);
    }
  }
}
