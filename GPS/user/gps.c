#include "esp_common.h"
#include "uart.h"
#include "lwip/mem.h"
#include "lwip/sockets.h"
#include "lwip/inet.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"


#define pheadbuffer "Connection: keep-alive\r\n\
Cache-Control: max-age=0\r\n\
Upgrade-Insecure-Requests: 1\r\n\
User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/69.0.3497.100 Safari/537.36 \r\n\
DNT: 1\r\n\
Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,image/apng,*/*;q=0.8\r\n\
Accept-Encoding: deflate\r\n\
Accept-Language: es-US,es;q=0.9,en-US;q=0.8,en;q=0.7,es-419;q=0.6\r\n\r\n"

#define DEMO_SERVER "104.31.85.110"
#define DEMO_SERVER_PORT 80

void get_data(char* info);
void Parser(char *word);

char JSON_DATA[200];
char result[20]={0};
char lat[30]={0};
char lon[30]={0};
char range_error[20]={0};

uint8 webname[] = {"api.mylnikov.org"};

uint8 MAC_ADDRES[10][20];

typedef enum {
  IDLE,
  INIT,
  BRACKET,
  LAT,
  LON,
  ERR,
  COMMA,
  END
} parse_state ;

void get_cordanates(void *pvParameters)
{
    int recbytes;
    int sin_size;
    int sta_socket;
    char recv_buf[1460];
	uint8_t BINARY	  =   0;
    //uint8 MAC_add[] = {"d8:37:be:da:41:ff"};
    uint8 *MAC_add = MAC_ADDRES[1];
    printf("MAC: %s\n",MAC_ADDRES);
    struct sockaddr_in remote_ip;
    //while (1) 
	//{
        //CHECK SOCKET STATUS
        sta_socket = socket(PF_INET, SOCK_STREAM, 0);
        if (-1 == sta_socket) 
		{

            close(sta_socket);
            printf("socket fail !\r\n");
           //continue;
        }
        printf("socket ok!\r\n");

        bzero(&remote_ip, sizeof(struct sockaddr_in));
        remote_ip.sin_family = AF_INET;
        remote_ip.sin_addr.s_addr = inet_addr(DEMO_SERVER);
        remote_ip.sin_port = htons(DEMO_SERVER_PORT);
        
        //CONNECT SOCKET
        if(0 != connect(sta_socket,(struct sockaddr *)(&remote_ip),sizeof(struct sockaddr)))
        {
            close(sta_socket);
            printf("connect fail!\r\n");
        }
        printf("connect ok!\r\n");

		//HTTP REQUEST
        char *pbuf = (char *) zalloc(512);
		sprintf(pbuf, "GET /geolocation/wifi?v=1.2&bssid=%s HTTP/1.1\r\nHost: %s\r\n"pheadbuffer"", MAC_add, webname);
        if (write(sta_socket,pbuf,strlen(pbuf)+1) < 0) 
		{
            close(sta_socket);
            printf("send fail\n");
            free(pbuf);
        }
        printf("send success\n");
        free(pbuf);
		//HERE GET THE HTTP PACKETS AND SAVE IT INTO THE FLASH IN ORDER TO LATER BOOT
        int i = 0;
        int size = 0;

        recbytes = read(sta_socket, recv_buf, 1460);
        get_data(recv_buf);
        Parser(JSON_DATA);

        if (recbytes < 0) 
		{
            printf("read data fail!\r\n");
            close(sta_socket);
        }
   // }
   vTaskDelete(NULL);
}
void get_data(char* info)
{
    
    short i=0;
    short j=0;
    bool end = false;
    bool begin = false;
    unsigned char brackets = 0;
    while(end == false)
    {
        //printf("%c",info[i]);
        if(info[i]=='{')
        {
            begin = true;
            brackets++;
        }
        if(info[i]=='}')
        {
            brackets--;
            if(brackets == 0)
            {
                end = true;
            }
        }
        if(begin == true)
        {
           JSON_DATA[j] = info[i];
           j++;
        }
        i++;
    }
}
void Parser(char *word)
{
    short i = 0;
    short j=0;
    parse_state state = IDLE;
    while(state !=END)
    {
        if(word[i]=='{' && state==IDLE)
        {
            state = INIT;
            i++;
        }
        if(state == INIT)
        {
            if(word[i]!=',')
            {
               result[j]=word[i];
               j++;
            }
            else
            {
                j=0;
                state = BRACKET;
            }
        }
        
        if(state == BRACKET)
        {
            if(word[i]=='{')
            {
                state = LAT;
                i++;
            }
        }
        
        if(state == LAT)
        {
            if(word[i]!=',')
            {
                if(word[i]!=' ')
                {
                    lat[j]=word[i];
                    j++;
                }
            }
            else
            {
                j=0;
                i++;
                state = ERR;
            }
        }
        if(state==ERR)
        {
            if(word[i]!=',')
            {
                if(word[i]!=' ')
                {
                    range_error[j]=word[i];
                    j++;
                }
            }
            else
            {
                range_error[j]=' ';
                range_error[j+1]='m';
                j=0;
                i++;
                state = LON;
            }
        }

        if(state == LON)
        {
            if(word[i]==',')
            {
                j=0;
                state = END;
            }
            if(word[i]!=' ' && word[i]!='}' && state !=END)
            {
                lon[j]=word[i];
                j++;
            }
        }
        
        i++;
    }
    printf("%s\n",result);
    printf("%s\n",lat );
    printf("%s\n",lon );
    printf("%s\n",range_error);
}