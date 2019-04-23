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
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "gpio.h"
#include "espconn.h"

//GPIO OUTPUT SETTINGS
#define  LED_IO_MUX  PERIPHS_IO_MUX_MTDO_U

#define  LED_IO_NUM  13
#define  LED_IO_FUNC FUNC_GPIO13
#define  LED_IO_PIN  GPIO_Pin_13

#define RED 13
#define GREEN 14
#define BLUE 15
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
void Mein_Task (void *pvParameters)
{
	while(1)
	{
		vTaskDelay( 500/portTICK_RATE_MS );
		printf("Mein Erste TASK hier\n");
	}
	
}
void RGB_LED(char r,char g, char b)
{
    GPIO_OUTPUT_SET(RED, r);
    GPIO_OUTPUT_SET(GREEN, g);
    GPIO_OUTPUT_SET(BLUE, b);
}
void secundaria(void *pvParameters)
{
    char toggle;
	while(1)
	{
        RGB_LED(1,0,0);//rojo
        printf("RED\r\n");
		vTaskDelay( 3000/portTICK_RATE_MS );
        RGB_LED(1,1,0);//amarillo
        printf("Yellow\r\n");
        vTaskDelay( 3000/portTICK_RATE_MS );
        RGB_LED(0,1,0);//verde
        printf("Green\r\n");
        vTaskDelay( 3000/portTICK_RATE_MS );
	}
}
void set_gpio(uint16 var)
{
    GPIO_ConfigTypeDef io_out_conf;
    io_out_conf.GPIO_IntrType = GPIO_PIN_INTR_DISABLE;
    io_out_conf.GPIO_Mode = GPIO_Mode_Output;
    io_out_conf.GPIO_Pin = var;
    io_out_conf.GPIO_Pullup = GPIO_PullUp_DIS;
    gpio_config(&io_out_conf);
}

void user_init(void)
{
    printf("SDK version:%s\n", system_get_sdk_version());
	printf("Mein Erste Code hier\n");
	//xTaskCreate( Mein_Task, (signed char *)"Der Task", 256, NULL, 2, NULL );

    set_gpio(GPIO_Pin_13);//R
    set_gpio(GPIO_Pin_14);//G
    set_gpio(GPIO_Pin_15);//B
    

    GPIO_OUTPUT_SET(RED, 0);
    GPIO_OUTPUT_SET(GREEN, 0);
    GPIO_OUTPUT_SET(BLUE, 0);
    xTaskCreate(secundaria, (signed char *)"Task2", 256,NULL, 3,NULL);
}
