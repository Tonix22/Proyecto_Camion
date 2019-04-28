/*
 * ESPRSSIF MIT License
 *
 * Copyright (c) 2015 <ESPRESSIF SYSTEMS (SHANGHAI) PTE LTD>
 *
 * Permission is hereby granted for use on ESPRESSIF SYSTEMS ESP8266 only, in which case,
 * it is free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the Software is furnished
 * to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#include "esp_common.h"

#include "freeRTOS_wrapper.h"
#include "../include/user_config.h"
#include "../application_drivers/gpio_config.h"
#include "../web_services/ntp_time.h"
#include "../interfaces/printer.h"
#include "../interfaces/barras.h"
#include "../network_config/network.h"
#include "../web_services/http_server.h"
#include "../application_drivers/Flash_driver.h"
#include "../web_services/Tcp_mail.h"
#include "../custom_logic/common_logic.h"

#define LED_RED RGB_LED(1,0,0);
#define LED_YELLOW RGB_LED(1,1,0);
#define LED_GREEN RGB_LED(0,1,0);
extern bool network_sucess;
extern SemaphoreHandle_t Provising;
extern SemaphoreHandle_t ip_connect;
SemaphoreHandle_t Flash_Ready;
QueueHandle_t Flash_Flag;
FlashData* Configuration;
/******************************************************************************
 * FunctionName : user_rf_cal_sector_set
 * Description  : SDK just reversed 4 sectors, used for rf init data and paramters.
 *                We add this function to force users to set rf cal sector, since
 *                we don't know which sector is free in user's application.
 *                sector map for last several sectors : ABCCC
 *                A : rf cal
 *                B : rf init data
 *                C : sdk parameters
 * Parameters   : none
 * Returns      : rf cal sector
*******************************************************************************/



uint32 user_rf_cal_sector_set(void)
{
    flash_size_map size_map = system_get_flash_size_map();
    uint32 rf_cal_sec = 0;

    switch (size_map) {
        case FLASH_SIZE_4M_MAP_256_256:
            rf_cal_sec = 128 - 5;
            break;

        case FLASH_SIZE_8M_MAP_512_512:
            rf_cal_sec = 256 - 5;
            break;

        case FLASH_SIZE_16M_MAP_512_512:
        case FLASH_SIZE_16M_MAP_1024_1024:
            rf_cal_sec = 512 - 5;
            break;

        case FLASH_SIZE_32M_MAP_512_512:
        case FLASH_SIZE_32M_MAP_1024_1024:
            rf_cal_sec = 1024 - 5;
            break;
        case FLASH_SIZE_64M_MAP_1024_1024:
            rf_cal_sec = 2048 - 5;
            break;
        case FLASH_SIZE_128M_MAP_1024_1024:
            rf_cal_sec = 4096 - 5;
            break;
        default:
            rf_cal_sec = 0;
            break;
    }

    return rf_cal_sec;
}
void provisioning(void)
{
    http_server_netconn_init();
    /*Wait semaphore to finish provising task*/
    while(xSemaphoreTake( Provising, ( TickType_t ) 10000/portTICK_RATE_MS ) == pdFALSE);
    /*Task is deleted after provising*/
}
void Flash_thread(void* pvParameters)
{
    bool provising = false;
    /*Flash pointers setup*/
    Flash_init();
    Configuration = Flash_read();
    DEBUG_MAIN("Flash data: Configuration->Saved: %d\r\n",Configuration->Saved);
    /*
        Check if data had been written before,
        if the answer is True Flash data 
        is ready to be used, else
        set up html provising mode
    */
    xSemaphoreGive(Flash_Ready);
    xQueueSend(Flash_Flag, ( void * )&(Configuration->Saved),( portTickType ) 10);

    if(0xFF == Configuration->Saved )
    {
        provising = true;
        /******************************************************************************
        * FunctionName : http_server_netconn_init
        * Description  : Request wifi and printer data and save it into flash
        * HTTP page
        * Flash save system
        *******************************************************************************/
        DEBUG_MAIN("provising data\r\n");
        LED_YELLOW;

        /*save date gotten from html request page*/
        provisioning();

        /*read flash again*/
        Configuration = Flash_read();
    }

    /******************************************************************************
     * FunctionName : printer_init
     * Description  : Initialize UART1, and create printer task
     * NTP_Request-->Semaphore 
     *******************************************************************************/
    DEBUG_MAIN("configuration setup begin..\r\n");
	printer_init(Configuration);
    email_setup(Configuration);
    set_mail_time(Configuration);
    DEBUG_MAIN("configuration setup end..\r\n");

    if(provising == true)
    {
        //update finished
        system_restart();
    }
    
    vTaskDelete(NULL);
}
void services_thread(void* pvParameters)
{
    bool NET_START = TRUE;
    uint8_t try;
    DEBUG_MAIN("service thread\r\n");
    ip_connect = xSemaphoreCreateMutex();
    if(ip_connect!=NULL)
    {
        xSemaphoreTake( ip_connect, ( TickType_t ) 0 );
    }
    DEBUG_MAIN("semaphore waiting ip connect..\r\n");
    //Holding while wifi connect

    while(xSemaphoreTake( ip_connect, ( TickType_t ) milli_sec(1000) ) == pdFALSE);
    if(network_sucess == FALSE)
    {
        DEBUG_MAIN("wifi and info data incorrect\r\n");
        LED_RED;
    }
    else
    {
         DEBUG_MAIN("wifi and info data correct\r\n");
         LED_GREEN;
    }

    while(try<MAX_CONNECT_TRIES && NET_START == TRUE)
    {
        if(network_sucess == TRUE)
        {
            /****************************************************
            ***********MQTT CONNECT INIT************************
            ****************************************************/

            mqtt_init();
            NET_START = FALSE;
            DEBUG_MAIN("MQTT INIT WELL\r\n");

            /****************************************************
            ***********NTP SERVER TASK INIT*********************
            ****************************************************/
            xTaskCreate(Time_check,"ntp server",1024,NULL,3,NULL);
            /****************************************************/
        }
        else
        {
            vTaskDelay(milli_sec(2000));
             try++;
        }

    }
    if(try == MAX_CONNECT_TRIES)
    {
        DEBUG_MAIN("NET services Fail\r\n");
    }

    /******************************************************************************
     * FunctionName : printer task
     * Description  : print ticket info 
     *******************************************************************************/
    xTaskCreate(printer_task, "Printer Task", 1024, NULL, 4, NULL );
    vTaskDelay(milli_sec(500));

    /******************************************************************************
     * FunctionName : barras_delanteras_task
     * Description  : Bar check logic whith semaphores and timers. 
     * NTP_Request-->Semaphore 
     *******************************************************************************/
    xTaskCreate(barras_delanteras_task,"Barras delanteras",1024,NULL,5,NULL);
    vTaskDelay(milli_sec(500));
    vTaskDelete(NULL);
}
/******************************************************************************
 * FunctionName : user_init
 * Description  : entry of user application, init user function here
 * Parameters   : none
 * Returns      : none
 *******************************************************************************/

 void user_init(void)
{
   
    uint8_t system_ready;
    //disable software watchdog
    //system_soft_wdt_feed();

    //Change BaudRate to 9600
    uart_user_init();
    DEBUG_MAIN("User init\n");
    /******************************************************************************
     * FunctionName : GPIO_init
     * Description  : GPIOS are divided in Bars and printers
     * GPIO 12, 10 --> BARS [DER,IZQ]
     * GPIO 0,4,5  -->[NORMAL,MITAD,TRANSVALE]
     * TODO : Add reset data button and config state
     *******************************************************************************/
    GPIO_init();
    /*
        Get data from flash:
            Wifi SSID & PASS
            Ticket info
            Email setup: user to send and time to send
    */
    
    #ifdef FIRST_TIME_SETUP

    vTaskDelay(500/portTICK_RATE_MS);

    Flash_Ready = xSemaphoreCreateMutex();
    Flash_Flag = xQueueCreate(1,sizeof(uint8_t));
    xSemaphoreTake(Flash_Ready,( TickType_t ) 0);

    xTaskCreate(Flash_thread,"Flash_thread",2048,NULL,4,NULL);
    DEBUG_MAIN("Acces Point init\n");
    while(xSemaphoreTake(Flash_Ready,( TickType_t ) milli_sec(100)) == pdFALSE );
    vTaskDelay(milli_sec(10));
    xQueueReceive(Flash_Flag, &(system_ready), ( TickType_t ) 20);

    if(0xFF == system_ready)
    {
        /*
        Acces point configuration setup
            Access point data
            NAME:central_comunication
            Pass:12345678
        */
        conn_AP_Init(system_ready);
    }
    else
    {
        /*Preloaded data avoid first time setup*/
        #else
        char MITAD_PRECIO[8];
        Configuration = (FlashData*) malloc(sizeof(FlashData));
        strcpy(Configuration->SSID_DATA,  WIFI_SSID);
        strcpy(Configuration->PASS_DATA,  WIFI_PASS);
        strcpy(Configuration->RUTA_DATA,  ROUTE);
        strcpy(Configuration->UNIDAD_DATA,BUS_ID);
        strcpy(Configuration->COSTO_DATA, NORMAL_TICKET);
        strcpy(Configuration->EMAIL_DATA, MAIL_RECIEVER);
        strcpy(Configuration->EMAIL_TIME, MAIL_TIME);

        DEBUG_MAIN("configuration setup begin..\r\n");
        printer_init(Configuration);
        email_setup(Configuration);
        set_mail_time(Configuration);
        DEBUG_MAIN("configuration setup end..\r\n");
        #endif
        vTaskDelay(milli_sec(100));
        /******************************************************************************
         * FunctionName : services_thread
         * Description  : intialize mqtt services, bar check and printer thread, but
         * before this, it holds the wifi semaphore, which is set when wifi is connected.
         *  
         *******************************************************************************/
        xTaskCreate(services_thread,"services_thread",2048,NULL,4,NULL);
        vTaskDelay(milli_sec(10));
        /******************************************************************************
         * FunctionName : wifi_init
         * Description  : gets the data given by flash, and connects to acces point
         * 
         *******************************************************************************/
        wifi_init(Configuration);

    #ifdef FIRST_TIME_SETUP
    }
    #endif
}
