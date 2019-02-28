#include "esp_common.h"
#include "uart.h"
#include "lwip/mem.h"
#include "lwip/sockets.h"
#include "lwip/inet.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "gps.h"

void get_data(char* info);
void Parser(char *word);

char JSON_DATA[200];
char result[25]={0};
char lat[35]={0};
char lon[35]={0};
char range_error[25]={0};
signed char Streght[10]={0};
uint8 MAC_ADDRES[10][20];
uint8 MAC_SIZE;

uint8 webname[] = {"api.mylnikov.org"};





void get_cordanates(void *pvParameters)
{
    int recbytes;
    int sin_size;
    int sta_socket;
    char recv_buf[1460];
	uint8_t BINARY	  =   0;
    struct sockaddr_in remote_ip;
    uint8_t i;
    //uint8 MAC_add[] = {"d8:37:be:da:41:ff"};
    uint8 *MAC_add ;
    signed char ssid;
    while (i<MAC_SIZE) 
	{
        memset(JSON_DATA,0,sizeof(JSON_DATA));
        memset(result,0,sizeof(result));
        memset(lat,0,sizeof(lat));
        memset(lon,0,sizeof(lon));
        memset(range_error,0,sizeof(range_error));
    
        printf("\r\n");
        printf("MAC number: %d\r\n",i);  
        //CHECK SOCKET STATUS
        MAC_add = MAC_ADDRES[i];
        printf("MAC: %s\r\n",MAC_add);
        ssid =Streght[i];
        printf("rssi: %i\r\n",ssid);
        sta_socket = socket(PF_INET, SOCK_STREAM, 0);
        if (-1 == sta_socket) 
		{

            close(sta_socket);
            printf("socket fail !\r\n");
           //continue;
        }
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

		//HTTP REQUEST
        char *pbuf = (char *) zalloc(512);
		sprintf(pbuf, "GET /geolocation/wifi?v=1.2&bssid=%s HTTP/1.1\r\nHost: %s\r\n"pheadbuffer"", MAC_add, webname);
        if (write(sta_socket,pbuf,strlen(pbuf)+1) < 0) 
		{
            close(sta_socket);
            printf("send fail\n");
            free(pbuf);
        }
        //printf("%s",pbuf);
        free(pbuf);
		//HERE GET THE HTTP PACKETS AND SAVE IT INTO THE FLASH IN ORDER TO LATER BOOT
        recbytes = read(sta_socket, recv_buf, 1460);
        http_parse(recv_buf);
        JSON_parse(JSON_DATA);

        if (recbytes < 0) 
		{
            printf("read data fail!\r\n");
            close(sta_socket);
        }
        i++;
    }
   vTaskDelete(NULL);
}
void http_parse(char* info)
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
void JSON_parse(char *word)
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