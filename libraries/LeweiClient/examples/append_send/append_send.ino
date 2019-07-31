/*
   lewei50 open platform sensor client
   This code is in the public domain.
  */
#include <ESP8266WiFi.h> 
#include <LeweiClient.h>

#ifndef STASSID
#define STASSID "your-ssid"
#define STAPSK  "your-password"
#endif

const char* ssid     = STASSID;
const char* password = STAPSK;

#define LW_USERKEY "xxxxxxxxxxxxxxxxxxxx"
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
    Serial.print("--- start data collection --- \r\n");
    //T1,H1.. must using the same name setting on lewei50.com .
    lwc->append("T1", 22.32);
    lwc->append("H1", 56.65);
    Serial.print("send ");
    lwc->send();
    //Grammar changed by Wei&Anonymous ;)
    Serial.print("--- send completed --- \r\n");
 
   delay(POST_INTERVAL);
   }
 }