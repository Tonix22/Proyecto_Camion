#include <time.h>
#include <stdio.h>
#include "lwip/apps/sntp.h"
#include "freeRTOS_wrapper.h"
#include "espconn.h"
#include "esp_common.h"
#include "printer.h"
#include "Tcp_mail.h"
#include "data_base.h"
#include "user_config.h"
#include "uart.h"

extern TaskHandle_t xData_Base;
extern xQueueHandle xQueueUart;
#define TIME_COMMAND_QUEUE_SIZE 1U
uint8_t mail_time[BUFFER_SIZE];
ticket_time ntp_time;
QueueHandle_t time_state_queue = NULL;
extern SemaphoreHandle_t NTP_Request;
bool time_to_send = false;
struct tm * time_string;
static void Summer_winter_time(struct tm * time_temp);
uint8_t hora;
uint8_t min;

uint32_t temporal;
uint32_t internal_timer;
time_t current_time;
time_t time_set;
uint8_t time_match;
uint8_t review;
uint8_t Command_check;
int compare;

void Time_check (void *pvParameters)
{
	ip_addr_t *addr = (ip_addr_t *)os_zalloc(sizeof(ip_addr_t));
	sntp_setservername(0, "us.pool.ntp.org"); // set server 0 by domain name
	sntp_setservername(1, "ntp.sjtu.edu.cn"); // set server 1 by domain name
	ipaddr_aton("210.72.145.44", addr);
	sntp_setserver(2, addr); // set server 2 by IP address
	sntp_init();
	/*wait valid data*/
	do
	{
		temporal = sntp_get_current_timestamp();
		vTaskDelay(1000/portTICK_RATE_MS);
	}
	while(temporal == 0);
	/*change UTC summer winter*/

	time_set = internal_timer;
	time_string = gmtime(&time_set);
	sntp_stop();
	vTaskDelay(500/portTICK_RATE_MS);
	Summer_winter_time(time_string);
	free(addr);
	/* Create a queue capable of containing 1 unsigned char */
	time_state_queue = xQueueCreate(TIME_COMMAND_QUEUE_SIZE, sizeof(struct tm *));
	
	if (NULL != time_state_queue) 
	{
		printf("ntp Queue created\r\n");
	} 
	else 
	{
		/* Return error */
		printf("ntp Queue error created\r\n");
	}
	do
	{
		temporal = sntp_get_current_timestamp();
		vTaskDelay(1000/portTICK_RATE_MS);
	}
	while(temporal == 0);
	internal_timer = temporal;

	
	vTaskDelay(100/portTICK_RATE_MS);
	while(1)
	{
		 vTaskDelay(250/portTICK_RATE_MS);
		 review++;
		 
		 if(review == 4)// 1 sec
		 {
			Command_check += review; 
			review = 0;
			temporal = sntp_get_current_timestamp();
			if(temporal == 0)
		 	{
				internal_timer++;
		 	}
		 	else
		 	{
			 	internal_timer = temporal;
		 	}
			time_set = internal_timer;
		 	time_string = gmtime(&time_set);
			
			if(Command_check == 40) // each 10 seconds
			{	
				Command_check = 0;
				if(time_string->tm_hour==hora && time_string->tm_min==min && time_to_send==false )
				{
					xTaskCreate(data_base_task,"data base",1024,NULL,2,xData_Base);
					//KILll task when end
						/*
						This task manage all data between GPIOS printer and 
						movments senors, to send it to TCP server.
						Also recieved data from back sensor that are send it
						by tcp client
						*/
					time_to_send = true;
					
				}
				if(xQueueReceive(xQueueUart, &(mail_time), ( TickType_t ) 100) )
				{
					printf("rcv: %s",mail_time);
					if(mail_time[0]=='m'&& mail_time[1]=='a'&&  mail_time[2]=='i' &&  mail_time[3]=='l' &&  mail_time[4]==':' )
					{
						hora = (mail_time[5]-0x30)*10 + (mail_time[6]-0x30);
						min  = (mail_time[8]-0x30)*10 + (mail_time[9]-0x30);
						printf("Time: %d:%d\r\n",hora,min);
					}
				}
			}
		 }
		 if(NTP_Request!=NULL)
		 {
			if(xSemaphoreTake(NTP_Request, ( TickType_t ) 100 ) == pdTRUE)
		 	{
				time_set        = internal_timer;
				time_string = gmtime(&time_set);
				if(time_state_queue!=NULL)
				{
					xQueueSend(time_state_queue, ( void * ) &time_string,( portTickType ) 10);
				}
			 }
		 }
	}
}
void set_mail_time(FlashData* Setup_time)
{
	char *mail_alarm = Setup_time->EMAIL_TIME;
	hora = (mail_alarm[0]-0x30)*10 + (mail_alarm[1]-0x30);
	min  = (mail_alarm[5]-0x30)*10 + (mail_alarm[6]-0x30);
	printf("Time: %d:%d\r\n",hora,min);
}
static void Summer_winter_time(struct tm * time_temp)
{
	int week_day = time_temp->tm_wday;
	int day      = time_temp->tm_mday;
	int month    = (time_temp->tm_mon)+1;
	bool summer   = false;
	//If it is sunday bigger than 25 and October
	if(month>=10 && month<4)
	{
		if(day<25 && month == 10)
		{	
			summer = true;
		}
		else if(week_day == 0 && day>=25 && month == 10)
		{
			summer = false;
		}
	}
	if((month>=4 && month<10))
	{
		if (day<7 && month == 4 && week_day != 0)
		{	
			summer = false;
		}
		else
		{
			summer = true;
		}
	}
	if(summer == false)
	{
		if(true == sntp_set_timezone(-6))
		{
			sntp_init();
		}
		printf("winter time\r\n");
	}
	else
	{
		if(true == sntp_set_timezone(-5))
		{
			sntp_init();
		}
		printf("summer time\r\n");
	}
}