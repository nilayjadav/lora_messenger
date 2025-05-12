# ESP32 LoRa Messenger (FreeRTOS-based)
## Author: Nilay Jadav

### This project demonstrates a **LoRa-based wireless messaging system** using two **ESP32** boards and **SX1276 LoRa modules**, with the **sender controlled via a web interface** and the **receiver displaying messages on an OLED screen**, buzzing and blinking an LED on reception. Both sender and receiver use **FreeRTOS tasks** for concurrent operation.

---

## 📦 Hardware Used

### Sender:
- ESP32 Dev Board
- LoRa SX1276 module (via SPI)
- Onboard LED (GPIO 2)
- Wi-Fi access point (for web interface)

### Receiver:
- ESP32 Dev Board
- LoRa SX1276 module (via SPI)
- OLED Display (SSD1306, 128x32, I2C)
- Buzzer (GPIO 4)
- Onboard LED (GPIO 13)

---

## 📡 Connections

### LoRa SX1276 to ESP32
__________________________________________
| LoRa Pin | Sender GPIO | Receiver GPIO |
|----------|-------------|---------------|
| MISO     | 19          | 19            |
| MOSI     | 27          | 23            |
| SCK      | 5           | 5             |
| NSS / SS | 18          | 18            |
| RST      | 14          | 25            |
| DIO0     | 26          | 26            |
__________________________________________
### OLED Display (Receiver only)
_________________________
| OLED Pin | ESP32 GPIO |
|----------|------------|
| SDA      | 21         |
| SCL      | 22         |
_________________________

### Buzzer and LED (Receiver)
- Buzzer: GPIO 4
- Onboard LED: GPIO 13

---

## 🚦 FreeRTOS Integration

FreeRTOS is used for **non-blocking multitasking** on both ESP32 devices:

### Sender
- Web interface receives user input and enqueues messages.
- A FreeRTOS task (`sendLoRaTask`) dequeues and sends messages over LoRa.

### Receiver
- LoRa continuously listens for packets in the loop.
- A FreeRTOS task (`handleReceivedMessage`) dequeues received messages and:
  - Displays them on OLED.
  - Blinks onboard LED.
  - Activates the buzzer.

This architecture ensures smooth concurrent execution without blocking UI, LoRa, or output responses.

---

## 🌐 Web Interface

Hosted on the **sender ESP32**, accessible over Wi-Fi:

- Enter a message.
- Click "Send Message".
- Message is queued and sent over LoRa.


## 🔁 Setup Instructions

### 1. Install Arduino Libraries
- [ESPAsyncWebServer](https://github.com/me-no-dev/ESPAsyncWebServer)
- [AsyncTCP](https://github.com/me-no-dev/AsyncTCP)
- [LoRa](https://github.com/sandeepmistry/arduino-LoRa)
- [Adafruit SSD1306](https://github.com/adafruit/Adafruit_SSD1306)
- [Adafruit GFX](https://github.com/adafruit/Adafruit-GFX-Library)

### 2. Wiring
Connect both ESP32 boards with respective components based on the **Connections** section above.

### 3. Configure Wi-Fi
Update the `ssid` and `password` in `sender.ino` with your Wi-Fi credentials.

### 4. Upload
- Upload `sender.ino` to sender ESP32.
- Upload `receiver.ino` to receiver ESP32.

### 5. Access Web UI
- Check serial monitor for IP address.
- Open the IP in a browser to access the message input interface.

---

## ✅ Example Communication Flow

1. User enters message "Hello World!" on web interface.
2. Message queued and sent via LoRa with prefix `RTOS`.
3. Receiver receives it, strips `RTOS`, and:
   - Displays: `#1: Hello World!`
   - Buzzes twice and blinks LED.
   - Shows RSSI value on OLED.

---

## 🛠️ Future Enhancements

- Message logging on SD card
- Multiple receiver support
- RSSI-based distance estimation
- Bi-directional LoRa communication

---

## 📄 License

This project is open-source under the MIT License.

---

## 🤝 Contributing

Pull requests and suggestions are welcome!
