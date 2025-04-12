# AHT25 Telegram Bot 🌡️💧🤖

A simple Telegram bot that reads temperature and humidity data from an AHT25 sensor connected to a microcontroller (e.g., Raspberry Pi, ESP32/ESP8266) and sends updates or responds to commands via Telegram.

## Overview

This project allows you to remotely monitor the temperature and humidity readings from an AHT25 sensor using the convenience of a Telegram bot. You can request the current sensor readings on demand or potentially configure periodic updates or alerts (depending on implementation).

## Features

* Reads temperature (°C or °F) from the AHT25 sensor.
* Reads relative humidity (%) from the AHT25 sensor.
* Connects to the Telegram Bot API.).
* (Optional) Sends periodic updates to a specified chat ID.
* (Optional) Sends alerts if readings cross predefined thresholds.

## Hardware Requirements

* A microcontroller board (e.g., Raspberry Pi, ESP32, ESP8266).
* AHT25 Temperature and Humidity Sensor module.
* Jumper wires for connecting the sensor to the microcontroller.
* Power supply for the microcontroller.

## Software Requirements & Prerequisites

* **Programming Language:** C
* **Telegram Bot Token:** You need to create a bot using Telegram's @BotFather and get an API token.
* **I2C Enabled:** Ensure I2C communication is enabled on your microcontroller.
    * On Raspberry Pi: Use `sudo raspi-config` -> Interface Options -> I2C -> Enable.

## Installation

1.  **Clone the repository:**
    ```bash
    git clone [your-repository-url]
    cd aht25-telegram-bot
    ```
3.  **Hardware Connection:**
    * Connect the AHT25 sensor to your microcontroller's I2C pins:
        * `VIN` or `VCC` to 3.3V or 5V (check sensor datasheet)
        * `GND` to Ground (GND)
        * `SCL` to the SCL pin of your board
        * `SDA` to the SDA pin of your board
    * Ensure you are using the correct I2C bus if your board has multiple.
## License

This project is licensed under the MIT.

## Acknowledgements

* Mention any libraries, guides, or individuals that helped significantly.
* [Adafruit AHTx0 Library]()
* [UniversalTelegramBot Library]()  

---
