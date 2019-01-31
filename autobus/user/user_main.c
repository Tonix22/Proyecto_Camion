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
//#include "user_config.h"
//#include "ntp_time.h"
//#include "printer.h"
//#include "gpio_config.h"
//#include "data_base.h"
#include "barras.h"
//#include "network.h"
//#include "MQTTEcho.h"
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

/******************************************************************************
 * FunctionName : user_init
 * Description  : entry of user application, init user function here
 * Parameters   : none
 * Returns      : none
 *******************************************************************************/

 void user_init(void)
{
    //disable software watchdog
    //system_soft_wdt_feed();

    //Change BaudRate to 9600
    uart_user_init();
    vTaskDelay(5000/portTICK_RATE_MS);
    printf("User init\n");
    /******************************************************************************
     * FunctionName : GPIO_init
     * Description  : GPIOS are divided in Bars and printers
     * GPIO 12, 10 --> BARS [DER,IZQ]
     * GPIO 0,4,5  -->[NORMAL,MITAD,TRANSVALE]
     *******************************************************************************/

    GPIO_init();
	vTaskDelay(100/portTICK_RATE_MS);

    /******************************************************************************
     * FunctionName : printer_init
     * Description  : Initialize UART1, and create printer task
     * NTP_Request-->Semaphore 
     *******************************************************************************/

	printer_init();

    /******************************************************************************
     * FunctionName : barras_delanteras_task
     * Description  : Bar check logic whith semaphores and timers. 
     * NTP_Request-->Semaphore 
     *******************************************************************************/
    xTaskCreate(barras_delanteras_task,"Barras delanteras",1024,NULL,4,NULL);
    vTaskDelay(100/portTICK_RATE_MS);

    /******************************************************************************
     * FunctionName : wifi_init
     * Description  : Station and acces point mode
        Access point data
            NAME:central_comunication
            Pass:12345678
        Time_check: ntp server
            INIT MQTT after ntp services gets time.
            TcpLocalServer: PORT 1024 
     *******************************************************************************/
       /*wifi_init

    */
    wifi_init();
}
