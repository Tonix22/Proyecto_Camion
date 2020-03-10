#include <time.h>
#include <stdio.h>
#include "lwip/apps/sntp.h"
#include "freeRTOS_wrapper.h"
#include "espconn.h"
#include "esp_common.h"
#include "../interfaces/printer.h"
#include "../web_services/Tcp_mail.h"
#include "../database/data_base.h"
#include "user_config.h"
#include "uart.h"
#include "../include/user_config.h"

#define TIME_COMMAND_QUEUE_SIZE 1U
#define SNTP_DISCONNECT 0
#define TEN_SEC 40
extern TaskHandle_t xData_Base;
extern xQueueHandle xQueueUart;
ticket_time ntp_time;
QueueHandle_t time_state_queue = NULL;
extern SemaphoreHandle_t NTP_Request;
bool time_to_send = false;
struct tm * time_string;
static void Summer_winter_time(struct tm * time_temp);
uint8_t Mail_Hour;
uint8_t Mail_Min;

uint32_t sntp_time;
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
		sntp_time = sntp_get_current_timestamp();
		vTaskDelay(1000/portTICK_RATE_MS);
	}
	while(sntp_time == 0);
	/*change UTC summer winter*/

	time_set = sntp_time;
	time_string = gmtime(&time_set);
	sntp_stop();
	Summer_winter_time(time_string);

	free(addr);
	/* Create a queue capable of containing 1 struct*/
	time_state_queue = xQueueCreate(TIME_COMMAND_QUEUE_SIZE, sizeof(struct tm *));
	
	if (NULL != time_state_queue) 
	{
		DEBUG_NTP("ntp Queue created\r\n");
	} 
	else 
	{
		/* Return error */
		DEBUG_NTP("ntp Queue error created\r\n");
	}
	do
	{
		sntp_time = sntp_get_current_timestamp();
		vTaskDelay(1000/portTICK_RATE_MS);
	}
	while(sntp_time == SNTP_DISCONNECT);

	internal_timer = sntp_time;
	
	while(1)
	{
		 vTaskDelay(250/portTICK_RATE_MS);
		 review++;

		 if(review == 4)// 1 sec
		 {
			Command_check += review; 
			review = 0;
			sntp_time = sntp_get_current_timestamp();

			if(sntp_time == SNTP_DISCONNECT)
		 	{
				internal_timer++;
		 	}
		 	else
		 	{
			 	internal_timer = sntp_time;
		 	}
			time_set = internal_timer;

			if(Command_check == TEN_SEC) // each 10 seconds
			{	
				time_string = gmtime(&time_set);
				DEBUG_NTP("TIME %d:%d:%d\r\n",time_string->tm_hour,time_string->tm_min,time_string->tm_sec);
				Command_check = 0;
				
				//TIME SCHEDULER
				if(time_string->tm_hour == Mail_Hour && time_string->tm_min == Mail_Min && time_to_send == false )
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
						xQueueSend(time_state_queue, ( void * ) &time_string,( portTickType ) 0);
					}
				}
				xSemaphoreGive( NTP_Request );
		 }
	}
}
void set_mail_time(FlashData* Setup_time)
{
	char *mail_alarm = Setup_time->EMAIL_TIME;
	Mail_Hour = (mail_alarm[0]-0x30)*10 + (mail_alarm[1]-0x30);
	Mail_Min  = (mail_alarm[5]-0x30)*10 + (mail_alarm[6]-0x30);
	DEBUG_NTP("Mail Alarm: %d:%d\r\n",Mail_Hour,Mail_Min);
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
		DEBUG_NTP("winter time\r\n");
	}
	else
	{
		if(true == sntp_set_timezone(-5))
		{
			sntp_init();
		}
		DEBUG_NTP("summer time\r\n");
	}
}