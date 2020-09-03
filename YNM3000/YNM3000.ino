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
#include <FastCRC.h> //for S8

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


const uint8_t Hmi_End[3] = {0xff,0xff,0xff};
const uint8_t hcho_cmd[7] = {0x42,0x4d,0x01,0x00,0x00,0x00,0x90};  
const uint8_t co2_cmd[8] = {0xfe,0x04,0x00,0x03,0x00,0x01,0xd5,0xc5};  
unsigned char  Pm_data[32];
unsigned int check;
uint8_t wifiState=4;
uint16_t PM25, PM100, AQI, HCHO=0x3031,CO2=0,HCHO1=0,model=0,timeout1=0,timeout2=0,timeout3=0;
unsigned char sermode = 0, pagenum=0, lastpagenum=0 ,pageflushcount=0;

#define AQI_BASE_US             0
#define AQI_BASE_CN             1

#define AQI_DATA                0
#define AQI_LEVEL               1
#define AQI_MAIN_POLLUTANT      2

#define POLLUTANT_PM2_5         0
#define POLLUTANT_PM10          1

const uint16_t AQIindex[8][6] = {
    {0,     0,      0,      0,      0,      0},
    {120,   350,    540,    500,    500,    0},//35 50
    {354,   750,    1540,   1500,   1000,   1},//75 150
    {554,   1150,   2540,   2500,   1500,   2},//115 250
    {1504,  1500,   3540,   3500,   2000,   3},//150 350
    {2504,  2500,   4240,   4200,   3000,   4},//250 420
    {3504,  3500,   5040,   5000,   4000,   5},//350 500
    {5004,  5000,   6040,   6000,   5000,   5}//500 600C I
};
uint16_t AQIBUFFER[2][3] = {{0,0,0},{0,0,0}};

void chgmode(unsigned char tempmode);
uint16_t getAQI(uint8_t _base);
uint8_t getAQILevel(uint8_t _base);
String getMainPollu(uint8_t _base);
void parseAQI();
String level2cn(uint16_t level);
void refreshPage();
void serialdata();

//hardware v1
#define ONE_WIRE_BUS D5
#define SDA D5
#define SCL D6
#define Sel_A D1
#define Sel_B D2
#define CS 2

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
String jsondata;

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
void hmi_txt_send(String sensorname, float tempdata, unsigned int num, String unit);

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

void handle_ht();
void init_ssdp();
void update_lw();

FastCRC16 CRC16;
LeWeiClient *lwc;

boolean needReset = false;


/********************************************\
|* 功能： 测试函数 不用可以删除              *|
\********************************************/
void hellotest(const char * test) //for test
{
     Serial.print("test const char ...");
       Serial.println(test);
}



void setup() 
{
  pinMode(CS, OUTPUT);
  digitalWrite(CS, LOW); 
  delay(1000);
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

 
  pinMode(Sel_A, OUTPUT);
  pinMode(Sel_B, OUTPUT);
  digitalWrite(Sel_A, LOW);
  digitalWrite(Sel_B, LOW);
delay(1000);
  Serial.write(Hmi_End,3);
    Serial.print("info.txt=\"PM monitor\"");
    Serial.write(Hmi_End,3);
    Serial.print("page 1");
    Serial.write(Hmi_End,3);
    Serial.print("deviceName.txt=\"AIR monitor\"");
    Serial.write(Hmi_End,3);
delay(1000);
   
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
  
  while (Serial.available()>0) 
    {   
      serialdata();
     }

    IPAddress ip = WiFi.localIP();
    ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);

  unsigned long currentMillis = millis();   // 记录程序执行到此处的时间，数值类型为毫秒,当前时间与上一次记录时间的差值，如果大于等于internal数值，则执行内部操作，否则进入下一次loop循环。
  if (currentMillis - previousMillis10s >= 1000) {   
      previousMillis10s = currentMillis;  //预执行的用户程序

      handle_ht();  //温湿度读取函数 DS18B20/HTU21D/SI7021
      sermode ++;
      if (sermode >3) sermode = 0; 
      chgmode(sermode); //0:hmi 1:pm25 2:hcho 3:co2
      if (sermode == 2) Serial.write(hcho_cmd,7); //发送读取攀藤hcho指令,单独的甲醛模块
      if (sermode == 3) Serial.write(co2_cmd,8); //发送读取S8-CO2指令,单独的CO2模块
      if (sermode == 0) refreshPage();  //进入读取hmi模式
      if(HCHO<0x3000){
        HCHO1=HCHO;  
      }else{
        HCHO1=0;  
      }
      SensorType = "T1:" + String(T1) + "C H1:" + String(H1) + "% PM2.5:" + String(PM25) + "ug/m3 AQI:" + String(AQI) + " HCHO:" + String(float(HCHO1)/1000) + "mg/m3 CO2:" + String(CO2) + "ppm";
      jsondata = String(T1) + "," + String(H1) + "," + String(PM25) + "," + String(AQI) + "," + String(float(HCHO1)/1000) + "," + String(CO2);  //[T,H,pm25,aqi,hcho,co2]
  }
   
  interval = atoi(intervalValue) * 1000;
  if (currentMillis - previousMillis >= interval) {   
      previousMillis = currentMillis;  
      update_lw(); //update datas to lewei50 
  }  
  
}
/**********************************************************************
* 函数名: unsigned char FucCheckSum(uchar *i,uchar ln)
* 功能描述:求和校验（取发送、接收协议的1\2\3\4\5\6\7的和取反+1）
* 函数说明:将数组的元素1-倒数第二个元素相加后取反+1（元素个数必须大于2）
**********************************************************************/
unsigned char FucCheckSum(unsigned char *i,unsigned char ln)
{
  unsigned char j,tempq=0;
  i+=1;
  for(j=0;j<(ln-2);j++)
  {
    tempq+=*i;
    i++;
  }
  tempq=(~tempq)+1;
  return(tempq);
}
/**
 * serial data read and check
 */ 
void serialdata()
{
      Pm_data[0]=Serial.read();
      delay(2);
      if(sermode == 1){
        timeout1++;
        if(Pm_data[0]==0x32 ||Pm_data[0]==0x42 ){
          Pm_data[1]=Serial.read();delay(2);
          if(Pm_data[1]==0x3d ||Pm_data[1]==0x4d ){
            for(unsigned char k=2;k<32;k++){
              Pm_data[k]=Serial.read();
              if(k<30){check=check +Pm_data[k];}
              delay(2);
            }
          }
          if(Pm_data[0]==0x32 && Pm_data[1]==0x3d ){
            check=check+111;
            if(check==(Pm_data[30]*256+Pm_data[31])){ //A4
              PM25 = Pm_data[6]*256+Pm_data[7];
              PM100 = Pm_data[8]*256+Pm_data[9];
              //HCHO = Pm_data[28]*256+Pm_data[29];
              parseAQI();
              timeout1=0;
            }
          }
          if(Pm_data[0]==0x42 && Pm_data[1]==0x4d )
          {
            check=check+143;
            if(check==(Pm_data[30]*256+Pm_data[31])){ //PM5003s
              PM25 = Pm_data[12]*256+Pm_data[13];
              PM100 = Pm_data[14]*256+Pm_data[15];
              HCHO = Pm_data[28]*256+Pm_data[29];
              parseAQI();
              timeout1=0;
            }
          }
         check=0;   
        } 
        if(timeout1 > 6){
          timeout1=0;
        }
      }
      //hcho
      if(sermode == 2){
        timeout2++;
        //DS-HCHO
        if(Pm_data[0]==0x42 ){
          Pm_data[1]=Serial.read();delay(2);
          if(Pm_data[1]==0x4d ){
            for(unsigned char k=2;k<10;k++){
              Pm_data[k]=Serial.read();
              if(k<8){check=check +Pm_data[k];}
              delay(2);
            }
          }
          check=check+143;
          if(check==(Pm_data[8]*256+Pm_data[9])){ 
            HCHO = (Pm_data[6]*256+Pm_data[7])*pow(10, (4 - Pm_data[5]));
            timeout2=0;
          }
         check=0;   
        } 
      
        //HH-HCHO-M/WZ-S/ZE07/ZE08
        if(Pm_data[0]==0xff ){
          Pm_data[1]=Serial.read();delay(2);
          if(Pm_data[1]==0x17 ){
            for(unsigned char k=2;k<9;k++){
              Pm_data[k]=Serial.read();
              delay(2);
            }
            if(FucCheckSum(Pm_data,9)==Pm_data[8]){ 
              HCHO = (Pm_data[4]*256+Pm_data[5]);  // *pow(10, Pm_data[3]); //查手册小数位数
              timeout2=0;
            }       
          }  
        } 
          
        if(timeout2 > 6){
          timeout2=0;
          //HCHO = 0x3031;
        }          
      }
      //co2
      if(sermode == 3){
        timeout3++;
        if(Pm_data[0]==0xfe ){
          Pm_data[1]=Serial.read();delay(2);
          if(Pm_data[1]==0x04 ){
            for(unsigned char k=2;k<7;k++){
              Pm_data[k]=Serial.read();
              delay(2);
            }
          }
          //model=CRC16.modbus(Pm_data, 5);
            if(CRC16.modbus(Pm_data, 5)==(Pm_data[6]*256+Pm_data[5])){ //S8-CO2 *注意CRC的高低位
              CO2 = (Pm_data[3]*256+Pm_data[4]);
              timeout3=0;
            }
        }
        if(timeout3 > 6){
          timeout3=0;
          CO2 = 0;
        }             
      }
      if(HCHO < 0x3000 ){
        pagenum = 4;  //pm + hcho
      }else{
        pagenum = 1;  //pm
      }
      if(CO2 > 0 ){
         if(HCHO < 0x3000 )
          {
            pagenum = 6;  //PM + hcho + co2
          }else{
            pagenum = 5;  //PM + co2
          }
      }
}
/**
 * format hmi send data
 */ 
void hmi_txt_send(String sensorname, float tempdata, unsigned int num, String unit){
   Serial.print(sensorname);
   Serial.print(".txt=\"");
   Serial.print(tempdata,num);
   Serial.print(unit);
   Serial.print("\"");
   Serial.write(Hmi_End,3);
}
/**
 * reflush hmi page datas
 */ 
void refreshPage()
{    
    if( lastpagenum != pagenum ){
      if(pagenum==1)Serial.print("page 1");Serial.write(Hmi_End,3); //PM25+HT
      if(pagenum==4)Serial.print("page 4");Serial.write(Hmi_End,3); //PM25+HT+hcho
      if(pagenum==5)Serial.print("page 5");Serial.write(Hmi_End,3); //PM25+HT+co2
      if(pagenum==6)Serial.print("page 6");Serial.write(Hmi_End,3); //PM25+HT+hcho+co2
      pageflushcount++;
      if(pageflushcount>2){lastpagenum = pagenum;pageflushcount=0;}
    }
    
    if(T1<255)hmi_txt_send("temp",T1,1,"C");
    if(H1<255)hmi_txt_send("hum",H1,1,"%");
    if( HCHO < 0x3000 )hmi_txt_send("HCHO",float(HCHO)/1000 ,3,"mg");  //mg/m3
    if( CO2 > 0 )hmi_txt_send("CO2",CO2,0,"ppm");
    String pmleveltmp =  ( " " + getMainPollu(1) + " L" + String(getAQILevel(1)) );    
    hmi_txt_send("pm25",PM25,0, pmleveltmp );   
      Serial.print("level.val=");
      Serial.print(getAQILevel(1)*20 );
      Serial.write(Hmi_End,3);    
    AQI = getAQI(1);
    hmi_txt_send("aqi",AQI,0,"");
    
    if(WiFi.status() == WL_CONNECTED){
      wifiState=5;
    }else{
      wifiState=4;
    }
    Serial.print("wifiState.pic=");
    Serial.print(wifiState);
    Serial.write(Hmi_End,3);   
    Serial.print("info.txt=\"");
    Serial.print(WiFi.localIP());
    Serial.print(" ");
    Serial.print(WiFi.RSSI());
    Serial.print("dBm\"");
    Serial.write(Hmi_End,3);
    Serial.write(Hmi_End,3);
    Serial.print("deviceName.txt=\"AIR monitor\"");
    Serial.write(Hmi_End,3);
}
/**
 * select serial with A,B for change hmi,pm25,hcho,co2
 */ 
void chgmode(unsigned char tempmode){
   switch(tempmode){
     case 0:  //hmi
       digitalWrite(Sel_A, LOW);
       digitalWrite(Sel_B, LOW);
       break;
     case 1:  //pm25
       digitalWrite(Sel_A, HIGH);
       digitalWrite(Sel_B, LOW);
     break;
     case 2:  //hcho
       digitalWrite(Sel_A, LOW);
       digitalWrite(Sel_B, HIGH);
     break;
     case 3:  //co2
       digitalWrite(Sel_A, HIGH);
       digitalWrite(Sel_B, HIGH);
     break;
     default:
       digitalWrite(Sel_A, LOW);
       digitalWrite(Sel_B, LOW);
       break;
   }
}
/**
 * level to cn
 */ 
String level2cn(uint16_t level)
{ 
   switch(level){
     case 0:  
       return "优";
       break;
     case 1: 
       return "良";
     break;
     case 2:  
       return "轻度污染";
     break;
     case 3: 
       return "中度污染";
     break;
     case 4: 
       return "重度污染";
     break;
     case 5: 
       return "严重污染";
     break;
     default:
       return "优";
       break;
   }
}
/**
 * get AQI
 */ 
uint16_t getAQI(uint8_t _base) {
    if (_base >= AQI_BASE_US && _base <= AQI_BASE_CN) {
        return AQIBUFFER[_base][AQI_DATA];
    }
    else {
        return AQIBUFFER[AQI_BASE_US][AQI_DATA];
    }
}
/**
 * get AQI level
 */ 
uint8_t getAQILevel(uint8_t _base) {
    if (_base >= AQI_BASE_US && _base <= AQI_BASE_CN) {
        return AQIBUFFER[_base][AQI_LEVEL];
    }
    else {
        return AQIBUFFER[AQI_BASE_US][AQI_LEVEL];
    }
}
/**
 * get main pollu
 */ 
String getMainPollu(uint8_t _base) {
    if (_base >= AQI_BASE_US && _base <= AQI_BASE_CN) {
        return AQIBUFFER[_base][AQI_MAIN_POLLUTANT] ? "PM10" : "PM2.5";
    }
    else {
        return AQIBUFFER[AQI_BASE_US][AQI_MAIN_POLLUTANT] ? "PM10" : "PM2.5";
    }
}
/**
 * parse PM to AQI
 */ 
void parseAQI() {
    uint16_t AQI25, AQI100, color;
    for (uint8_t Bnum = 0; Bnum < 2; Bnum++) {
    // uint8_t Bnum = 0;
        AQI25 = 0;
        AQI100 = 0;
        for (uint8_t Inum = 1; Inum < 8; Inum++) {
            if (PM25*10 <= AQIindex[Inum][0+Bnum]) {
                // IOT_DEBUG_PRINT4(F("Inum: "), Inum, F("Bnum: "), Bnum);
                // IOT_DEBUG_PRINT2(F("AQIindex[Inum][0+Bnum]: "), AQIindex[Inum][0+Bnum]);
                AQI25 = ((AQIindex[Inum][4] - AQIindex[Inum-1][4])*(PM25*10 - AQIindex[Inum-1][0+Bnum]) / (AQIindex[Inum][0+Bnum] - AQIindex[Inum - 1][0+Bnum]) + AQIindex[Inum-1][4])/10;
                color = AQIindex[Inum][5];
                break;
            }
            if (Inum == 7) {
                AQI25 = 500;
                color = 5;
            }
        }
        for (uint8_t Inum = 1; Inum < 8; Inum++) {
            if (PM100*10 <= AQIindex[Inum][2+Bnum]) {
                // IOT_DEBUG_PRINT4(F("Inum: "), Inum, F("Bnum: "), Bnum);
                // IOT_DEBUG_PRINT2(F("AQIindex[Inum][0+Bnum]: "), AQIindex[Inum][0+Bnum]);
                AQI100 = ((AQIindex[Inum][4] - AQIindex[Inum-1][4])*(PM100*10 - AQIindex[Inum-1][2+Bnum]) / (AQIindex[Inum][2+Bnum] - AQIindex[Inum - 1][2+Bnum]) + AQIindex[Inum-1][4])/10;
                
                // IOT_DEBUG_PRINT4(F("AQI25: "), AQI25, F("  AQI100: "), AQI100);
                if(AQI25 >= AQI100) {
                    // return String(AQI25);
                    AQIBUFFER[Bnum][AQI_DATA] = AQI25;
                    AQIBUFFER[Bnum][AQI_LEVEL] = color;
                    AQIBUFFER[Bnum][AQI_MAIN_POLLUTANT] = POLLUTANT_PM2_5;
                    break;
                }
                else {
                    AQIBUFFER[Bnum][AQI_DATA] = AQI100;
                    color = AQIindex[Inum][5];
                    AQIBUFFER[Bnum][AQI_LEVEL] = color;
                    AQIBUFFER[Bnum][AQI_MAIN_POLLUTANT] = POLLUTANT_PM10;
                    break;
                    // return String(AQI100);
                }
            }
            if (Inum == 7) {
                AQIBUFFER[Bnum][0] = 500;
                AQIBUFFER[Bnum][1] = 5;
            }
        }
    }
}

/********************************************\
|* 功能： 温湿度处理函数              *|
\********************************************/
void handle_ht(){
      if(myHTU21D.begin(SDA,SCL) == true){
          //Serial.println(F("HTU21D, SHT21 sensor is active"));
          /* DEMO - 1 */
          /*  Serial.println(F("DEMO 1: 12-Bit Resolution"));
          Serial.print(F("Humidity............: ")); Serial.print(myHTU21D.readHumidity());            Serial.println(F(" +-2%"));
          Serial.print(F("Compensated Humidity: ")); Serial.print(myHTU21D.readCompensatedHumidity()); Serial.println(F(" +-2%"));
          
          Serial.println(F("DEMO 1: 14-Bit Resolution")); 
          Serial.print(F("Temperature.........: ")); Serial.print(myHTU21D.readTemperature()); Serial.println(F(" +-0.3C"));    
          */
          //H1 = myHTU21D.readHumidity();
          H1 = myHTU21D.readCompensatedHumidity(); //补偿湿度
          H1 = float( int(H1*100) )/ 100;
          T1 = myHTU21D.readTemperature();
          T1 = float( int(T1*100) )/ 100;
      }else{
          sensors.begin();
          //Serial.println(F("DS18B20 sensor is active")); 
          //Serial.print("Requesting temperatures...");
          sensors.requestTemperatures(); // Send the command to get temperatures
          //Serial.println("DONE");
          // We use the function ByIndex, and as an example get the temperature from the first sensor only.
          float tempC = sensors.getTempCByIndex(0);
          // Check if reading was successful
          if(tempC != DEVICE_DISCONNECTED_C) 
          {
           // Serial.print("Temperature for the device 1 (index 0) is: ");
          //  Serial.println(tempC);
            T1 = tempC;
            H1 = 255;
          } 
          else
          {
             T1 = 255;
             H1 = 255;
           // Serial.println("Error: Could not read temperature data");
          //  Serial.println("Error: No sensor connect!");
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
              lwc->append("dust", PM25);
              lwc->append("AQI", AQI);
              if (HCHO < 0x3000){
                lwc->append("hcho", float(HCHO)/1000);
              }
              if (CO2 > 0){
                lwc->append("CO2", CO2);
              }
              Serial.print("send ");
              lwc->send();
              Serial.print("---send completed \r\n");
          }
      }
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
  s += "<title>Lewei50 PM monitor</title></head><body>YNM3000";
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
  s += "\",\"server\":\"PM\",\"SN\":\"";
  s += SNValue;
  s += "\",\"Data\":[";
  s += jsondata;
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
