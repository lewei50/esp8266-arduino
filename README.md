Esp8266 arduino lib and application for lewei50/devicebit iot platform
===========================================

# Quick links

- [WATH8266](/WATH8266)
- [YNM3000](/YNM3000)
- [ESP8266PM25](/ESP8266PM25)
- [libraries](/libraries)
- [API list](https://www.lewei50.com/dev/apiList?version=1&sk=71)
- [lewei50 documentation](https://www.kancloud.cn/lewei50/lewei50-usermanual/811104)
- [devicebit for HomeAssistant](https://github.com/lewei50/homeassistant/tree/master/custom_components/devicebit)



# WATH8266/YNM3000/ESP8266PM25

WATH8266/YNM3000/ESP8266PM25 support for HomeAssistant, click [devicebit for HomeAssistant](https://github.com/lewei50/homeassistant/tree/master/custom_components/devicebit). Please updates the Latest firmware. 

### First use the program

Starting with 1.6.4, Arduino allows installation of third-party platform packages using Boards Manager. We have packages available for Windows, Mac OS, and Linux (32 and 64 bit).

- Use the [Flash Download Tools](https://www.espressif.com/en/support/download/other-tools) to download the Latest firmware.
- Restart and then you will find the AP eMonitor..
- Connect eMonitor.. use password 12345678 
- Open 192.168.4.1 >> config page >> set your WiFi SSID , WiFi password , SN >> Then Apply .
- Disconnect the eMonitor.. (* Only disconnect it they can into work mode)
- Open your computer network and you will find the equipment like eMonitor_sn , open it you can set the parameter

### if you have already use it and only update the program
- Plese connect WATH8266/YNM3000/ESP8266PM25 and then open its ip >> config page >> Firmware update.

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
- [FastCRC](https://github.com/FrankBoesing/FastCRC)
