#include "esp_common.h"
#include "user_config.h"
#define GET_WIFI
static void conn_AP_Init(void);

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
 * Description  : Set the module as acces point, setting ip addresses to lease
 *                and he dhcp services, also here is selected the wifi name and password
 *                for acces point
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
static void conn_AP_Init(void)
{   
	struct softap_config *config = (struct softap_config *) zalloc(sizeof(struct softap_config)); 

    /* Get soft-AP config context first*/
    wifi_softap_get_config(config); 

    /*Name and pass of the acces point, which will be visible for device that want conection*/
    sprintf(config->ssid, SOFT_AP_SSID);
    sprintf(config->password, SOFT_AP_PASSWORD);

    /*acces point settings*/
    config->authmode = AUTH_WPA_WPA2_PSK;
    config->ssid_len = 0; // or its actual SSID length
    config->max_connection = DEVICES_CAPACITY;// 4 devices as max

    /* Set ESP8266 soft-AP config)*/

    if(wifi_softap_set_config(config)==true) 
    {
        printf("acces point configuration succeed \r\n");
    }
    else
    {
         printf("acces point configuration fail \r\n");
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
        printf("DCHP succeed\r\n");
    }
    else
    {
        printf("DCHP fail\r\n");
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
    wifi_set_opmode(STATIONAP_MODE);
    /*Handler to jump when connection is ready*/
    wifi_set_event_handler_cb(network_init);
    #ifdef STATION
    /*Timer set to callback a configuration fucntion*/
    conn_AP_Init();
    #endif
    #ifdef GET_WIFI
    /*station configuration*/
    struct station_config config;
    bzero(&config, sizeof(struct station_config));  //set value of config from address of &config to width of size to be value '0'
    sprintf(config.ssid, SSID); // name of the acces point
    sprintf(config.password, PASS);
    wifi_station_set_config(&config);
    /*when connection is ready it will jump to the network_init callback*/
    wifi_station_connect();
    #endif
}