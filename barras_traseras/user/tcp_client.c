#include "c_types.h"
#include "esp_common.h"
#include "tcp_client.h"
#include "barras.h"
#include "freeRTOS_wrapper.h"
#include "espconn.h"

static struct espconn *global_write;
static struct _esp_tcp user_tcp;
static struct espconn user_tcp_conn;
bool conection_enable = false;

QueueHandle_t TCP_queue;
SemaphoreHandle_t TCP_semaphore;

void tcp_client_task(void *pvParameters)
{
	//TODO
	//check espconn_disconnect and connect
	// check keep alive
	uint8_t action;
	user_esp_platform_check_ip();
	TCP_queue     = xQueueCreate(1, sizeof(uint8_t));
	TCP_semaphore = xSemaphoreCreateMutex();
	while(1)
	{
		if( xSemaphoreTake( TCP_queue, ( TickType_t ) 100 ) == pdTRUE)
		{
			if(xQueueReceive(TCP_queue, &(action), ( TickType_t ) 100 ) == pdPASS)
			{ 
				action += 0x30;//for converting to char meanable
				user_send_data(&action,1);
			}
		}
	}
}


static void user_send_data(uint8_t *data,uint8_t pack_zise)
{
	
	if(conection_enable == true)
	{
		espconn_send(global_write, data, 1);
	}
}


static void user_tcp_recv_cb(void *arg, char *pusrdata, unsigned short length)
{
	//received some data from tcp connection

	printf("Received data string: %s \r\n", pusrdata);

}
static void user_tcp_sent_cb(void *arg)
{
	//data sent successfully

	printf("Sent callback: data sent successfully.\r\n");
}

static void user_tcp_discon_cb(void *arg)
{
	//tcp disconnect successfully
	conection_enable = false;
	printf("Disconnected from server.\r\n");
}



static void user_tcp_connect_cb(void *arg)
{
	struct espconn *pesp = arg;
	global_write = pesp;
	conection_enable = true;
	printf("Connected to server...\r\n");

	espconn_regist_recvcb(pesp, user_tcp_recv_cb);
	espconn_regist_sentcb(pesp, user_tcp_sent_cb);
	espconn_regist_disconcb(pesp, user_tcp_discon_cb);

}
static void user_tcp_recon_cb(void *arg, sint8 err)
{
	//error occured , tcp connection broke. user can try to reconnect here.
	conection_enable = true;
	printf("Reconnect callback called, error code: %d !!! \r\n",err);
}

void user_esp_platform_check_ip(void)
{
	espconn_init();
	printf("Connected to router and assigned IP!\r\n");
	// Connect to tcp server as NET_DOMAIN
	user_tcp_conn.proto.tcp = &user_tcp;
	user_tcp_conn.type = ESPCONN_TCP;
	user_tcp_conn.state = ESPCONN_NONE;

	//const char esp_tcp_server_ip[4] = {192, 168, 5, 1}; // remote IP of TCP server
	const char esp_tcp_server_ip[4] = {192, 168, 0, 105}; // remote IP of TCP server
	memcpy(user_tcp_conn.proto.tcp->remote_ip, esp_tcp_server_ip, 4);

	user_tcp_conn.proto.tcp->remote_port = 1023;  // remote port

	user_tcp_conn.proto.tcp->local_port = espconn_port(); //local port of ESP8266

	espconn_regist_connectcb(&user_tcp_conn, user_tcp_connect_cb); // register connect callback
	espconn_regist_reconcb(&user_tcp_conn, user_tcp_recon_cb); // register reconnect callback as error handler
	espconn_connect(&user_tcp_conn);
	
}