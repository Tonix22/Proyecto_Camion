#include <stdio.h>
#include <string.h>
#include "esp_common.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "espconn.h"
#include "printer.h"
#include "barras.h"
#include "data_base.h"
#include "Tcp_mail.h"
#define  ADD(name) total.##name=barras_data.##name+tcp_barr_rcv.##name
uint8_t  M_SUBIDAS[]  = {"SUBIDAS:000\r\n\0"};//14
uint8_t  M_BAJADAS[]  = {"BAJADAS:000\r\n\0"};
uint8_t  M_NORMAL[]   = {"Normal:000\r\n\0"};//13
uint8_t  M_TRANSVALE[]= {"Transvale:000\r\n\0"};//15
uint8_t  M_MITAD[]    = {"Mitad:000\r\n\0"};//12
uint8_t  M_OBSTRUC[]  = {"OBSTRUCCIONES:hh:mm:ss\r\n\0"};//25
uint8_t ALL[130];
TaskHandle_t xData_Base;
extern ticket_base_t ticket_info;
extern barras_t barras_data;
extern barras_t tcp_barr_rcv;
extern bool time_to_send;
os_timer_t mail_send;

void data_base_task (void *pvParameters)
{
    vTaskDelay(1000/portTICK_RATE_MS);
    barras_t total;

    total.bajadas=barras_data.bajadas+tcp_barr_rcv.bajadas;
    total.subidas=barras_data.subidas+tcp_barr_rcv.subidas;

    os_timer_setfn(&mail_send,(os_timer_func_t *)sending_mail, NULL);
    while(1)
    {
        vTaskDelay(100/portTICK_RATE_MS);
        if(time_to_send == true)
        {
            string_parse(M_SUBIDAS,barras_data.subidas);
            string_parse(M_BAJADAS,barras_data.bajadas);
            parse_time(M_OBSTRUC,barras_data.obs);

            string_parse(M_NORMAL,ticket_info.normal);
            string_parse(M_MITAD,ticket_info.mitad);
            string_parse(M_TRANSVALE,ticket_info.transvale);
            strcat(ALL,"Subject: REPORTE RUTA 27\r\n");
            strcat(ALL,"REPORTE\r\n\r\n");
            strcat(ALL,M_SUBIDAS);
            strcat(ALL,M_BAJADAS);
            strcat(ALL,M_OBSTRUC);
            strcat(ALL,M_NORMAL);
            strcat(ALL,M_MITAD);
            strcat(ALL,M_TRANSVALE);
            printf("%s",ALL);
            // falta para el folio
            os_timer_arm(&mail_send,50,0);
            vTaskDelete(xData_Base);
        }
    }
    
}
static void string_parse(uint8_t * string,uint16_t value)
{
    uint8_t i=0;
    do
    {
        i++;
    }while(string[i] != ':');
    i++;
    string[i] = (value/100)+0x30;
    string[i+1] = ((value%100)/10)+0x30;
    string[i+2] = (value%10)+0x30;
}
static void parse_time(uint8_t *data,uint16_t time)
{
	uint16_t temp;
	//Horas
	temp=time/3600;
    data[14]=0x30+(temp/10);//14,15
    data[15]=0x30+(temp%10);
    //Minutos
    temp=(time%3600)/60;
    data[17]=0x30+(temp/10);//17,18
    data[18]=0x30+(temp%10);
    //segundos
    temp=time%60;//20,21
    data[20]=0x30+(temp/10);//17,18
    data[21]=0x30+(temp%10);
}