Esp8266 arduino lib and application for lewei50/devicebit iot platform
===========================================

# Quick links

- [WATH8266](/WATH8266)
- [libraries](/libraries)
- [API list](https://www.lewei50.com/dev/apiList?version=1&sk=71)
- [lewei50 documentation](https://www.kancloud.cn/lewei50/lewei50-usermanual/811104)

# Arduino on ESP8266

This project brings support for ESP8266 chip to the Arduino environment. It lets you write sketches using familiar Arduino functions and libraries, and run them directly on ESP8266, no external microcontroller required.

ESP8266 Arduino core comes with libraries to communicate over WiFi using TCP and UDP, set up HTTP, mDNS, SSDP, and DNS servers, do OTA updates, use a file system in flash memory, work with SD cards, servos, SPI and I2C peripherals.

### Installing with Boards Manager

Starting with 1.6.4, Arduino allows installation of third-party platform packages using Boards Manager. We have packages available for Windows, Mac OS, and Linux (32 and 64 bit).

- Install the current upstream Arduino IDE at the 1.8.7 level or later. The current version is on the [Arduino website](https://www.arduino.cc/en/main/software).
- Start Arduino and open the Preferences window.
- Enter ```https://arduino.esp8266.com/stable/package_esp8266com_index.json``` into the *Additional Board Manager URLs* field. You can add multiple URLs, separating them with commas.
- Open Boards Manager from Tools > Board menu and install *esp8266* platform (and don't forget to select your ESP8266 board from Tools > Board menu after installation).

### Useing libraries

- [LeweiClient](/libraries/LeweiClient)
- [IotWebConf](https://github.com/prampec/IotWebConf)
- [ESP8266SSDP](/libraries/ESP8266SSDP)
- [Wire](https://www.arduino.cc/en/Reference/Wire)
- [HTU21D](https://github.com/enjoyneering/HTU21D)
- [OneWire](https://github.com/PaulStoffregen/OneWire)
- [DallasTemperature](https://github.com/milesburton/Arduino-Temperature-Control-Library)
