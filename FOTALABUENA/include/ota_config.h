#ifndef __OTA_CONFIG_H__
#define __OTA_CONFIG_H__

#define DEMO_SERVER "192.168.0.113"
#define DEMO_SERVER_PORT 80

#define DEMO_WIFI_SSID     "AXTEL-XTREMO-4157"
#define DEMO_WIFI_PASSWORD  "89615461"

#define OTA_TIMEOUT 120000  //120000 ms

#define pheadbuffer "Connection: keep-alive\r\n\
Save-Data: on\r\n\
Upgrade-Insecure-Requests: 1\r\n\
User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/69.0.3497.100 Safari/537.36 \r\n\
DNT: 1\r\n\
Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,image/apng,*/*;q=0.8\r\n\
Accept-Encoding: gzip, deflate\r\n\
Accept-Language: es-US,es;q=0.9,en-US;q=0.8,en;q=0.7,es-419;q=0.6\r\n\r\n"


#endif

//Cache-Control: no-cache\r\n\