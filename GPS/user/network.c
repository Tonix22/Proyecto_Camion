#include "esp_common.h"
#include "user_config.h"
#include "gps.h"
#include "MQTT_task.h"
extern void get_cordanates(void *pvParameters);
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
            printf("connect to ssid %s, channel %d\n", evt->event_info.connected.ssid,
                    evt->event_info.connected.channel);
            break;
        case EVENT_STAMODE_DISCONNECTED:
            printf("disconnect from ssid %s, reason %d\n", evt->event_info.disconnected.ssid,
                    evt->event_info.disconnected.reason);
            break;
        case EVENT_STAMODE_AUTHMODE_CHANGE:
            printf("mode: %d -> %d\n", evt->event_info.auth_change.old_mode, evt->event_info.auth_change.new_mode);
            break;
        case EVENT_STAMODE_GOT_IP:
            printf("ip:" IPSTR ",mask:" IPSTR ",gw:" IPSTR, IP2STR(&evt->event_info.got_ip.ip),
                    IP2STR(&evt->event_info.got_ip.mask), IP2STR(&evt->event_info.got_ip.gw));
            printf("\n");
            xTaskCreate( get_cordanates, (signed char *)"Der Task", 4096, NULL, 3, NULL );
            break;
        case EVENT_SOFTAPMODE_STACONNECTED:
            printf("station: " MACSTR "join, AID = %d\n", MAC2STR(evt->event_info.sta_connected.mac),
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