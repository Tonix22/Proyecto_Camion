/*
 * printer.c
 *
 *  Created on: 03/07/2018
 *      Author: EmilioTonix
 */
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "gpio_config.h"
#include "uart.h"
#include "esp_common.h"
#include "freeRTOS_wrapper.h"
#include "printer.h"
#include "ntp_time.h"
#include "data_base.h"

#define BOLD_ON 1
#define BOLD_OFF 0
#define CENTRAR_ON 1
#define CENTRAR_OFF 0
#define NORMAL 1
#define MITAD 2
#define TRANSVALE 3
#define STATE_MACHINE_TICKS_MS       (5000 / spb_portTICK_PERIOD_MS)

uint8_t Printer_Start[27]={0x1B,0x38,0x00,0x00,0x1B,0x40,0x1B,0x44,0x04,
						   0x08,0x0C,0x10,0x14,0x18,0x1C,0x00,0x1B,0x37,
						   0x0B,0x78,0x28,0x12,0x23,0x4A,'\r','\n','\0'};

uint8_t Feed[3]       = {0x1B,0x64,0x02};
uint8_t Centro[3]     = {0x1B,0x61,0x01};
uint8_t Negritas_ON[3]= {0x1B,0x21,0x08};
			/*NEGRITAS Y CENTRO*/
uint8_t TITULO[]      = {"RRUTA 27\r\n\0"}; 
				/*IZQUIERDA*/

ticket_base_t ticket_info;
ticket_time printer_time;

uint8_t UNIDAD[]      = {"UUnidad: 2069\r\n\0"};
SemaphoreHandle_t NTP_Request;
extern QueueHandle_t time_state_queue;
extern QueueHandle_t gpio_state_queue;
extern SemaphoreHandle_t gpio_printer_semaphore;

void printer_init(void)
{
	NTP_Request = xSemaphoreCreateMutex();
	xSemaphoreTake(NTP_Request,( TickType_t ) 0);
	memcpy(printer_time.hora, "12:00:00", sizeof(printer_time.hora));
	memcpy(printer_time.fecha, "dd/mm/aa", sizeof(printer_time.fecha));
	UART_SetPrintPort(UART1);
	vTaskDelay(10/portTICK_RATE_MS);
	printf("%s",Printer_Start);
	UART_SetPrintPort(UART0);
	vTaskDelay(100/portTICK_RATE_MS);
	xTaskCreate(printer_task, (signed char *)"Printer Task", 512, NULL, 4, NULL );
    printf("printer task created\r\n");
}
static void Centrar(uint8_t ok)
{
	printf("%c",0x1B);
	printf("%c",0x61);
	printf("%c",ok);
	printf("\r");
}
static void Negritas(uint8_t enable)
{
	printf("%c",0x1B);
	printf("%c",0x21);
    if(enable == 1)
    {
    	printf("%c",0x08);
    }
    else
    {
    	printf("%c",0x00);
    }
	printf("\r");
}
void FEED(uint8_t feeds)
{
    Feed[2]=feeds;
    printf("%s",Feed);
}
void printer_print_TITLE(void *arg)
{
	printf("%s",TITULO);/*9 bytes*/
}
void printer_print_leftover(gpio_action_t ticket_recieved )
{
	uint8_t hora_ticket[]     = {"Hora: "};
	uint8_t fecha_ticket[]    = {"Fecha: "};
	struct tm *  ntp_time_rcv;
	if(time_state_queue != NULL )
	{
		if(xQueueReceive(time_state_queue, &(ntp_time_rcv), ( TickType_t ) 100) )
		{
			sprintf(printer_time.hora,"%d:%d:%d\0",ntp_time_rcv->tm_hour,ntp_time_rcv->tm_min,ntp_time_rcv->tm_sec);
			sprintf(printer_time.fecha,"%d/%d/%d\0",ntp_time_rcv->tm_mday,(ntp_time_rcv->tm_mon)+1,(ntp_time_rcv->tm_year)%100);
		}
	}
		
	 printf("%s",UNIDAD);/*14 bytes*/

	 if(ticket_recieved == normal)
	 {
		 ticket_info.normal++;
		 printf("Costo: $7\r\n");/*11 bytes*/
	 }
	 if(ticket_recieved == mitad)
	 {
		 ticket_info.mitad++;
		 printf("Costo: $3.5\r\n");/*11 bytes*/
	 }
	 if(ticket_recieved == transvale)
	 {
		 ticket_info.transvale++;
		 printf("Costo: TRANSVALE\r\n");/*11 bytes*/
	 }
	 
	 
	 strcat(hora_ticket, printer_time.hora);
	 printf("%s\r\n",hora_ticket);/*14 bytes*/
	 
	 strcat(fecha_ticket, printer_time.fecha);
	 printf("%s\r\n",fecha_ticket);/*17 bytes*/

	 ticket_info.folio++;
	 printf("Folio: %d",ticket_info.folio);/*11 bytes*/
	 printf("\n\n");
}
void printer_task(void *pvParameters)
{
	gpio_action_t boleto;
	uint8_t access;
	ticket_info.folio=-1;
	while(1)
	{
		if(xSemaphoreTake(gpio_printer_semaphore, ( TickType_t ) 100 ) == pdTRUE)
		{
			xSemaphoreGive(NTP_Request);
			if(xQueueReceive(gpio_state_queue, &(boleto), ( TickType_t ) 0) == pdPASS)
			{ 
				
				access = boleto;
				if(access != barra_derecha && access != barra_izquierda)
				{
					//UART_SetPrintPort(UART1);
					Centrar(CENTRAR_ON);
					vTaskDelay(100/portTICK_RATE_MS);
					Negritas(BOLD_ON);
					vTaskDelay(100/portTICK_RATE_MS);
					printer_print_TITLE((void *)NULL);
					Centrar(CENTRAR_OFF);
					Negritas(BOLD_OFF);
					vTaskDelay(100/portTICK_RATE_MS);
					printer_print_leftover(access);
					//UART_SetPrintPort(UART0);
					vTaskDelay(100/portTICK_RATE_MS);
				}
			}
		}
	}
}
