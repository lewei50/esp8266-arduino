/*
   Devicebit open platform sensor client
   This code is in the public domain.
  */
 
#include <ESP8266WiFi.h> 
#include <LeweiClient.h>
#include <SHT1x.h>

#ifndef STASSID
#define STASSID "your-ssid"
#define STAPSK  "your-password"
#endif

const char* ssid     = STASSID;
const char* password = STAPSK;

// Specify data and clock connections and instantiate SHT1x object
#define dataPin 4
#define clockPin 5
SHT1x sht1x(dataPin, clockPin);
 
//put your api key here,find it in lewei50.com->my account->account setting
#define LW_USERKEY "YOUR_API_KEY_HERE"
//put your gateway number here,01 as default
#define LW_GATEWAY "01"
 
//delay between updates
#define POST_INTERVAL (30*1000)
 
LeWeiClient *lwc;
 
 
void setup() {
  Serial.begin(115200);

  // We start by connecting to a WiFi network
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  
   lwc = new LeWeiClient(LW_USERKEY, LW_GATEWAY);
   
   //lwc = new LeWeiClient(LW_USERKEY, LW_GATEWAY,ssid,password);
 }
 
void loop() {
   // read the analog sensor:
   //int sensorReading = analogRead(A0);  
 
  // if there's incoming data from the net connection.
   // send it out the serial port.  This is for debugging
   // purposes only:
 
  if (lwc) {
    Serial.println("read data ");
    
    float temp_c;
    float temp_f;
    float humidity;
    
    // Read values from the sensor
    temp_c = sht1x.readTemperatureC();
    temp_f = sht1x.readTemperatureF();
    humidity = sht1x.readHumidity();
    //T1,H1.. must using the same name setting on web server.
    lwc->append("T1", temp_c);
    lwc->append("H1", humidity);
    //Serial.print("*** data send ***");
    lwc->send();
    //Grammar changed by Wei&Anonymous ;)
    Serial.print("--- send completed --- \r\n");
 
   delay(POST_INTERVAL);
   }
 }
