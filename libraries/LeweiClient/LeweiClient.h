#ifndef __LEWEICLIENT_H__
#define __LEWEICLIENT_H__

#include <ESP8266WiFi.h> 

class LeWeiClient
{
public:
    LeWeiClient(const char * user_key, const char *gateway);
    LeWeiClient(const char * user_sn);
//    int begin();
    int append(const char * name, int value);
    int append(const char * name, double value);
//    int end();
    int send();
private:
    char head[160];

    char * user_data;
    int user_str_length;

    bool begin;
    bool end;

  // Use WiFiClient class to create TCP connections
  WiFiClient client;
};

#endif /* end of include guard: __LEWEICLIENT_H__ */
