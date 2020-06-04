/**
 * IotWebConf05Callbacks.ino -- IotWebConf is an ESP8266/ESP32
 *   non blocking WiFi/AP web configuration library for Arduino.
 *   https://github.com/prampec/IotWebConf 
 *
 * Copyright (C) 2018 Balazs Kelemen <prampec+arduino@gmail.com>
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

/**
 * Example: Callbacks
 * Description:
 *   This example shows, what callbacks IotWebConf provides.
 *   (See previous examples for more details!)
 * 
 * Hardware setup for this example:
 *   - An LED is attached to LED_BUILTIN pin with setup On=LOW.
 *   - [Optional] A push button is attached to pin D2, the other leg of the
 *     button should be attached to GND.
 */
/***************************************************************************************************/
/* 
  This is an Arduino example for SHT21, HTU21D Digital Humidity & Temperature Sensor

  written by : enjoyneering79
  sourse code: https://github.com/enjoyneering/

  This sensor uses I2C bus to communicate, specials pins are required to interface
  Board:                                    SDA                    SCL
  Uno, Mini, Pro, ATmega168, ATmega328..... A4                     A5
  Mega2560, Due............................ 20                     21
  Leonardo, Micro, ATmega32U4.............. 2                      3
  Digistump, Trinket, ATtiny85............. 0/physical pin no.5    2/physical pin no.7
  Blue Pill, STM32F103xxxx boards.......... PB7*                   PB6*
  ESP8266 ESP-01:.......................... GPIO0/D5               GPIO2/D3
  NodeMCU 1.0, WeMos D1 Mini............... GPIO4/D2               GPIO5/D1

                                           *STM32F103xxxx pins B7/B7 are 5v tolerant, but bi-directional
                                            logic level converter is recommended

  Frameworks & Libraries:
  ATtiny Core           - https://github.com/SpenceKonde/ATTinyCore
  ESP8266 Core          - https://github.com/esp8266/Arduino
  ESP8266 I2C lib fixed - https://github.com/enjoyneering/ESP8266-I2C-Driver
  STM32 Core            - https://github.com/rogerclarkmelbourne/Arduino_STM32

  GNU GPL license, all text above must be included in any redistribution, see link below for details:
  - https://www.gnu.org/licenses/licenses.html
*/
/***************************************************************************************************/ 
#include <ESP8266WiFi.h>   //get mac adderss
#include <IotWebConf.h>
#include <ESP8266mDNS.h>
#include <ESP8266SSDP.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <LeweiClient.h>
#include <Wire.h>
#include <HTU21D.h>

// -- Initial name of the Thing. Used e.g. as SSID of the own Access Point.
const char thingName[] = "eMonitor";

// -- Initial password to connect to the Thing, when it creates an own Access Point.
const char wifiInitialApPassword[] = "12345678";

#define STRING_LEN 128

// -- Configuration specific key. The value should be modified if config structure was changed.
#define CONFIG_VERSION "1.04"

// -- When CONFIG_PIN is pulled to ground on startup, the Thing will use the initial
//      password to buld an AP. (E.g. in case of lost password)
#define CONFIG_PIN D7

// -- Status indicator pin.
//      First it will light up (kept LOW), on Wifi connection it will blink,
//      when connected to the Wifi it will turn off (kept HIGH).
#define STATUS_PIN LED_BUILTIN

String ipStr = "192.168.1.1";


//delay between updates
unsigned long previousMillis = 0; // 定义上一次loop到当前loop的时间间隔，数值类型为毫秒，变量类型为无符号长整型。
unsigned long previousMillis10s = 0; // 定义上一次loop到当前loop的时间间隔，数值类型为毫秒，变量类型为无符号长整型。
long interval = 60*1000; // 此处为1000毫秒 上传间隔调试完成将30改为合适的设置 比如60s 或者 300s


//hardware v1
#define ONE_WIRE_BUS D5
#define SDA D5
#define SCL D6

//hardware v2
// Data wire is plugged into port 2 on the Arduino
/*
#define ONE_WIRE_BUS D2
#define SDA D2
#define SCL D1
*/

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);
HTU21D myHTU21D(HTU21D_RES_RH12_TEMP14);

float T1;
float H1;
String SensorType = "No sensor";
String DataTH;

#define LW_USERKEY "xxxxxxxxxxxxxxxxxxxx"
#define LW_GATEWAY "01"
int commaPosition;  
char *KEY1;
char *KEY2;
char *p;

// -- Callback method declarations.
void wifiConnected();
void configSaved();
boolean formValidator();
void messageReceived(String &topic, String &payload);

DNSServer dnsServer;
WebServer server(80);
HTTPUpdateServer httpUpdater;

char SNValue[STRING_LEN];
char intervalValue[STRING_LEN];
char stringBuffer[STRING_LEN];

IotWebConf iotWebConf(thingName, &dnsServer, &server, wifiInitialApPassword, CONFIG_VERSION);
IotWebConfParameter SNParam = IotWebConfParameter("SN", "SN", SNValue, STRING_LEN);
IotWebConfSeparator separator1 = IotWebConfSeparator();
IotWebConfParameter intervalParam = IotWebConfParameter("Interval Time (s)", "interval", intervalValue, STRING_LEN,"number","60..3600","300","min='60' max='3600' step='1'");

LeWeiClient *lwc;

/********************************************\
|* 功能： 测试函数 不用可以删除              *|
\********************************************/
void hellotest(const char * test) //for test
{
     Serial.print("test const char ...");
       Serial.println(test);
}
/********************************************\
|* 功能： 初始化ssdp              *|
\********************************************/
void init_ssdp(){
    String ssdpname= "eMonitor_";
    int dd = strlen(SNValue);
    for(int i=(dd-8);i<dd;i++){
      ssdpname += SNValue[i];
    }
    //Serial.println(ssdpname);
    Serial.printf("Starting SSDP...\n");
    SSDP.setSchemaURL("description.xml");
    SSDP.setHTTPPort(80);
    //SSDP.setName("eMonitor_xxxxxx");
    SSDP.setName(ssdpname);
    SSDP.setSerialNumber("21254862");
    SSDP.setURL("/");
    SSDP.setModelName("Lewei Wifi eMonitor");
    SSDP.setModelNumber("1");
    SSDP.setModelURL("http://www.lewei50.com");
    SSDP.setManufacturer("lewei50");
    SSDP.setManufacturerURL("http://www.lewei50.com");
    SSDP.begin();
    Serial.println("Ready."); 
}
/********************************************\
|* 功能： 温湿度处理函数              *|
\********************************************/
void handle_ht(){
      if(myHTU21D.begin(SDA,SCL) == true){
          Serial.println(F("HTU21D, SHT21 sensor is active"));
          /* DEMO - 1 */
          /*  Serial.println(F("DEMO 1: 12-Bit Resolution"));
          Serial.print(F("Humidity............: ")); Serial.print(myHTU21D.readHumidity());            Serial.println(F(" +-2%"));
          Serial.print(F("Compensated Humidity: ")); Serial.print(myHTU21D.readCompensatedHumidity()); Serial.println(F(" +-2%"));
          
          Serial.println(F("DEMO 1: 14-Bit Resolution")); 
          Serial.print(F("Temperature.........: ")); Serial.print(myHTU21D.readTemperature()); Serial.println(F(" +-0.3C"));    
          */
          //H1 = myHTU21D.readHumidity();
          H1 = myHTU21D.readCompensatedHumidity(); //补偿湿度
          T1 = myHTU21D.readTemperature();
          SensorType = "HTU21D/SI7021 T1:" + String(T1) + "C H1:" + String(H1) + "%";
          DataTH = String(T1) + "," + String(H1);
      }else{
          sensors.begin();
          Serial.println(F("DS18B20 sensor is active")); 
          Serial.print("Requesting temperatures...");
          sensors.requestTemperatures(); // Send the command to get temperatures
          Serial.println("DONE");
          // We use the function ByIndex, and as an example get the temperature from the first sensor only.
          float tempC = sensors.getTempCByIndex(0);
          // Check if reading was successful
          if(tempC != DEVICE_DISCONNECTED_C) 
          {
            Serial.print("Temperature for the device 1 (index 0) is: ");
            Serial.println(tempC);
            T1 = tempC;
            H1 = 255;
            SensorType = "DS18B20 T1:" + String(T1) + "C";
            //DataTH = String(T1);
            DataTH = String(T1) + ",0"; //for bad HA
          } 
          else
          {
             T1 = 255;
             H1 = 255;
             SensorType = "No sensor";
             DataTH = "";
            Serial.println("Error: Could not read temperature data");
            Serial.println("Error: No sensor connect!");
          }   
      }
}
/********************************************\
|* 功能： 数据上传              *|
\********************************************/
void update_lw(){        
      if(T1 < 255){ 
          if (lwc) {
              Serial.print("---connect to lewei50.com ...... \r\n");
              //T1,H1.. must using the same name setting on lewei50.com .
              lwc->append("T1", T1);
              if (H1 < 255){
                lwc->append("H1", H1);
              }
              Serial.print("send ");
              lwc->send();
              Serial.print("---send completed \r\n");
          }
      }
}
boolean needReset = false;

void setup() 
{
  Serial.begin(9600);
  Serial.println();
  Serial.println("Starting up...");

  iotWebConf.setStatusPin(STATUS_PIN);
  iotWebConf.setConfigPin(CONFIG_PIN);
  iotWebConf.addParameter(&SNParam);
  iotWebConf.addParameter(&separator1);
  iotWebConf.addParameter(&intervalParam);
  iotWebConf.setConfigSavedCallback(&configSaved);
  iotWebConf.setFormValidator(&formValidator);
  iotWebConf.setWifiConnectionCallback(&wifiConnected);
  iotWebConf.setupUpdateServer(&httpUpdater);
  
  // -- Initializing the configuration.
  boolean validConfig = iotWebConf.init();
  if (!validConfig)
  {
    SNValue[0] = '\0';
  }

  // -- Set up required URL handlers on the web server.
  server.on("/", handleRoot);
  server.on("/config", []{ iotWebConf.handleConfig(); });
  // server.on("/index.html", HTTP_GET, []() {
  //  server.send(200, "text/plain", "Hello World!");
  //});
  server.on("/description.xml", HTTP_GET, []() {
    SSDP.schema(server.client());
  });  
  server.on("/monitorjson", monitorjson);
  server.onNotFound([](){ iotWebConf.handleNotFound(); });


  init_ssdp();//初始化ssdp

  if (strlen(SNValue)!=0){  
      lwc = new LeWeiClient(SNValue);
  } 
}

void loop() 
{
    if(WiFi.status() == WL_CONNECTED){
        // Set up mDNS responder:
        // - first argument is the domain name, in this example
        //   the fully-qualified domain name is "esp8266.local"
        // - second argument is the IP address to advertise
        //   we send our IP address on the WiFi network
        String domainname= "eMonitor_";
        int dd = strlen(SNValue);
        for(int i=(dd-8);i<dd;i++){
          domainname += SNValue[i];
        }
        if (!MDNS.begin(domainname)) {
          //Serial.println("Error setting up MDNS responder!");
         // while (1) {
          //  delay(1000);
          //}
        }
       // Serial.println("mDNS responder started");
        MDNS.update();
    }else{
      
    }

  
  // -- doLoop should be called as frequently as possible.
  iotWebConf.doLoop();

  if (needReset)
  {
    Serial.println("Rebooting after 1 second.");
    iotWebConf.delay(1000);
    ESP.restart();
  }

    IPAddress ip = WiFi.localIP();
    ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);

  unsigned long currentMillis = millis();   // 记录程序执行到此处的时间，数值类型为毫秒
  // 当前时间与上一次记录时间的差值，如果大于等于internal数值，则执行内部操作，否则进入下一次loop循环。
  if (currentMillis - previousMillis10s >= 5000) {   
      previousMillis10s = currentMillis;  
      //预执行的用户程序
      handle_ht();
  }
  //update datas to lewei50  
  interval = atoi(intervalValue) * 1000;
  if (currentMillis - previousMillis >= interval) {   
      previousMillis = currentMillis;  
      //预执行的用户程序
      update_lw(); 
  }  
  
}

/**
 * Handle web requests to "/" path.
 */
void handleRoot()
{
  // -- Let IotWebConf test and handle captive portal requests.
  if (iotWebConf.handleCaptivePortal())
  {
    // -- Captive portal request were already served.
    return;
  }
  String s = "<!DOCTYPE html><html lang=\"en\"><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1, user-scalable=no\"/>";
  s += "<title>Lewei50 HT monitor</title></head><body>HT monitor";
  s += "<ul>";
  s += "<li>IP: ";
  s += ipStr;
  s += "<li>";
  s += SensorType;
  s += "<li>SN value: ";
  s += SNValue;
  s += "<li>Interval Time: ";
  s += intervalValue;
  s += " second";
  s += "</ul>";
  s += "Go to <a href='config'>configure page</a> to change values.";
  s += "<ul>";
  s += "<li>(user:admin password:your password(default: 12345678))";
  s += "</ul>";
  s += "</body></html>\n";

  server.send(200, "text/html", s);
}

/**
 * Handle web requests to "/monitorjson" path.
 */ 
void monitorjson()
{
  // -- Let IotWebConf test and handle captive portal requests.
  if (iotWebConf.handleCaptivePortal())
  {
    // -- Captive portal request were already served.
    return;
  }
  String s = "{\"method\":\"uploadsn\",\"mac\":\"";
  s += WiFi.macAddress();
  s += "\",\"version\":\"";
  s += CONFIG_VERSION;
  s += "\",\"server\":\"HT\",\"SN\":\"";
  s += SNValue;
  s += "\",\"Data\":[";
  s += DataTH;
  s += "]}";
  
  server.send(200, "text/html", s);
}

void wifiConnected()
{
  Serial.println("WiFi was connected.");
}

void configSaved()
{
  Serial.println("Configuration was updated.");
  needReset = true;
}

boolean formValidator()
{
  Serial.println("Validating form.");
  boolean valid = true;

  int l = server.arg(SNParam.getId()).length();
  if (l < 3)
  {
    SNParam.errorMessage = "Please provide at least 3 characters for this test!";
    valid = false;
  }

  return valid;
}
