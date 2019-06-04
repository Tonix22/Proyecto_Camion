#include "esp_common.h"
#include "user_config.h"
#include "../include/user_config.h"
#include "../web_services/ntp_time.h"
#include "../web_services/udp_client.h"
#include "../web_services/MQTTEcho.h"
#include "freeRTOS_wrapper.h"
#include "../application_drivers/Flash_driver.h"
#include "../custom_logic/common_logic.h"

#define SOFT_AP_SSID      "central_comunication"
#define SOFT_AP_PASSWORD  "12345678"
#define DEVICES_CAPACITY 4

os_timer_t acces_point_config;
os_timer_t MQTT_timer;
bool network_sucess = false;
SemaphoreHandle_t ip_connect = NULL;
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
            DEBUG_NETWORK("connect to ssid %s, channel %d\n", evt->event_info.connected.ssid,
                    evt->event_info.connected.channel);
            break;
        case EVENT_STAMODE_DISCONNECTED:
            DEBUG_NETWORK("disconnect from ssid %s, reason %d\n", evt->event_info.disconnected.ssid,
                    evt->event_info.disconnected.reason);
            break;
        case EVENT_STAMODE_AUTHMODE_CHANGE:
            DEBUG_NETWORK("mode: %d -> %d\n", evt->event_info.auth_change.old_mode, evt->event_info.auth_change.new_mode);
            break;
        case EVENT_STAMODE_GOT_IP:
            DEBUG_NETWORK("ip:" IPSTR ",mask:" IPSTR ",gw:" IPSTR, IP2STR(&evt->event_info.got_ip.ip),
                    IP2STR(&evt->event_info.got_ip.mask), IP2STR(&evt->event_info.got_ip.gw));
            DEBUG_NETWORK("\n");
            network_sucess = true;
            xSemaphoreGive(ip_connect);
            os_timer_arm(&acces_point_config,10,0);
            
            break;
        case EVENT_SOFTAPMODE_STACONNECTED:
            udpServer();//1024
            DEBUG_NETWORK("station: " MACSTR "join, AID = %d\n", MAC2STR(evt->event_info.sta_connected.mac),
                    evt->event_info.sta_connected.aid);
            break;
        default:
            break;
    }
}
/******************************************************************************
 * FunctionName : wifi_init
 * Description  : Set the module as acces point, setting ip addresses to lease
 *                and he dhcp services, also here is selected the wifi name and password
 *                for acces point
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
void conn_AP_Init(uint8_t flash_state)
{   

    /*Wifi configuration mode as station and acces point*/
    wifi_set_opmode(STATIONAP_MODE);

    /*Handler to jump when connection is ready*/
    wifi_set_event_handler_cb(network_init);

	struct softap_config *config = (struct softap_config *) zalloc(sizeof(struct softap_config)); 
    
    /* Get soft-AP config context first*/
    wifi_softap_get_config(config); 

    /*Name and pass of the acces point, which will be visible for device that want conection*/
    if(flash_state==0xFF)
    {
        sprintf(config->ssid, "SYSTEM SET UP");
        sprintf(config->password, SOFT_AP_PASSWORD);
    }
    else
    {
        sprintf(config->ssid, SOFT_AP_SSID);
        sprintf(config->password, SOFT_AP_PASSWORD);
    }

    /*acces point settings*/
    config->authmode = AUTH_WPA_WPA2_PSK;
    config->ssid_len = 0; // or its actual SSID length
    config->max_connection = DEVICES_CAPACITY;// 4 devices as max

    /* Set ESP8266 soft-AP config)*/

    if(wifi_softap_set_config(config)==true) 
    {
        DEBUG_NETWORK("acces point configuration succeed \r\n");
    }
    else
    {
         DEBUG_NETWORK("acces point configuration fail \r\n");
    }
    free(config);
    /*Get the information of stations connected to the ESP8266 soft-AP,
    including MAC and IP.*/


    /****************************************************
     ***********DHCP and IP CONFIG***********************
     ****************************************************/
    wifi_softap_free_station_info(); // Free it by calling functionss
    wifi_softap_dhcps_stop(); // disable soft-AP DHCP server
    struct ip_info info;
    IP4_ADDR(&info.ip, 192, 168, 5, 2); // set IP for the acces point
    IP4_ADDR(&info.gw, 192, 168, 5, 1); // set gateway
    IP4_ADDR(&info.netmask, 255, 255, 255, 0); // set netmask
    wifi_set_ip_info(SOFTAP_IF, &info);
    struct dhcps_lease lease;
    IP4_ADDR(&lease.start_ip, 192, 168, 5, 3);
    IP4_ADDR(&lease.end_ip, 192, 168, 5, 6);
    wifi_softap_set_dhcps_lease(&lease);

    if(wifi_softap_dhcps_start()==true)
    {
        DEBUG_NETWORK("DCHP succeed\r\n");
    }
    else
    {
        DEBUG_NETWORK("DCHP fail\r\n");
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
void wifi_init(FlashData* Conection_data)
{
    char* ssid_parse = Conection_data->SSID_DATA;
    printf("assign var\r\n");

    SSID_space_parse(ssid_parse);

    printf("ssid parse\r\n");
    /*Wifi configuration mode as station and acces point*/
    wifi_set_opmode(STATIONAP_MODE);

    /*station configuration*/
    struct station_config config;
    bzero(&config, sizeof(struct station_config));  //set value of config from address of &config to width of size to be value '0'
    sprintf(config.ssid, ssid_parse); // name of the acces point
    sprintf(config.password, Conection_data->PASS_DATA);
    wifi_station_set_config(&config);
    /*Handler to jump when connection is ready*/
    wifi_set_event_handler_cb(network_init);

    /*Timer set to callback a configuration fucntion*/
    os_timer_setfn(&acces_point_config,(os_timer_func_t *)conn_AP_Init, NULL);

    /*when connection is ready it will jump to the network_init callback*/
    wifi_station_connect();

}