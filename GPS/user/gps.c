#include "esp_common.h"
#include "uart.h"
#include "lwip/mem.h"
#include "lwip/sockets.h"
#include "lwip/inet.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "gps.h"


uint32_t ten[]={1,10,100,1000,10000,100000,1000000,10000000,100000000,1000000000};

char JSON_DATA[200];
char result[20]={0};
char lat[30]={0};
char lon[30]={0};
char range_error[20]={0};

uint8 webname[] = {"api.mylnikov.org"};
//source data 
uint8 MAC_ADDRES[10][20];
char RSSI[10];
uint8 MAC_SIZE;

//decimal part
    //constants
int16_t set_lat = 20;
int16_t set_long = -103;
    //if contants not matches data is ignored, this may vary in each city
//variables
uint8_t decimal_index; // array index
//for data filter
int16_t integer_lat;
int16_t integer_lon;
//data get by strings
uint32_t decimal_lat[10];
uint32_t decimal_log[10];


void get_cordanates(void *pvParameters)
{
    int recbytes;
    int sin_size;
    int sta_socket;
    char recv_buf[1460];
	uint8_t BINARY	  =   0;
    struct sockaddr_in remote_ip;
    uint8_t i;
    uint8 *MAC_add;   
    while (i<MAC_SIZE) 
	{
        memset(result,'\0',sizeof(result));
        memset(lat,'\0',sizeof(lat));
        memset(lon,'\0',sizeof(lon));
        memset(range_error,'\0',sizeof(range_error));

        //CHECK SOCKET STATUS
        MAC_add = MAC_ADDRES[i];
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

        printf("MAC: %s\r\n",MAC_add);
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
        //printf("%s",pbuf);
        free(pbuf);
		//HERE GET THE HTTP PACKETS AND SAVE IT INTO THE FLASH IN ORDER TO LATER BOOT
        recbytes = read(sta_socket, recv_buf, 1460);
        get_data(recv_buf);
        Parser(JSON_DATA);

        if (recbytes < 0) 
		{
            printf("read data fail!\r\n");
            close(sta_socket);
        }
        i++;
    }
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
bool Parse_Result(char* str)
{
    uint8_t i;
    uint8_t data_len;
    parse_state state = IDLE;
    char data[3];
    int32_t assigned_value;
    while(state !=END)
    {
        if(str[i]==':')
        {
            state = DOBLE_DOT;
        }
        if(state == DOBLE_DOT)
        {
            data[i]=str[i];
            data_len++;
            if(data_len == 3)
            {
                state= END;
            }
        }
        i++;
    }
    assigned_value = string_to_int(data);
    if(assigned_value == 200)
    {
        return true;
    }
    if(assigned_value == 400)
    {
        return false;
    }
    return false;
}

char[4] integer_part;//after point
char[7] decimal_val;//after point

int32_t decimal;
int32_t integer;

void Parse_Cordenate(char* str)
{
    uint8_t i;
    uint8_t j;
    uint8_t len;
    int32_t val;
    parse_state state = IDLE;
    while(state !=END)
    {
       if(str[i]==':')
        {
	    state = DOBLE_DOT;
	}
	if( state == DOBLE_DOT)
	{
	    integer_part[j] = str[i];
	    j++;
	    if(str[i]=='.')
	    {
	       state = DOT;
	       j=0;
	       integer = string_to_int(integer_part);
	    }
	}

	if(state == DOT)
	{
	    decimal_val[j] = str[i];
	    j++;
	    if(j==6)
	    {
		decimal = string_to_int(decimal_val);
	    	state= END;
	    }
	}
	i++;
    }
    
    return val;
}
int32_t string_to_int(char* string)
{
    bool sign = false;
    uint8_t i = 0;
    uint8_t j = 0;
    uint8_t p = 0;
    int32_t var =0;
    while(string[j]!='\0')
    {
        j++;
    }
    if(string[0]=='-')
    {
        sign = true;
        i++;
        j--;
    }

    p=j;
    p--;
    while(string[i]!='\0')
    {
        var += (string[i]-0x30)*ten[p];//power of ten
        
        p--;
        i++;
    }
    if(sign == true)
    {
        var = var * -1;
    }
    return var;
}
