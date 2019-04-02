#include "esp_common.h"
#include "freeRTOS_wrapper.h"
#include "uart.h"
#include "lwip/mem.h"
#include "lwip/sockets.h"
#include "lwip/inet.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "gps.h"
#include <string.h>
#include <stdlib.h>
#include "MQTT_task.h"

//#define GPS_DEBUG
//#define ERROR_DEBUG
//#define HTTP_DEBUG
#ifdef GPS_DEBUG
    #define GPS_DEBUG_PRINT printf
#else
    #define GPS_DEBUG_PRINT
#endif

#ifdef ERROR_DEBUG
    #define GPS_ERR_PRINT printf
#else
    #define GPS_ERR_PRINT
#endif

extern QueueHandle_t MQTT_Queue;
extern SemaphoreHandle_t MQTT_semaphore ;

//Router INFO
signed char Streght[MAXROUTERS]={0};
uint8 MAC_ADDRES[MAXROUTERS][20];
uint8 MAC_SIZE = 0;

//HTTP Data
uint8 webname[] = {"api.mylnikov.org"};
char recv_buf[3000];
//char recv_buf[1460];


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
//cordenates filter
int lat_filter = 20;
int lon_filer = -103;

//average
cordenate scanning_avg;

cordenate delta_avg;

cordenate filter_sum;

cordenate non_filter_sum;

cordenate delta_sum;

cordenate previous;

cordenate avg_distance;

uint8_t weight;
int sum_weight;

//this one it is used for filter unvalid data
extern SemaphoreHandle_t Scan_semaphore;
//for save taken values
int lat_data[MAX_VALID_DATA];
int lon_data[MAX_VALID_DATA];
int rssi_data[MAX_VALID_DATA];
uint8_t valid_data_counter = 0;
uint8_t filter_data_size;
uint8_t filter_index;
//HTTPVARS
char pbuf [512];
//time filter
cordenate history[HISTORY_SIZE]={0};
cordenate history_avr;
cordenate history_sum;

uint8_t history_index=0;


void get_cordanates(void *pvParameters)
{
    //HTTP VARS
    int recbytes;
    int sin_size;
    int sta_socket;
    struct sockaddr_in remote_ip;
    memset(pbuf,'\0',512);
    int http_valid_request;
    char *http_token;

    //Control variables
    uint8_t i = 0;
    bool read = true;

    //DEBUG variables
    uint8 *MAC_add ;
    signed char ssid;
    valid_data_counter = 0;
    filter_data_size = 0;
    i = 0;

    /*check all the mac addreses gotten*/
    while (i<MAC_SIZE) 
	{
        //CHECK SOCKET STATUS
        MAC_add = MAC_ADDRES[i];
        ssid = Streght[i];
        if(i == 0 || read == false)
        {
            sta_socket = socket(PF_INET, SOCK_STREAM, 0);
            if (-1 == sta_socket) 
            {
                close(sta_socket);
                GPS_ERR_PRINT("socket fail !\r\n");
                read = false;
            }
            else
            {
                read = true;
            }
            bzero(&remote_ip, sizeof(struct sockaddr_in));
            remote_ip.sin_family = AF_INET;
            remote_ip.sin_addr.s_addr = inet_addr(DEMO_SERVER);
            remote_ip.sin_port = htons(DEMO_SERVER_PORT);
            
            //CONNECT SOCKET
            if(0 != connect(sta_socket,(struct sockaddr *)(&remote_ip),sizeof(struct sockaddr)))
            {
                close(sta_socket);
                GPS_ERR_PRINT("connect fail!\r\n");
                read = false;
            }
            else
            {
                read = true;
            }
        }
        
        if(read == true)
        {
            //HTTP REQUEST
            sprintf(pbuf, "GET /geolocation/wifi?v=1.2&bssid=%s HTTP/1.1\r\nHost: %s\r\n"pheadbuffer"", MAC_add, webname);
            if (write(sta_socket,pbuf,strlen(pbuf)+1) < 0) 
            {
                close(sta_socket);
                GPS_ERR_PRINT("send fail\n");
                //free(pbuf);
                read = false;
            }
        }
        
        if(read == true)
        {
            //HERE GET THE HTTP PACKETS
            recbytes = read(sta_socket, recv_buf, 1460);
            #ifdef HTTP_DEBUG
                printf("**************************\r\n");
                printf("%s\r\n",recv_buf);
                printf("**************************\r\n");
                vTaskDelay(5000/portTICK_RATE_MS);
            #endif
            vTaskDelay(800/portTICK_RATE_MS);//this is hardcoded due to system works
            http_token = strtok(recv_buf," ");
            http_token = strtok(NULL," ");
            if(http_token[0] == '2')
            {
                http_parse(recv_buf);
                exec = Data_Result(JSON_DATA);
                if(exec)
                {
                    lat_int = integer_part();
                    lat_dec = decimal_part();
                    error   = integer_part();
                    decimal_part();
                    lon_int = integer_part();
                    lon_dec = decimal_part();
                    //data    = location(); TODO CHECK A DINAMIC ALLOCATION FOR THIS MESSAGE BECAUSE IS BIG
                    
                    //other lat and lon values will be discarted, not sense to average them.
                    if(lat_filter == lat_int && lon_filer == lon_int && valid_data_counter< MAX_VALID_DATA)
                    {
                        non_filter_sum.lat+=lat_dec;
                        non_filter_sum.lon+=lon_dec;
                        if(valid_data_counter > 0)
                        {
                            delta_sum.lat +=abs(previous.lat-lat_dec);
                            delta_sum.lon +=abs(previous.lon-lon_dec);
                        }
                        previous.lat = lat_dec;
                        previous.lon = lon_dec;
                        
                        lat_data[valid_data_counter]=lat_dec;
                        
                        lon_data[valid_data_counter]=lon_dec;
                        
                        rssi_data[valid_data_counter]= (100+ssid);

                        valid_data_counter++;
                        
                        //debug vars
                        GPS_DEBUG_PRINT("MAC: %s\r\n",MAC_add);
                        GPS_DEBUG_PRINT("rssi: %i\r\n",ssid);
                        GPS_DEBUG_PRINT("NON filtered lat: %d\r\n",lat_dec);
                        GPS_DEBUG_PRINT("NON filtered lon: %d\r\n",lon_dec);
                        
                        #ifdef DEBUG
                        GPS_DEBUG_PRINT("lat_int: %d\r\n",lat_int);
                        GPS_DEBUG_PRINT("lat_dec: %d\r\n",lat_dec);
                        GPS_DEBUG_PRINT("lon_int: %d\r\n",lon_int);
                        GPS_DEBUG_PRINT("lon_dec: %d\r\n",lon_dec);
                        GPS_DEBUG_PRINT("error: %d\r\n",error);
                        #endif 
                        ///* GPS_DEBUG_PRINT("extra info:%s\r\n",data);*/
                    }
                }
            }
            else
            {
                GPS_ERR_PRINT("ERROR 404: Object was not found\r\n");
                read = false;
                close(sta_socket);
            }
        }
        else
        {
            GPS_ERR_PRINT("http fail!\r\n");
            close(sta_socket);
        }
        if (recbytes < 0) 
		{
            GPS_ERR_PRINT("bare data, buffer empty!\r\n");
            close(sta_socket);
        }
        i++;
    }
     GPS_DEBUG_PRINT("http end\r\n");
    //Free HTTP resources
    close(sta_socket);
    if(valid_data_counter != 0)
    {
        // average for data and deltas
        scanning_avg.lat = non_filter_sum.lat / valid_data_counter;
        scanning_avg.lon = non_filter_sum.lon / valid_data_counter;
        
        //same for longitud
        delta_avg.lat = delta_sum.lat /valid_data_counter;
        delta_avg.lon = delta_sum.lon / valid_data_counter;
        
        delta_avg.lat += delta_avg.lat /2;
        delta_avg.lon += delta_avg.lon /2;
        
        //Filtering data which is out of the averange and deviation
        GPS_DEBUG_PRINT("filter begins\r\n");
        GPS_DEBUG_PRINT("valid data counter: %d\r\n",valid_data_counter);
        GPS_DEBUG_PRINT("delta_lat: %d\r\n",delta_avg.lat);
        GPS_DEBUG_PRINT("delta_lon: %d\r\n",delta_avg.lon);
        GPS_DEBUG_PRINT("\r\n");
        for(filter_index=0;filter_index<valid_data_counter;filter_index++)
        {
            GPS_DEBUG_PRINT("\r\n");
            GPS_DEBUG_PRINT("filter_index: %d\r\n",filter_index);
            avg_distance.lat = abs(lat_data[filter_index]-scanning_avg.lat);
            avg_distance.lon = abs(lon_data[filter_index]-scanning_avg.lon);
            GPS_DEBUG_PRINT("lat_data:%d\r\n",lat_data[filter_index]);
            GPS_DEBUG_PRINT("lon_data:%d\r\n",lon_data[filter_index]);
            GPS_DEBUG_PRINT("avg_distance.lat: %d\r\n", avg_distance.lat);
            GPS_DEBUG_PRINT("avg_distance.lon: %d\r\n", avg_distance.lon);
            

            if(avg_distance.lat < delta_avg.lat && avg_distance.lon < delta_avg.lon )
            {
                GPS_DEBUG_PRINT("filtered lat: %d\r\n",lat_data[filter_index]);
                GPS_DEBUG_PRINT("filtered lon: %d\r\n",lon_data[filter_index]);
                #ifdef Weight_average
                filter_sum.lat += (lat_data[filter_index]*rssi_data[filter_index]);
                filter_sum.lon += (lon_data[filter_index]*rssi_data[filter_index]);
                sum_weight += rssi_data[filter_index];
                #else
                filter_sum.lat+= lat_data[filter_index];
                filter_sum.lon+= lon_data[filter_index];
                filter_data_size++;
                #endif
            }
        }
        //decimal part is avereged only
        //average weighted sum
        if(filter_data_size != 0)
        {
            #ifdef Weight_average
            scanning_avg.lat = filter_sum.lat / sum_weight;
            scanning_avg.lon = filter_sum.lot / sum_weight;
            #else
            scanning_avg.lat = filter_sum.lat / filter_data_size;
            scanning_avg.lon = filter_sum.lon / filter_data_size;
            #endif
        }
        else
        {
            GPS_DEBUG_PRINT("too many variance\r\n");
        }

        delta_sum.lat=0;
        delta_sum.lon=0;
        non_filter_sum.lat=0;
        non_filter_sum.lon=0;
        filter_sum.lat=0;
        filter_sum.lon=0;
        memset(lat_data,'\0',MAX_VALID_DATA);
        memset(lon_data,'\0',MAX_VALID_DATA);
        //20 and -103 are hardcoded
        GPS_DEBUG_PRINT("average lat: 20.%d\r\n",scanning_avg.lat);
        GPS_DEBUG_PRINT("average lon: -103.%d\r\n",scanning_avg.lon);
        GPS_DEBUG_PRINT("valid data number: %d\r\n",filter_data_size);
        // For future use
        //HISTORY_AVR();
        MQTT_start();
        vTaskDelay(1000/portTICK_RATE_MS);
    }
    else
    {
        vTaskDelay(1000/portTICK_RATE_MS);
        xSemaphoreGive(Scan_semaphore);
        GPS_DEBUG_PRINT("valid data number: %d\r\n",filter_data_size);
    }
    
    vTaskDelete(NULL);
}

void MQTT_start(void)
{
    if(MQTT_Queue != NULL && MQTT_semaphore!=NULL)
    {
        xSemaphoreGive( MQTT_semaphore );
        xQueueSend( MQTT_Queue,( void * ) &scanning_avg.lat,( TickType_t ) 100 );
        xQueueSend( MQTT_Queue,( void * ) &scanning_avg.lon,( TickType_t ) 100 );
    }
    xTaskCreate ( mqtt_client_thread, MQTT_CLIENT_THREAD_NAME,
    MQTT_CLIENT_THREAD_STACK_WORDS,
    NULL,
    3,
    NULL);
}
void HISTORY_AVR(void)
{
    uint8_t i=0;
    history[history_index] = scanning_avg;
    
    if(history_index<HISTORY_SIZE)
    {
        for(i=0;i<history_index;i++)
        {
            history_sum.lat += history[i].lat;
            history_sum.lon += history[i].lon;
        }
        history_avr.lat=history_sum.lat/HISTORY_SIZE;
        history_avr.lon=history_sum.lon/HISTORY_SIZE;
    }
    else
    {
        history_index=0;
    }
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