//#include <Arduino.h>
#include <stdio.h>

#include <LeweiClient.h>

#ifdef __ART__
#define malloc      rt_malloc
#define realloc     rt_realloc
#define free        rt_free

#define DEBUG_PRINTF    rt_kprintf
#else
// taken from http://playground.arduino.cc/Main/Printf
#include <stdarg.h>
static void _print(char *fmt, ... )
{
        char tmp[128]; // resulting string limited to 128 chars
        va_list args;
        va_start (args, fmt );
        vsnprintf(tmp, 128, fmt, args);
        va_end (args);
        Serial.print(tmp);
}
#define DEBUG_PRINTF    _print
#endif

//#define DEBUG_PRINTF(...)

char lwserver[] = "open.lewei50.com";

LeWeiClient::LeWeiClient(const char * user_key, const char * gateway)
{
    char *ptr = head;
    int head_length = 0;
    int tmp;

    user_data = NULL;
    user_str_length = 0;

    begin = false;
    end = false;

    // build head.
    tmp = sprintf(ptr,
                  "POST /api/V1/Gateway/UpdateSensors/%s HTTP/1.1\r\n",
                  gateway);
    head_length += tmp;
    ptr += tmp;

    // build userkey.
    tmp = sprintf(ptr,
                  "userkey: %s\r\n",
                  user_key);
    head_length += tmp;
    ptr += tmp;

    // build Host.
    tmp = sprintf(ptr, "Host: open.lewei50.com \r\n");
    head_length += tmp;
    ptr += tmp;

    // build User-Agent.
    tmp = sprintf(ptr, "User-Agent: RT-Thread ART\r\n");
    head_length += tmp;
    ptr += tmp;


}

LeWeiClient::LeWeiClient(const char * user_sn)
{
    char *ptr = head;
    int head_length = 0;
    int tmp;

    user_data = NULL;
    user_str_length = 0;

    begin = false;
    end = false;
    
    bool bUseSn = false;
    
    char *user_key;
	char *gateway;
	char *p;
    char stringBuffer[128];
    
    strcpy(stringBuffer,user_sn);
    if (strlen(stringBuffer)!=0){  
    // 将获取到的sn转换为usekey gateway
        user_key=strtok_r(stringBuffer,"_",&p); //usekey
        Serial.println(user_key);
        gateway=strtok_r(NULL,"_",&p); //gateway
        Serial.println(gateway);   
           
  //      hellotest(SNValue);   //for test
    if ((user_key != NULL)&&(gateway != NULL)){
      bUseSn = false;
    }else{
      bUseSn = true;
    }
  }

    // build head.
    if(bUseSn)
    {
    	tmp = sprintf(ptr,
                  "POST /api/V1/Gateway/UpdateSensorsBySN/%s HTTP/1.1\r\n",
                  user_sn);
    }
    else
	{
		tmp = sprintf(ptr,
                  "POST /api/V1/Gateway/UpdateSensors/%s HTTP/1.1\r\n",
                  gateway);
	    head_length += tmp;
	    ptr += tmp;
	    // build userkey.
	    tmp = sprintf(ptr,
	                  "userkey: %s\r\n",
	                  user_key);
	}
    head_length += tmp;
    ptr += tmp;

    // build Host.
    tmp = sprintf(ptr, "Host: open.lewei50.com \r\n");
    head_length += tmp;
    ptr += tmp;

    // build User-Agent.
    tmp = sprintf(ptr, "User-Agent: ESP8266\r\n");
    head_length += tmp;
    ptr += tmp;


}

int LeWeiClient::append(const char * name, int value)
{
    int length;
    int tmp;
    char * ptr;

    if(begin == false)
    {
        user_data = (char *)malloc(2);
        if(user_data == NULL)
        {
            return -1;
        }

        user_data[0] = '[';
        user_data[1] = 0;
        user_str_length = 1;

        begin = true;
    }

    if(user_data == NULL)
    {
        return -1;
    }

    length  = 23; /* >>{"Name":"","Value":""},<< */
    length += 8;  /* name */
    length += 10; /* value */

    ptr = (char *)realloc(user_data, user_str_length + length + 1);
    if(ptr == NULL)
    {
        return -1;
    }
    user_data = ptr;

    ptr = user_data + user_str_length;

    length = sprintf(ptr,
            "{\"Name\":\"%s\",\"Value\":\"%d\"},",
            name,
            value);

    user_str_length += length;
    DEBUG_PRINTF("append:%s\r\n", ptr);
}

int LeWeiClient::append(const char * name, double value)
{
    int length;
    int tmp;
    char * ptr;

    if(begin == false)
    {
        user_data = (char *)malloc(2);
        if(user_data == NULL)
        {
            DEBUG_PRINTF("malloc(2) ERROR!\r\n");
            return -1;
        }

        user_data[0] = '[';
        user_data[1] = 0;
        user_str_length = 1;

        begin = true;
    }

    if(user_data == NULL)
    {
        DEBUG_PRINTF("user_data == NULL!\r\n");
        return -1;
    }

    length  = 23; /* >>{"Name":"","Value":""},<< */
    length += 8;  /* name */
    length += 20; /* value: ab.cd */

    ptr = (char *)realloc(user_data, user_str_length + length + 1);
    if(ptr == NULL)
    {
        DEBUG_PRINTF("realloc == NULL!\r\n");
        return -1;
    }
    user_data = ptr; "", 

    ptr = user_data + user_str_length;

//    length = sprintf(ptr,
//            "{\"Name\":\"%s\",\"Value\":\"%d.%02u\"},",
//            name,
//            (int)value, (long)(abs(value)*100+0.5) % 100);

    length = sprintf(ptr,
            "{\"Name\":\"%s\",\"Value\":\"%.2f\"},",
            name,value);

    user_str_length += length;
    DEBUG_PRINTF("append:%s\r\n", ptr);
}

int LeWeiClient::send()
{
    int result = 0;
    char value_str[6] = "00.00";
    int total_len = 0;
    int tmp_len;
  
    if (client.connect(lwserver, 80))
    {	
        // send the HTTP PUT request:
        client.print(head);
        client.print("Content-Length: ");
        client.println(strlen(user_data) + 1); // ']'
        // last pieces of the HTTP PUT request:
        client.println("Connection: close");
        client.println();
        // post data
        client.print(user_data);
        client.println("]");

        DEBUG_PRINTF("data:%s]\r\n", user_data);
        result = 0;

        goto send_exit;
    }
    else
    {
        DEBUG_PRINTF("connect failed!\r\n");
        result = -1;
        goto send_exit;
    }

send_exit:
    client.stop();

    begin = false;
    end = false;

    if(user_data != NULL)
    {
        free(user_data);
    }

    return result;
}

