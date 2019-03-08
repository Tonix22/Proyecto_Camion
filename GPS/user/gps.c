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

extern QueueHandle_t MQTT_Queue;
extern SemaphoreHandle_t MQTT_semaphore ;

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

//for save taken values
int lat_data[MAX_VALID_DATA];
int lon_data[MAX_VALID_DATA];
int rssi_data[MAX_VALID_DATA];
uint8_t valid_data_counter = 0;
uint8_t filter_data_size;
uint8_t filter_index;


void get_cordanates(void *pvParameters)
{
    //HTTP VARS
    int recbytes;
    int sin_size;
    int sta_socket;
    struct sockaddr_in remote_ip;
    char *pbuf = (char *) zalloc(512);
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
                printf("socket fail !\r\n");
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
                printf("connect fail!\r\n");
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
                printf("send fail\n");
                //free(pbuf);
                read = false;
            }
        }
        
        if(read == true)
        {
            //HERE GET THE HTTP PACKETS
            recbytes = read(sta_socket, recv_buf, 1460);
            //Valid HTTP packet not fail
            http_token = strtok(recv_buf," ");
            http_token = strtok(NULL," ");
            http_valid_request = atoi(http_token);//check if it 200 meaning good request
            if(http_valid_request == 200)
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
                        printf("MAC: %s\r\n",MAC_add);
                        printf("rssi: %i\r\n",ssid);
                        printf("NON filtered lat: %d\r\n",lat_dec);
                        printf("NON filtered lon: %d\r\n",lon_dec);
                        
                        #ifdef DEBUG
                        printf("lat_int: %d\r\n",lat_int);
                        printf("lat_dec: %d\r\n",lat_dec);
                        printf("lon_int: %d\r\n",lon_int);
                        printf("lon_dec: %d\r\n",lon_dec);
                        printf("error: %d\r\n",error);
                        #endif 
                        ///* printf("extra info:%s\r\n",data);*/
                    }
                }
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
        vTaskDelay(300/portTICK_RATE_MS);
        i++;
    }
    //Free HTTP resources
    free(pbuf);
    close(sta_socket);

    // average for data and deltas
    scanning_avg.lat = non_filter_sum.lat / valid_data_counter;
    scanning_avg.lon = non_filter_sum.lon / valid_data_counter;
    
    //same for longitud
    delta_avg.lon = delta_sum.lon / valid_data_counter;
    delta_avg.lat = delta_sum.lat /valid_data_counter;
    //Filtering data which is out of the averange and deviation
    for(filter_index=0;filter_index<valid_data_counter;filter_index++)
    {
        avg_distance.lat = abs(lat_data[filter_index]-scanning_avg.lat)-THRESHOLD;
        avg_distance.lon = abs(lon_data[filter_index]-scanning_avg.lon)-THRESHOLD;

        if(avg_distance.lat < delta_avg.lat && avg_distance.lon < delta_avg.lon )
        {
            printf("filtered lat: %d\r\n",lat_data[filter_index]);
            printf("filtered lon: %d\r\n",lon_data[filter_index]);
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
    #ifdef Weight_average
        scanning_avg.lat = filter_sum.lat / sum_weight;
        scanning_avg.lon = filter_sum.lot / sum_weight;
    #else
        scanning_avg.lat = filter_sum.lat / filter_data_size;
        scanning_avg.lon = filter_sum.lon / filter_data_size;
    #endif

    memset(lat_data,0,MAX_VALID_DATA);
    memset(lon_data,0,MAX_VALID_DATA);
    
    printf("valid data number: %d\r\n",filter_data_size);
    printf("average lat: 20.%d\r\n",scanning_avg.lat);
    printf("average lon: -103.%d\r\n",scanning_avg.lon);

    MQTT_start();
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
    else
    {
        printf("MQTT init\r\n");
    }
    xTaskCreate ( mqtt_client_thread, MQTT_CLIENT_THREAD_NAME,
                MQTT_CLIENT_THREAD_STACK_WORDS,
                NULL,
                2,
                NULL);
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