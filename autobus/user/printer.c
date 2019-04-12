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

#define DEBUG

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
//uint8_t TITULO[]      = {"RUTA 27\r\n\0"};
char* TITULO;
				/*IZQUIERDA*/

ticket_base_t ticket_info;

ticket_time printer_time;

//uint8_t UNIDAD[]      = {"Unidad: 2069\r\n\0"};
char* UNIDAD;
char* PRECIO;
char MITAD_PRECIO[8];

SemaphoreHandle_t NTP_Request;
extern QueueHandle_t time_state_queue;
extern QueueHandle_t printer_state_queue;
extern SemaphoreHandle_t gpio_printer_semaphore;

void printer_init(FlashData* Cfg)
{
	uint32_t temp;
	uint32_t decimal;
	uint32_t ten;
	uint8_t price_size;
	uint8_t i;
	uint8_t price_index = 0;

	NTP_Request = xSemaphoreCreateMutex();
	if(NTP_Request!=NULL)
	{
		xSemaphoreTake(NTP_Request,( TickType_t ) 10);
	}
	
	UART_SetPrintPort(UART1);
	vTaskDelay(10/portTICK_RATE_MS);
	printf("%s",Printer_Start);
	vTaskDelay(10/portTICK_RATE_MS);
	UART_SetPrintPort(UART0);

	TITULO = Cfg->RUTA_DATA;
	UNIDAD = Cfg->UNIDAD_DATA;
	PRECIO = Cfg->COSTO_DATA;

	//Ajustar el de mitad de precio
	price_size = strlen(PRECIO);
	do
	{
		for(i=0;i<price_size-1;i++)
		{
			ten*=10;
		}
		decimal+=(PRECIO[price_index]-0x30)*ten;
		ten=1;
		price_index++;
		price_size--;
	}
	while(price_size!=0);

    if(decimal%2 == 0)
	{
    
   	 	decimal=decimal/2;
   	 	sprintf(MITAD_PRECIO,"%d",decimal);
    }
	else
    {
   	 	decimal=decimal/2;
   	  	sprintf(MITAD_PRECIO,"%d.5",decimal);
    }

	#ifdef DEBUG
		printf("TITULO: %s\r\n",TITULO);
		printf("UNIDAD: %s\r\n",UNIDAD);
		printf("PRECIO: %s\r\n",PRECIO);
		printf("MITAD: %s\r\n",MITAD_PRECIO);
	#endif
}
static void Centrar(uint8_t ok)
{
	printf("%c",0x1B);
	printf("%c",0x61);
	if(ok==1)
	{
		uart_tx_one_char(UART0,0x01);
	}
	else
	{
		uart_tx_one_char(UART0,0x00);
	}
	vTaskDelay(100/portTICK_RATE_MS);
}
static void Negritas(uint8_t enable)
{
	printf("%c",0x1B);
	printf("%c",0x21);
    if(enable == 1)
    {
    	uart_tx_one_char(UART0,0x08);
    }
    else
    {
		uart_tx_one_char(UART0,0x08);
    }
	vTaskDelay(100/portTICK_RATE_MS);
}
void FEED(uint8_t feeds)
{
    Feed[2]=feeds;
    printf("%s",Feed);
}
void printer_print_TITLE(void)
{
	printf("RUTA: %s\r\n",TITULO);/*9 bytes*/
}
void printer_print_leftover(gpio_action_t ticket_recieved )
{
	//falta arreglar este queue
	struct tm * ntp_time_rcv;

	if(time_state_queue != NULL )
	{
		if(xQueueReceive(time_state_queue, &(ntp_time_rcv), ( TickType_t ) 100) )
		{
			sprintf(printer_time.hora,"%d:%d:%d\0",ntp_time_rcv->tm_hour,ntp_time_rcv->tm_min,ntp_time_rcv->tm_sec);
			sprintf(printer_time.fecha,"%d/%d/%d\0",ntp_time_rcv->tm_mday,(ntp_time_rcv->tm_mon)+1,(ntp_time_rcv->tm_year)%100);
		}

	}
		
	 printf("UNIDAD: %s\r\n",UNIDAD);/*14 bytes*/

	 if(ticket_recieved == normal)
	 {
		 ticket_info.normal++;
		 printf("Costo: $%s\r\n",PRECIO);/*11 bytes*/
	 }
	 if(ticket_recieved == mitad)
	 {
		 ticket_info.mitad++;
		 printf("Costo: $%s\r\n",MITAD_PRECIO);/*11 bytes*/
	 }
	 if(ticket_recieved == transvale)
	 {
		 ticket_info.transvale++;
		 printf("Costo: TRANSVALE\r\n");/*11 bytes*/
	 }
	 
	 printf("Hora: %s\r\n",printer_time.hora);/*14 bytes*/
	 
	 printf("Fecha: %s\r\n",printer_time.fecha);/*17 bytes*/

	 ticket_info.folio++;
	 printf("Folio: %d",ticket_info.folio);/*11 bytes*/
	 printf("\n\n");
}
void printer_task(void *pvParameters)
{
	gpio_action_t boleto;
	uint8_t access;

	printer_time.hora  = (char*)malloc(10);
	printer_time.fecha = (char*)malloc(10);
	sprintf(printer_time.hora,"12:00:00");
	sprintf(printer_time.fecha,"dd/mm/aa");

	ticket_info.folio=-1;

	while(1)
	{
		if(xSemaphoreTake(gpio_printer_semaphore, ( TickType_t ) 100 ) == pdTRUE)
		{
			/*check if semaphore is released*/
			xSemaphoreGive(NTP_Request);

			if(xQueueReceive(printer_state_queue, &(boleto), ( TickType_t ) 20) == pdPASS)
			{ 
				access = boleto;
				if(access != barra_derecha && access != barra_izquierda)
				{
					//UART_SetPrintPort(UART0);
					Centrar(CENTRAR_ON);
					Negritas(BOLD_ON);
					vTaskDelay(100/portTICK_RATE_MS);
					printer_print_TITLE();
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
