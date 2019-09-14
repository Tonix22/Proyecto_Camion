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
#include "../custom_logic/common_logic.h"
#include "../application_drivers/Provisioning.h"


extern bool network_sucess;
extern SemaphoreHandle_t Provising;
extern SemaphoreHandle_t ip_connect;
extern QueueHandle_t     Flash_Flag;
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

    while(try < MAX_CONNECT_TRIES && NET_START == TRUE)
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
   
    uint8_t Flash_state;
    FlashData* Flash_data_configuration;
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
     *******************************************************************************/
    GPIO_init();
    /*
        Get data from flash:
            Wifi SSID & PASS
            Ticket info
            Email setup: user to send and time to send
    */
    
    #ifdef FIRST_TIME_SETUP

    Provisioning_init();
    xQueueReceive(Flash_Flag, &(Flash_data_configuration), ( TickType_t ) 20);
    /*  if system is errased put a local wifi
        network to make the first time setup
    */
    Flash_state = Flash_data_configuration->Saved;
    DEBUG_MAIN("Flash data MAIN: Configuration->Saved: %d\r\n",Flash_data_configuration->Saved);
    if(SYSTEM_ERRASED == Flash_state)
    {
        /*
        Acces point configuration setup
            Access point data
            NAME:central_comunication
            Pass:12345678
        */
        conn_AP_Init(Flash_state);
    }
    else
    {
        /*Preloaded data avoid first time setup*/
        #else
            Flash_data_configuration = (FlashData*) malloc(sizeof(FlashData));
            Avoid_Provisioning(Flash_data_configuration);
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
        wifi_init(Flash_data_configuration);

    #ifdef FIRST_TIME_SETUP
    }
    #endif
}
