#include "esp_common.h"
#include "freeRTOS_wrapper.h"
#include "espconn.h"
#include "Tcp_mail.h"
#include "data_base.h"

/**
DEFINES
 */
#define EHLO_LEN 6
#define LOG_LEN 12
#define USER_LEN 38
#define KEY_LEN 22
#define MAIL_FROM_LEN 35
#define DATA_LEN 6
#define MESSAGE_LEN 25
#define DOT_LEN 3
#define QUIT_LEN 6

#define M_SUBIDA_LEN 13
#define M_BAJADAS_LEN 13
#define M_OBSTRUC_LEN 24
#define M_NORMAL_LEN 12
#define M_TRANSVALE_LEN 15
#define M_MITAD_LEN 11

#define NET_DOMAIN "mail.smtp2go.com"
#define PORT 2525

uint8_t static HELLO[]={"EHLO\r\n"};
uint8_t static AUTH_LOGIN[]={"AUTH LOGIN\r\n"};
uint8_t static USER[]={"ZW1pbGlvdG9uaXhAYnVzZXJ2aWNlLmNvbQ==\r\n"};
uint8_t static KEY[]={"S2VubmVkeTE5NjMuLi8=\r\n"};
uint8_t static FROM[]={"MAIL FROM:<emiliotonix@gmail.com>\r\n"};
uint8_t static TO[]={"RCPT To:<emiliotonix@gmail.com>\r\n"};
uint8_t static DATA[]={"DATA\r\n"};
//uint8_t static MESSAGE[]={"Subject: REPORTE RUTA 27\n"};
uint8_t static DOT[]={".\r\n"};
uint8_t static QUIT[]={"QUIT\r\n"};

extern uint8_t ALL[ALL_size];
static uint8_t data_counter=0;
bool First_flag=true;
bool DATA_END=false;

ip_addr_t tcp_server_ip;
static struct espconn *INFO;
static struct _esp_tcp user_tcp;
static struct espconn user_tcp_conn;



void user_dns_found(const char *name, ip_addr_t *ipaddr, void *arg) //GET THE DNS AND LATER HAS TWO POSIIBLE CALLBACKS
{
	struct espconn *pespconn = (struct espconn *)arg;

	if (ipaddr == NULL)
	{
		printf("user_dns_found NULL \r\n");
		return;
	}

	//dns got ip
	printf("user_dns_found %d.%d.%d.%d \r\n",
			*((uint8 *)&ipaddr->addr), *((uint8 *)&ipaddr->addr + 1),
			*((uint8 *)&ipaddr->addr + 2), *((uint8 *)&ipaddr->addr + 3));

	if (tcp_server_ip.addr == 0 && ipaddr->addr != 0)
	{
		// dns succeed, create tcp connection
		tcp_server_ip.addr = ipaddr->addr;
		memcpy(pespconn->proto.tcp->remote_ip, &ipaddr->addr, 4); // remote ip of tcp server which get by dns

		pespconn->proto.tcp->remote_port = PORT; // remote port of tcp server

		pespconn->proto.tcp->local_port = espconn_port(); //local port of ESP8266

		espconn_regist_connectcb(pespconn, user_tcp_connect_cb); // register connect callback
		espconn_regist_reconcb(pespconn, user_tcp_recon_cb); // register reconnect callback as error handler

		espconn_connect(pespconn); // tcp connect
	}
	
}
static void MAIL_SEND(struct espconn *pespconn)
{
	if(data_counter<9 && DATA_END==false)
	{
		printf("CASE:%d\r\n",data_counter);
		switch(data_counter)
		{
		/*INIT HADSHAKE*/
		case 0:
			user_send_data(pespconn,AUTH_LOGIN,LOG_LEN);
			break;
			/*Log IN*/
		case 1:
			user_send_data(pespconn,USER,USER_LEN);
			break;
		case 2:
			user_send_data(pespconn,KEY,KEY_LEN);
			break;
		case 3:
			user_send_data(pespconn,FROM,MAIL_FROM_LEN);
			break;
		case 4:
			user_send_data(pespconn,TO,33);
			break;
			/*DATA PREPARE*/
		case 5:
			user_send_data(pespconn,DATA,DATA_LEN);
			break;
			/*MESSAGE SENDING*/
		case 6:
			user_send_data(pespconn,ALL,ALL_size);
			break;
		case 7:
			user_send_data(pespconn,DOT,DOT_LEN);
			break;
		case 8:
			user_send_data(pespconn,QUIT,QUIT_LEN);
			DATA_END=true;
			break;
		}
		data_counter++;
	}
	else
	{
		data_counter=0;
	}

}
static void user_tcp_connect_cb(void *arg)
{
	struct espconn *pesp = arg;
	INFO = arg;
	printf("Connected to server...\r\n");
	//HANDLERS IN CASE OF SITUATION
	espconn_regist_recvcb(pesp, user_tcp_recv_cb);
	espconn_regist_sentcb(pesp, user_tcp_sent_cb);
	espconn_regist_disconcb(pesp, user_tcp_discon_cb);
	user_send_data(pesp,HELLO,EHLO_LEN);
}
void user_send_data(struct espconn *pespconn, uint8_t *data, uint8_t pack_zise)
{
	char *pbuf = (char *)zalloc(pack_zise);

	sprintf(pbuf, data, NET_DOMAIN);
	espconn_send(pespconn, pbuf, strlen(pbuf));
	os_free(pbuf);

}

static void user_tcp_recon_cb(void *arg, sint8 err)
{
	//error occured , tcp connection broke. user can try to reconnect here.

	printf("Reconnect callback called, error code: %d !!! \r\n",err);
}
static void user_tcp_recv_cb(void *arg, char *pusrdata, unsigned short length)
{
	//received some data from tcp connection

	printf("Received data string: %s \r\n", pusrdata);
	if(First_flag==false)
	{
		vTaskDelay(1000/portTICK_RATE_MS);
		MAIL_SEND(INFO);
	}
	else
	{
		First_flag=false;
	}
	

}
static void user_tcp_sent_cb(void *arg)
{
	//data sent successfully
	printf("Sent callback: data sent successfully.\r\n");
	if(data_counter>6)
	{
		vTaskDelay(800/portTICK_RATE_MS);
		MAIL_SEND(INFO);
	}
}
static void user_tcp_discon_cb(void *arg)
{
	//tcp disconnect successfully
	printf("Disconnected from server.\r\n");
}
void sending_mail(void)
{
	espconn_init();
	struct ip_info ipconfig;
	wifi_get_ip_info(STATION_IF, &ipconfig);
	DATA_END = false;
	if (wifi_station_get_connect_status() == STATION_GOT_IP && ipconfig.ip.addr != 0)
	{
		// Connect to tcp server as NET_DOMAIN
		user_tcp_conn.proto.tcp = &user_tcp;
		user_tcp_conn.type = ESPCONN_TCP;
		user_tcp_conn.state = ESPCONN_NONE;
		//tcp_server_ip.addr = 0;
		const char esp_tcp_server_ip[4] = {173, 255, 233, 87}; // remote IP of TCP server
		memcpy(user_tcp_conn.proto.tcp->remote_ip, esp_tcp_server_ip, 4);

		user_tcp_conn.proto.tcp->remote_port = 2525;  // remote port

		user_tcp_conn.proto.tcp->local_port = espconn_port(); //local port of ESP8266

		espconn_regist_connectcb(&user_tcp_conn, user_tcp_connect_cb); // register connect callback
		espconn_regist_reconcb(&user_tcp_conn, user_tcp_recon_cb); // register reconnect callback as error handler
		espconn_connect(&user_tcp_conn);
		//espconn_gethostbyname(&user_tcp_conn, NET_DOMAIN, &tcp_server_ip, user_dns_found); // DNS function
	}

}

