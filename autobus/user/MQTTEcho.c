/*******************************************************************************
 * Copyright (c) 2014 IBM Corp.
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompany this distribution.
 *
 * The Eclipse Public License is available at
 *    http://www.eclipse.org/legal/epl-v10.html
 * and the Eclipse Distribution License is available at
 *   http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * Contributors:
 *    Ian Craggs - initial API and implementation and/or initial documentation
 *******************************************************************************/

#include <stddef.h>

#include "freeRTOS_wrapper.h"

#include "mqtt/MQTTClient.h"

#include "user_config.h"

#include "barras.h"
#include "MQTTEcho.h"




#define MQTT_BROKER  "io.adafruit.com"  /* MQTT Broker Address*/
#define MQTT_PORT    1883             /* MQTT Port*/

uint8_t AIO_USERNAME[]    =   "EmilioTonix";
uint8_t AIO_KEY[]         =  "8281440a417b4e16be3b67ba126247d0";
uint8_t PROV_NAME[]       = "MyFirstMQTT";
QueueHandle_t MQTT_Queue = NULL;
SemaphoreHandle_t MQTT_semaphore = NULL;
xTaskHandle mqttc_client_handle;

const uint16_t Ten_pow[5]={1,10,100,1000,10000};
static unsigned char sendbuf[80], readbuf[80] = {0};
static barras_t Que_handler;
static barras_t Data_Read;
static uint16_t pasajeros;
static MQTTClient client;
static Network network;
static int rc = 0, count = 0;


static void messageArrived(MessageData* data)
{
    printf("Message arrived: %s\n", data->message->payload);
}

void mqtt_client_thread(void* pvParameters)
{
		if(xSemaphoreTake(MQTT_semaphore, ( TickType_t ) 1000 ) == pdTRUE)
        {
            if(xQueueReceive(MQTT_Queue, &(Que_handler), ( TickType_t ) 0) == pdPASS)
            {
                Data_Read.bajadas  = Que_handler.bajadas;
                Data_Read.obs      = Que_handler.obs;
                Data_Read.subidas  = Que_handler.subidas;
                pasajeros          = Data_Read.subidas-Data_Read.bajadas;

                MQTTMessage* message = malloc(sizeof(MQTTMessage));
                char* payload   = malloc(sizeof(char)*4);

                message->qos = QOS2;
                message->retained = 0;
                message->payload = payload;
                message->payloadlen = strlen(payload);

                number_to_string(Data_Read.subidas,payload);
                printf("before update \n");
                if ((rc = MQTTPublish(&client, "EmilioTonix/feeds/subidas", message)) != 0) 
                {
                    printf("Return code from MQTT publish is %d\n", rc);
                } 
                else 
                {
                    printf("MQTT publish topic \"EmilioTonix/feeds/subidas\", message number is %d\n", count);
                }
                printf("after update \n");


                number_to_string(pasajeros,payload);

                if ((rc = MQTTPublish(&client, "EmilioTonix/feeds/Pasajeros", message)) != 0) 
                {
                    printf("Return code from MQTT publish is %d\n", rc);
                } 
                else 
                {
                    printf("MQTT publish topic \"EmilioTonix/feeds/Pasajeros\", message number is %d\n", count);
                }
                free(payload);
                free(message);
                //xSemaphoreGive(MQTT_semaphore);
            }
        }   
        vTaskDelete(mqttc_client_handle);
}

static void number_to_string(uint16_t val,uint8_t *string)
{
    string[0]='\0';
    string[1]='\0';
    string[2]='\0';
    string[3]='\0';

    string[0] = (val/1000)+0x30;
    string[1] = ((val%1000)/100)+0x30;
    string[2] = ((val%100)/10)+0x30;
    string[3] = (val%10)+0x30;
}

void mqtt_init(void)
{
    int ret;
    MQTT_Queue     = xQueueCreate(1, sizeof(barras_t));
    MQTT_semaphore = xSemaphoreCreateMutex();
    if ((NULL != MQTT_Queue) && (NULL != MQTT_semaphore) ) 
	{
        xSemaphoreTake( MQTT_semaphore, ( TickType_t ) 0 );
		printf("Mqtt sema and queue\r\n");
	} 
	else 
	{
		/* Return error */
		printf("Mqtt sema and queue erro\r\n");
	}

    char* address = malloc(sizeof(char)*16);
    address = MQTT_BROKER;
    printf("mqtt client thread starts\n");   
    //pvParameters = 0;
    MQTTPacket_connectData connectData = MQTTPacket_connectData_initializer;
    NetworkInit(&network);

    MQTTClientInit(&client, &network, 30000, sendbuf, sizeof(sendbuf), readbuf, sizeof(readbuf));

    if ((rc = NetworkConnect(&network, address, MQTT_PORT)) != 0) 
	{
        printf("Return code from network connect is %d\n", rc);
    }

#if defined(MQTT_TASK)

    if ((rc = MQTTStartTask(&client)) != pdPASS) 
	{
        printf("Return code from start tasks is %d\n", rc);
    } 
	else 
	{
        printf("Use MQTTStartTask\n");
    }

#endif

	connectData.username.cstring = AIO_USERNAME;
	connectData.password.cstring = AIO_KEY;
    connectData.MQTTVersion = 3;
    connectData.clientID.cstring = PROV_NAME;// HOW I WILL BE CALLED

    if ((rc = MQTTConnect(&client, &connectData)) != 0) 
	{
        printf("Return code from MQTT connect is %d\n", rc);
    } 
	else 
	{
        printf("MQTT Connected\n");
    }
	
    if ((rc = MQTTSubscribe(&client, "EmilioTonix/feeds/LED", 2, messageArrived)) != 0) 
	{
        printf("Return code from MQTT subscribe is %d\n", rc);
    } 
	else 
	{
        printf("MQTT subscribe to topic \"EmilioTonix/feeds/LED\"\n");
    }

}
