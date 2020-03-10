#include "esp_common.h"
#include "freeRTOS_wrapper.h"
#include "espconn.h"
#include "../interfaces/printer.h"
#include "../interfaces/barras.h"
#include "data_base.h"
#include "../web_services/Tcp_mail.h"
#include "../custom_logic/common_logic.h"
#include "../include/user_config.h"


uint8_t  M_SUBIDAS[]  = {"SUBIDAS:000\r\n\0"};//14
uint8_t  M_BAJADAS[]  = {"BAJADAS:000\r\n\0"};//14
uint8_t  M_NORMAL[]   = {"Normal:000\r\n\0"};//13
uint8_t  M_TRANSVALE[]= {"Transvale:000\r\n\0"};//15
uint8_t  M_MITAD[]    = {"Mitad:000\r\n\0"};//12
uint8_t  M_OBSTRUC[]  = {"OBSTRUCCIONES:hh:mm:ss\r\n\0"};//25
uint8_t  LINK[] ={"https://io.adafruit.com/EmilioTonix/dashboards/myfirstmqtt\r\n"};//60
// agregar link de 
uint8_t ALL[ALL_size];
TaskHandle_t xData_Base;
extern ticket_base_t ticket_info;
extern barras_t barras_data;

//BARRAS TRASERAS INFO
extern barras_t udp_barr_rcv;
//END INFO

extern bool time_to_send;
os_timer_t mail_send;

void data_base_task (void *pvParameters)
{
    vTaskDelay(1000/portTICK_RATE_MS);
    barras_t total;

    total.bajadas=barras_data.bajadas+udp_barr_rcv.bajadas;
    total.subidas=barras_data.subidas+udp_barr_rcv.subidas;

    os_timer_setfn(&mail_send,(os_timer_func_t *)sending_mail, NULL);
    while(1)
    {
        vTaskDelay(100/portTICK_RATE_MS);
        if(time_to_send == true)
        {
            mail_data_parse(M_SUBIDAS,barras_data.subidas);
            mail_data_parse(M_BAJADAS,barras_data.bajadas);
            mail_parse_time(M_OBSTRUC,barras_data.obs);

            mail_data_parse(M_NORMAL,ticket_info.normal);
            mail_data_parse(M_MITAD,ticket_info.mitad);
            mail_data_parse(M_TRANSVALE,ticket_info.transvale);

            strcat(ALL,"Subject: REPORTE RUTA 27\r\n");//26
            strcat(ALL,"REPORTE\r\n\r\n");//11
            strcat(ALL,M_SUBIDAS);
            strcat(ALL,M_BAJADAS);
            strcat(ALL,M_OBSTRUC);
            strcat(ALL,M_NORMAL);
            strcat(ALL,M_MITAD);
            strcat(ALL,M_TRANSVALE);
            strcat(ALL,LINK);
            DEBUG_DATA_BASE("%s",ALL);
            // falta para el folio
            os_timer_arm(&mail_send,50,0);
            vTaskDelete(xData_Base);
        }
    }
}
