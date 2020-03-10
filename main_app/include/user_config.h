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

#ifndef __USER_CONFIG_H__
#define __USER_CONFIG_H__

/****************
    GENERAL 
 ****************/

/*DECLARATIONS*/
#define ENABLE 1
#define DISABLE 0

/****************
 ******MAIN****** 
 ****************/
#define FTS_EN                  (0)
/*FIRST TIME SET UP ENABLE*/

#if FTS_EN == ENABLE
    #define FIRST_TIME_SETUP
#endif
/*MAX CONNECT TRYS*/
#define MAX_CONNECT_TRIES       (5)

#define SYSTEM_ERRASED         (0XFF)

/****************
    DEBUG LEVELS 
 ****************/

/*MAIN*/
    #define DGB_MAIN            (1)
/*APPS drivers*/
    #define DBG_FLASH_DRIVER    (1)
    #define DBG_GPIO_CONFIG     (1)
    #define DBG_UART_CONFIG     (1)
/*data base*/
    #define DBG_DATA_BASE       (1)
/*interfaces*/
    #define DBG_BARRAS          (1)
    #define DBG_PRINTER         (0)
/*network*/
    #define DBG_NETWORK         (1)
/*web services*/
    #define DBG_HTTP            (1)
    #define DBG_MQTT            (1)
    #define DBG_NTP             (1)
    #define DBG_MAIL            (1)
    #define DBG_UDP             (1)
/****************
    GPIOS
 ****************/
#define LED_RED    RGB_LED(1,0,0);
#define LED_YELLOW RGB_LED(1,1,0);
#define LED_GREEN  RGB_LED(0,1,0);

#define GPIO_SPECIAL_BUTTON  GPIO_Pin_5 
#define GPIO_HALF_BUTTON     GPIO_Pin_4
#define GPIO_COMPLETE_BUTTON GPIO_Pin_0

/*sensors*/
#define RIGHT_SENSOR 10
#define GPIO_RIGHT_SENSOR GPIO_Pin_10

#define LEFT_SENSOR 12
#define GPIO_LEFT_SENSOR GPIO_Pin_12

/*RGB LED*/
#define GPIO_RED_LED    GPIO_Pin_13
#define GPIO_GREEN_LED  GPIO_Pin_14
#define GPIO_BLUE_LED   GPIO_Pin_15

/*DELAY MACRO*/
#define milli_sec(milis) milis/portTICK_RATE_MS

/****************
    DATA
 ****************/

/*WIFI*/
#define WIFI_SSID "IZZI-99CD"
#define WIFI_PASS "704FB81799CD"
/*MQTT */

/*Mail*/
#define MAIL_RECIEVER "emiliotonix%40gmail.com"
#define MAIL_TIME "22%3A38"

/*Tiket Data*/

#define ROUTE "27"
#define BUS_ID "2069"
#define NORMAL_TICKET "7"
#define SPECIAL_TYCKET "Costo: TRANSVALE\r\n"

/*Printer Commands*/

/*NTP*/

/*DEGUBG PRINTS*/
#if DGB_MAIN == ENABLE
    #define DEBUG_MAIN printf
#else
    #define DEBUG_MAIN
#endif

#if DBG_FLASH_DRIVER == ENABLE
    #define DEBUG_FLASH_DRIVER printf
#else
    #define DEBUG_FLASH_DRIVER
#endif

#if DBG_GPIO_CONFIG == ENABLE
    #define DEBUG_GPIO_CONFIG printf
#else
    #define DEBUG_GPIO_CONFIG
#endif

#if DBG_UART_CONFIG == ENABLE
    #define DEBUG_UART_CONFIG printf
#else
    #define DEBUG_UART_CONFIG
#endif

#if DBG_DATA_BASE == ENABLE
    #define DEBUG_DATA_BASE printf
#else
    #define DEBUG_DATA_BASE
#endif

#if DBG_BARRAS == ENABLE
    #define DEBUG_BARRAS printf
#else
    #define DEBUG_BARRAS
#endif

#if DBG_PRINTER == ENABLE
    #define DEBUG_PRINTER printf
#else
    #define DEBUG_PRINTER
#endif

#if DBG_NETWORK == ENABLE
    #define DEBUG_NETWORK printf
#else
    #define DEBUG_NETWORK
#endif

#if DBG_HTTP == ENABLE
    #define DEBUG_HTTP printf
#else
    #define DEBUG_HTTP
#endif

#if DBG_MQTT == ENABLE
    #define DEBUG_MQTT printf
#else
    #define DEBUG_MQTT
#endif

#if DBG_NTP == ENABLE
    #define DEBUG_NTP printf
#else
    #define DEBUG_NTP
#endif

#if DBG_MAIL == ENABLE
    #define DEBUG_MAIL printf
#else
    #define DEBUG_MAIL
#endif

#if DBG_UDP == ENABLE
    #define DEBUG_UDP printf
#else
    #define DEBUG_UDP
#endif

#endif

