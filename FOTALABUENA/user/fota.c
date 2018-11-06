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
/* Standard includes */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include "ota_config.h"
#include "esp_common.h"
#include "lwip/mem.h"
#include "lwip/sockets.h"
#include "lwip/inet.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "upgrade.h"
#include "fota.h"

/*********************global param define start ******************************/
static uint32 totallength = 0;
static uint32 sumlength = 0;
static bool flash_erased = false;
static xTaskHandle *ota_task_handle = NULL;
/*********************global param define end *******************************/

/******************************************************************************
 * FunctionName : upgrade_fail
 * Description  : recyle upgrade task, if OTA finish switch to run another bin
 * Parameters   :
 * Returns      : none
 *******************************************************************************/
static void upgrade_fail(void)
{
    totallength = 0;
    sumlength = 0;
    flash_erased = false;
    system_upgrade_deinit();
	printf("UPGRADE MAY FAIL\r\n");
	printf("upgrade flag is: %d",system_upgrade_flag_check());
	ota_task_handle = NULL;
	vTaskDelete(ota_task_handle);
}

/******************************************************************************
 * FunctionName : upgrade_download
 * Description  : parse http response ,and download remote data and write in flash
 * Parameters   : int sta_socket : ota client socket fd
 *                char *pusrdata : remote data
 *                length         : data length
 * Returns      : none
 *******************************************************************************/
static void upgrade_download(int sta_socket, char *pusrdata, unsigned short length)
{
    char *ptr = NULL;
    char *ptmp2 = NULL;
    char lengthbuffer[32];
	long int temporal = 0;
	bool FLASH_ERROR  = false;
    if (totallength == 0 && (ptr = (char *)strstr(pusrdata, "\r\n\r\n")) != NULL &&
    (ptr = (char *)strstr(pusrdata, "Content-Length")) != NULL) 
	{
		/*CASTING STUFF FROM BINARY FORMAT HEADER*/
        ptr = (char *) strstr(pusrdata, "\r\n\r\n");
		length -= ptr - pusrdata;
        length -= 4;
        ptr = (char *) strstr(pusrdata, "\r\nContent-Length: ");
        if (ptr != NULL) 
		{
            ptr += 18;
            ptmp2 = (char *) strstr(ptr, "\r\n");

            if (ptmp2 != NULL) 
			{
                memset(lengthbuffer, 0, sizeof(lengthbuffer));
                memcpy(lengthbuffer, ptr, ptmp2 - ptr);
                temporal = strtol(lengthbuffer,NULL,10);
				sumlength=(uint32)temporal;//GET THE BINARY FILE SIZE
				printf("sum lengh: %d\r\n",sumlength);
                if (sumlength > 0) 
				{
					FLASH_ERROR=system_upgrade(pusrdata, sumlength);
					
                    if (FLASH_ERROR == false) 
					{
                        system_upgrade_flag_set(UPGRADE_FLAG_IDLE);
						printf("system_upgrade_flag_set\r\n");
                        upgrade_fail();
                    }
                    flash_erased = true;
					printf("flash_erase==true");
					ptr = (char *) strstr(pusrdata, "\r\n\r\n");
					FLASH_ERROR=system_upgrade(ptr+4, length);
                    if (FLASH_ERROR==false) 
					{
						printf("false == system_upgrade(ptr + 4, length)\r\n");
                        system_upgrade_flag_set(UPGRADE_FLAG_IDLE);
                        upgrade_fail();
                    }
                    totallength += length;
                    printf("sumlength = %d\n", sumlength);
                    return;
                }
            } 
			else 
			{
                printf("sumlength failed\n");
                system_upgrade_flag_set(UPGRADE_FLAG_IDLE);
                upgrade_fail();
            }
        } 
		else 
		{
            printf("Content-Length: failed\n");
            system_upgrade_flag_set(UPGRADE_FLAG_IDLE);
            upgrade_fail();
        }
    } 
	else 
	{
        totallength += length;
		printf("totallen = %d\n", totallength);
		FLASH_ERROR=system_upgrade(pusrdata, length);
        if (FLASH_ERROR==false) 
		{
            system_upgrade_flag_set(UPGRADE_FLAG_IDLE);
            upgrade_fail();
        }
        if (totallength == sumlength) 
		{
            printf("upgrade file download finished.\n");

            if (upgrade_crc_check(system_get_fw_start_sec(), sumlength) != true) 
			{
                printf("upgrade crc check failed !\n");
                system_upgrade_flag_set(UPGRADE_FLAG_IDLE);
                upgrade_fail();
            }

            system_upgrade_flag_set(UPGRADE_FLAG_FINISH);
			if (system_upgrade_flag_check() == UPGRADE_FLAG_FINISH) 
			{
				printf("WORK DONE\r\n");
				system_upgrade_reboot();
			}
        } 
		else 
		{
            return;
        }
    }
    close(sta_socket);
    upgrade_fail();
}
/******************************************************************************
 * FunctionName : fota_begin
 * Description  : ota_task function
 * Parameters   : task param
 * Returns      : none
 *******************************************************************************/
void fota_begin(void *pvParameters)
{
    int recbytes;
    int sin_size;
    int sta_socket;
    char recv_buf[1460];
	uint8_t BINARY	  =   0;
    uint8 user_bin[9] = { 0 };
    struct sockaddr_in remote_ip;
    printf("Hello, welcome to client!\r\n");
    while (1) 
	{
        sta_socket = socket(PF_INET, SOCK_STREAM, 0);
		
        if (-1 == sta_socket) 
		{

            close(sta_socket);
            printf("socket fail !\r\n");
            continue;
        }
        printf("socket ok!\r\n");
        bzero(&remote_ip, sizeof(struct sockaddr_in));
        remote_ip.sin_family = AF_INET;
        remote_ip.sin_addr.s_addr = inet_addr(DEMO_SERVER);
        remote_ip.sin_port = htons(DEMO_SERVER_PORT);

        if(0 != connect(sta_socket,(struct sockaddr *)(&remote_ip),sizeof(struct sockaddr)))
        {
            close(sta_socket);
            printf("connect fail!\r\n");
            system_upgrade_flag_set(UPGRADE_FLAG_IDLE);
            upgrade_fail();
        }
        printf("connect ok!\r\n");
        char *pbuf = (char *) zalloc(512);
		BINARY=system_upgrade_userbin_check();
		//CHECK Which bin to run
        if (BINARY == UPGRADE_FW_BIN1) 
		{
            memcpy(user_bin, "user2.bin", 10);
			printf("update user2.bin!\r\n");
        } 
		else if (BINARY == UPGRADE_FW_BIN2) 
		{
            memcpy(user_bin, "user1.bin", 10);
			printf("update user1.bin!\r\n");
        }
		else
		{
			printf("\r\n WARNING FAKE BIN\r\n");
			printf("real system upgrade is: %s",system_upgrade_userbin_check());
		}
		//HTTP REQUEST
		sprintf(pbuf, "GET /%s HTTP/1.1\r\nHost: %s\r\n"pheadbuffer"", user_bin, DEMO_SERVER);
		printf(pbuf);
        if (write(sta_socket,pbuf,strlen(pbuf)+1) < 0) 
		{
            close(sta_socket);
            printf("send fail\n");
            free(pbuf);
            system_upgrade_flag_set(UPGRADE_FLAG_IDLE);
            upgrade_fail();
        }
        printf("send success\n");
        free(pbuf);
		//HERE GET THE HTTP PACKETS AND SAVE IT INTO THE FLASH IN ORDER TO LATER BOOT
        while ((recbytes = read(sta_socket, recv_buf, 1460)) >= 0) 
		{
			printf("recbytes = %d\n", recbytes);
            if (recbytes != 0) 
			{
                upgrade_download(sta_socket, recv_buf, recbytes);
            }
        }
		//in case of ERROR
        if (recbytes < 0) 
		{
            printf("read data fail!\r\n");
            close(sta_socket);
            system_upgrade_flag_set(UPGRADE_FLAG_IDLE);
            upgrade_fail();
        }
    }
}


/******************************************************************************
 * FunctionName : user_init
 * Description  : entry of user application, init user function here
 * Parameters   : none
 * Returns      : none
 *******************************************************************************/
void FOTA_CALL(void)
{
	//if there is wifi
	printf("BINARY NUMER %d",system_upgrade_userbin_check());
    system_upgrade_flag_set(UPGRADE_FLAG_START);
    system_upgrade_init();
    if (ota_task_handle == NULL) 
	{
		xTaskCreate(fota_begin, "fota_task", 2048, NULL, 1, ota_task_handle);

	}
}

