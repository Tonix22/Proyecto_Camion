#include "esp_common.h"
#include "user_config.h"
#include "ntp_time.h"
#include "Tcp_mail.h"
#define SOFT_AP_SSID      "central_comunication"
#define SOFT_AP_PASSWORD  "12345678"
#define DEVICES_CAPACITY 4
static void conn_AP_Init(void);
os_timer_t acces_point_config;
void network_init(System_Event_t *evt)
{
    if (evt == NULL) {
        return;
    }
    switch (evt->event_id) {
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
           os_timer_arm(&acces_point_config,10,0);  
            break;
        case EVENT_SOFTAPMODE_STACONNECTED:
			TcpLocalServer();//PORT 1023
            printf("station: " MACSTR "join, AID = %d\n", MAC2STR(evt->event_info.sta_connected.mac),
                    evt->event_info.sta_connected.aid);
            break;
        default:
            break;
    }
}
static void conn_AP_Init(void)
{   
	struct softap_config *config = (struct softap_config *) zalloc(sizeof(struct softap_config)); // initialization
    wifi_softap_get_config(config); // Get soft-AP config first.
    sprintf(config->ssid, SOFT_AP_SSID);
    sprintf(config->password, SOFT_AP_PASSWORD);
    config->authmode = AUTH_WPA_WPA2_PSK;
    config->ssid_len = 0; // or its actual SSID length
    config->max_connection = DEVICES_CAPACITY;
    wifi_softap_set_config(config); // Set ESP8266 soft-AP config
	printf("acces point\r\n");
	struct station_info * station = wifi_softap_get_station_info();
    while (station) {
        printf("bssid : MACSTR, ip : IPSTR/n", MAC2STR(station->bssid), IP2STR(&station->ip));
        station = STAILQ_NEXT(station, next);
    }
    wifi_softap_free_station_info(); // Free it by calling functionss
    wifi_softap_dhcps_stop(); // disable soft-AP DHCP server
    struct ip_info info;
    IP4_ADDR(&info.ip, 192, 168, 5, 1); // set IP
    IP4_ADDR(&info.gw, 192, 168, 5, 1); // set gateway
    IP4_ADDR(&info.netmask, 255, 255, 255, 0); // set netmask
    wifi_set_ip_info(SOFTAP_IF, &info);
    struct dhcps_lease dhcp_lease;
    IP4_ADDR(&dhcp_lease.start_ip, 192, 168, 5, 100);
    IP4_ADDR(&dhcp_lease.end_ip, 192, 168, 5, 105);
    wifi_softap_set_dhcps_lease(&dhcp_lease);
    wifi_softap_dhcps_start(); // enable soft-AP DHCP server
    printf("AP_enabled\r\n");
    xTaskCreate(Time_check,"ntp server",512,NULL,3,NULL);

}
void wifi_init(void)
{
    os_timer_setfn(&acces_point_config,(os_timer_func_t *)conn_AP_Init, NULL);
    wifi_set_opmode(STATIONAP_MODE);
    struct station_config config;
    bzero(&config, sizeof(struct station_config));  //set value of config from address of &config to width of size to be value '0'
    sprintf(config.ssid, SSID);
    sprintf(config.password, PASS);
    wifi_station_set_config(&config);
    wifi_set_event_handler_cb(network_init);
    wifi_station_connect();
}