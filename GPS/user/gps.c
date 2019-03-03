#include "esp_common.h"
#include "uart.h"
#include "lwip/mem.h"
#include "lwip/sockets.h"
#include "lwip/inet.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "gps.h"
#include <string.h>
#include <stdlib.h>


//Router INFO
signed char Streght[MAXROUTERS]={0};
uint8 MAC_ADDRES[MAXROUTERS][20];
uint8 MAC_SIZE = 0;

//HTTP Data
uint8 webname[] = {"api.mylnikov.org"};
char recv_buf[1460];


//Cordenates variables
char JSON_DATA[200];
char *ptr = NULL;
bool exec = FALSE;
int lat_int;
int lat_dec;
int error;
int lon_int;
int lon_dec;
char *data;


void get_cordanates(void *pvParameters)
{
    //HTTP VARS
    int recbytes;
    int sin_size;
    int sta_socket;
    struct sockaddr_in remote_ip;
    char *pbuf = (char *) zalloc(512);
    //Control variables
    uint8_t i = 0;
    bool read = true;
    //DEBUG variables
    uint8 *MAC_add ;
    signed char ssid;
    while (i<MAC_SIZE) 
	{
        read = true;
        //memset(JSON_DATA,0,sizeof(JSON_DATA)-1);
    
        printf("\r\n");
        printf("MAC number: %d\r\n",i);  
        //CHECK SOCKET STATUS
        MAC_add = MAC_ADDRES[i];
        printf("MAC: %s\r\n",MAC_add);
        ssid = Streght[i];
        printf("rssi: %i\r\n",ssid);
        sta_socket = socket(PF_INET, SOCK_STREAM, 0);
        if (-1 == sta_socket) 
		{
            close(sta_socket);
            printf("socket fail !\r\n");
            read = false;
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
            read = false;
        }
        if(read == true)
        {
            //HTTP REQUEST
            sprintf(pbuf, "GET /geolocation/wifi?v=1.2&bssid=%s HTTP/1.1\r\nHost: %s\r\n"pheadbuffer"", MAC_add, webname);
            if (write(sta_socket,pbuf,strlen(pbuf)+1) < 0) 
            {
                close(sta_socket);
                printf("send fail\n");
                //free(pbuf);
                read = false;
            }
        }
        if(read == true)
        {
            //HERE GET THE HTTP PACKETS
            recbytes = read(sta_socket, recv_buf, 1460);
            http_parse(recv_buf);
            exec = Data_Result(JSON_DATA);
            if(exec)
            {
                lat_int = integer_part();
                lat_dec = decimal_part();
                error = integer_part();
                decimal_part();
                lon_int = integer_part();
                lon_dec = decimal_part();
                //data    = location(); TODO CHECK A DINAMIC ALLOCATION FOR THIS MESSAGE BECAUSE IS BIG
                printf("lat_int: %d\r\n",lat_int);
                printf("lat_dec: %d\r\n",lat_dec);
                printf("lon_int: %d\r\n",lon_int);
                printf("lon_dec: %d\r\n",lon_dec);
                printf("error: %d\r\n",error);
                //printf("extra info:%s\r\n",data);
            }
            else
            {
                printf("ERROR 404: Object was not found\r\n");
            }
        }
        else
        {
            printf("http fail!\r\n");
            close(sta_socket);
        }
        if (recbytes < 0) 
		{
            printf("bara data, buffer empty!\r\n");
            close(sta_socket);
        }
        vTaskDelay(500/portTICK_RATE_MS);
        i++;
    }
    free(pbuf);
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

bool Data_Result(char * string)
{
    bool valid = false;
    ptr = strtok(string, ":");
    ptr = strtok(NULL,",");
    int result = atoi( ptr );
    if(result == 200)
    {
        valid = true;
        ptr = strtok(NULL, ":");
    }
    else
    {
        valid = false;
    }
    return valid;
}
int integer_part(void)
{
    int val;
    ptr = strtok(NULL, ":");
    ptr = strtok(NULL, ".");
    val = atoi(ptr);
} 
int decimal_part(void)
{
    char* temp = (char*)malloc(7);
    memset(temp,0,7);
    int val =0;
    ptr = strtok(NULL, ",");
    memcpy ( temp, ptr, 6 );
    val = atoi (temp);
    free(temp);
    return val;
}
char * location(void)
{
    char *temp;
    temp = (char*) malloc (30);
    int size = 0;
    ptr = strtok(NULL, ":");
    ptr = strtok(NULL, ",");
    memset(temp,0,sizeof(temp)-1);
    sprintf(temp,"%s",ptr);
    return temp;
}