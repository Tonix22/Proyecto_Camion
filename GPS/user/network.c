#include "esp_common.h"
#include "user_config.h"
#include "gps.h"
#include "MQTT_task.h"
#include "freeRTOS_wrapper.h"
#define NETWORK_DEBUG
#ifdef NETWORK_DEBUG
    #define NETWORK_DEBUG_PRINT printf
#else
    #define NETWORK_DEBUG_PRINT
#endif

#define MAX_TRIES 4

extern uint8 MAC_ADDRES[MAXROUTERS][20];
extern signed char Streght[MAXROUTERS];
extern MAC_SIZE;
extern void get_cordanates(void *pvParameters);
SemaphoreHandle_t Scan_semaphore = NULL;
bool connected = false;
bool once = false;
uint8_t fail_counter = 0;
/******************************************************************************
 * FunctionName : network_init
 * Description  : Call back for different wifi situations
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
void network_init(System_Event_t *evt)
{
    if (evt == NULL) {
        return;
    }
    switch (evt->event_id) 
    {
        case EVENT_STAMODE_CONNECTED:
            NETWORK_DEBUG_PRINT("connect to ssid %s, channel %d\n", evt->event_info.connected.ssid,
                    evt->event_info.connected.channel);
            break;
        case EVENT_STAMODE_DISCONNECTED:
            NETWORK_DEBUG_PRINT("disconnect from ssid %s, reason %d\n", evt->event_info.disconnected.ssid,
                    evt->event_info.disconnected.reason);
            break;
        case EVENT_STAMODE_AUTHMODE_CHANGE:
            NETWORK_DEBUG_PRINT("mode: %d -> %d\n", evt->event_info.auth_change.old_mode, evt->event_info.auth_change.new_mode);
            break;
        case EVENT_STAMODE_GOT_IP:
            NETWORK_DEBUG_PRINT("ip:" IPSTR ",mask:" IPSTR ",gw:" IPSTR, IP2STR(&evt->event_info.got_ip.ip),
                    IP2STR(&evt->event_info.got_ip.mask), IP2STR(&evt->event_info.got_ip.gw));
            NETWORK_DEBUG_PRINT("\n");
            if(once==false)
            {
                once = true;
                mqtt_init();
            }
            xTaskCreate( get_cordanates, (signed char *)"GPS", 4096, NULL, 3, NULL );
            break;
        case EVENT_SOFTAPMODE_STACONNECTED:
            NETWORK_DEBUG_PRINT("station: " MACSTR "join, AID = %d\n", MAC2STR(evt->event_info.sta_connected.mac),
                    evt->event_info.sta_connected.aid);
            break;
        default:
            break;
    }
}


/******************************************************************************
 * FunctionName : wifi_init
 * Description  : Initialize the wifi conection as station and access point.
 *                Client parameters are set to first start conection as station,
 *                which provides internet services. When connection is done it is set a connection handler
 *                "network_init" which will set the "acces_point_config" timer which will jump to conn_AP_Init.
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
void wifi_init(void)
{
    /*Wifi configuration mode as station and acces point*/
    wifi_set_opmode(STATION_MODE);

    /*station configuration*/
    struct station_config config;
    bzero(&config, sizeof(struct station_config));  //set value of config from address of &config to width of size to be value '0'
    sprintf(config.ssid, SSID); // name of the acces point
    sprintf(config.password, PASS);
    wifi_station_set_config(&config);
    /*Handler to jump when connection is ready*/
    wifi_set_event_handler_cb(network_init);
    /*when connection is ready it will jump to the network_init callback*/
    wifi_station_connect();
}

/******************************************************************************
 * FunctionName : scan_done
 * Description  : Does scanning and task creating of gps setpoing average
 * Parameters   : status
 * Returns      : none
*******************************************************************************/
void scan_done(void *arg, STATUS status)
{
    NETWORK_DEBUG_PRINT("now doing the scan_done... \n");
    uint8* ssid = malloc(33);
    uint8 i=0;
    MAC_SIZE = 0;
    if (status == OK) 
    {
        fail_counter = 0;
        struct bss_info *bss_link = (struct bss_info *) arg;

        while (bss_link != NULL) 
        {
            memset(ssid, 0, 33);
            if (strlen(bss_link->ssid) <= 32)
            {
                memcpy(ssid, bss_link->ssid, strlen(bss_link->ssid));
            }
            else
            {
                memcpy(ssid, bss_link->ssid, 32);
            }
                
            NETWORK_DEBUG_PRINT("(%d,\"%s\",%d,\""MACSTR"\",%d)\r\n\r\n", bss_link->authmode, ssid, bss_link->rssi,
            MAC2STR(bss_link->bssid), bss_link->channel);

            /* saving all MAC_ADDRESS and rssi */

            if(MAC_SIZE < MAXROUTERS)
            {
                sprintf(MAC_ADDRES[i],MACSTR,MAC2STR(bss_link->bssid));
                Streght[i] = bss_link->rssi;
                MAC_SIZE++;
            }
            bss_link = bss_link->next.stqe_next;
            i++;
            
        }
        free(ssid);

        if(connected == false)
        {
            //wifi_station_disconnect();
            wifi_init();
            connected = true;
        }
        else
        {
            //GPS TASK AVERAGE TASK
            xTaskCreate( get_cordanates, (signed char *)"GPS", 4096, NULL, 2, NULL );
        }
    } 
    else 
    {
        xSemaphoreGive(Scan_semaphore);
        fail_counter++;

        NETWORK_DEBUG_PRINT("scan fail !!!\r\n");
        NETWORK_DEBUG_PRINT("try: %d",fail_counter);

        vTaskDelay(1000/portTICK_RATE_MS);

        if(fail_counter == MAX_TRIES)
        {
            NETWORK_DEBUG_PRINT("system will restart\r\n");
            system_restart();
        }
    }
}
void Scan_Task (void *pvParameters)
{
    Scan_semaphore = xSemaphoreCreateMutex();
    wifi_set_opmode(STATION_MODE);
    wifi_station_disconnect();
    while(1)
    {
        if(xSemaphoreTake(Scan_semaphore, ( TickType_t ) 500/portTICK_RATE_MS ) == pdTRUE)
        {
            //scan_done is the callback to the scan task
            wifi_station_scan(NULL,scan_done);
        }
        vTaskDelay(500/portTICK_RATE_MS);
    }
    vTaskDelete(NULL);
}