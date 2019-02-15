
#include "esp_common.h"
#include "espconn.h"
#include "c_types.h"
#include "user_config.h"
#include "udp_client.h"
#include "barras.h"
const uint8 udp_server_ip[4] = { 10, 0, 0, 4 };
barras_t udp_barr_rcv;
os_timer_t time1;
struct espconn udp_client;

void UdpRecvCb(void *arg, char *pdata, unsigned short len)
{
    struct espconn* udp_server_local = arg;
    DBG_LINES("UDP_RECV_CB");
    printf("%s\n",pdata);
    DBG_LINES("END");
    printf("\n");
    if(pdata[0]=='U' && pdata[1]=='P')
    {
        udp_barr_rcv.subidas++;
    }    
    else if(pdata[0]=='D' && pdata[1]=='O' && pdata[2]=='W' && pdata[3]=='N')
    {
        udp_barr_rcv.bajadas++;
    }
    else if(pdata[0]=='O' && pdata[1]=='B' && pdata[2]=='S')
    {
         udp_barr_rcv.obs++;
    }

}
void UdpSendCb(void* arg)
{
    struct espconn* udp_server_local = arg;
    DBG_LINES("UDP_SEND_CB");
    DBG_PRINT("UDP_SEND_CB ip:%d.%d.%d.%d port:%d\n", udp_server_local->proto.tcp->remote_ip[0],
            udp_server_local->proto.tcp->remote_ip[1], udp_server_local->proto.tcp->remote_ip[2],
            udp_server_local->proto.tcp->remote_ip[3], udp_server_local->proto.tcp->remote_port\
);
}

void udpServer(void*arg)
{
    static struct espconn udp_client;
    static esp_udp udp;
    udp_client.type = ESPCONN_UDP;
    udp_client.proto.udp = &udp;
    udp.local_port = UDP_SERVER_LOCAL_PORT;

    espconn_regist_recvcb(&udp_client, UdpRecvCb);
    espconn_regist_sentcb(&udp_client, UdpSendCb);
    int8 res = 0;
    res = espconn_create(&udp_client);
    if (res != 0) {
        DBG_PRINT("UDP SERVER CREAT ERR ret:%d\n", res);
    }
    //vTaskDelete(NULL);
}

void t1Callback(void* arg)
{
    os_printf("t1 callback\n");
    espconn_send(&udp_client, UDP_Client_GREETING, strlen(UDP_Client_GREETING));

}

void udpClient(void*arg)
{

    static esp_udp udp;
    udp_client.type = ESPCONN_UDP;
    udp_client.proto.udp = &udp;
    udp.remote_port = UDP_SERVER_LOCAL_PORT;

    memcpy(udp.remote_ip, udp_server_ip, sizeof(udp_server_ip));
    uint8 i = 0;
    os_printf("serve ip:\n");
    for (i = 0; i <= 3; i++) {
        os_printf("%u.", udp_server_ip[i]);
    }
    os_printf("\n remote ip\n");
    for (i = 0; i <= 3; i++) {
        os_printf("%u.", udp.remote_ip[i]);
    }
    os_printf("\n");
    espconn_regist_recvcb(&udp_client, UdpRecvCb);
    espconn_regist_sentcb(&udp_client, UdpSendCb);
    int8 res = 0;
    res = espconn_create(&udp_client);

    if (res != 0) {
        DBG_PRINT("UDP CLIENT CREAT ERR ret:%d\n", res);
    }

    os_timer_disarm(&time1);
    os_timer_setfn(&time1, t1Callback, NULL);
    os_timer_arm(&time1, 5000, 1);

    //vTaskDelete(NULL);
}

