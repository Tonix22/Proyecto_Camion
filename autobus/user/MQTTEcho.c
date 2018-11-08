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
//#include "MQTTEcho.h"


#define MQTT_CLIENT_THREAD_NAME         "mqtt_client_thread"
#define MQTT_CLIENT_THREAD_STACK_WORDS  2048
#define MQTT_CLIENT_THREAD_PRIO         8

#define MQTT_BROKER  "io.adafruit.com"  /* MQTT Broker Address*/
#define MQTT_PORT    1883             /* MQTT Port*/
uint8_t AIO_USERNAME[]    =   "EmilioTonix";
uint8_t AIO_KEY[]         =  "8281440a417b4e16be3b67ba126247d0";

QueueHandle_t MQTT_Queue = NULL;
SemaphoreHandle_t MQTT_semaphore = NULL;
static xTaskHandle mqttc_client_handle;



static void messageArrived(MessageData* data)
{
    printf("Message arrived: %s\n", data->message->payload);
}

static void mqtt_client_thread(void* pvParameters)
{
    barras_t Que_handler;
    barras_t Data_Read;
    uint16_t pasajeros;
    printf("mqtt client thread starts\n");
    MQTTClient client;
    Network network;
    unsigned char sendbuf[80], readbuf[80] = {0};
    int rc = 0, count = 0;
    MQTTPacket_connectData connectData = MQTTPacket_connectData_initializer;
    pvParameters = 0;
    NetworkInit(&network);
    MQTTClientInit(&client, &network, 30000, sendbuf, sizeof(sendbuf), readbuf, sizeof(readbuf));

    char* address = MQTT_BROKER;

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

	connectData.username.cstring=AIO_USERNAME;
	connectData.password.cstring=AIO_KEY;
    connectData.MQTTVersion = 3;
    connectData.clientID.cstring = "MyFirstMQTT";// HOW I WILL BE CALLED

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
		printf("value reciebed: %s", rc);
    }

    while (1) 
	{
		if(xSemaphoreTake(MQTT_semaphore, ( TickType_t ) 200 ) == pdTRUE)
        {
            if(xQueueReceive(MQTT_Queue, &(Que_handler), ( TickType_t ) 0) == pdPASS)
            {
                Data_Read.bajadas  = Que_handler.bajadas;
                Data_Read.obs      = Que_handler.obs;
                Data_Read.subidas  = Que_handler.subidas;
                pasajeros          = Data_Read.subidas-Data_Read.bajadas;
                MQTTMessage message;
                char payload[4];
                char data_to_send[4];
                message.qos = QOS2;
                message.retained = 0;
                message.payload = payload;
                message.payloadlen = strlen(payload);
                //itoa (Data_Read.subidas,data_to_send,10);
                sprintf(payload, data_to_send);
                

                if ((rc = MQTTPublish(&client, "EmilioTonix/feeds/subidas", &message)) != 0) 
                {
                    printf("Return code from MQTT publish is %d\n", rc);
                } 
                else 
                {
                    printf("MQTT publish topic \"EmilioTonix/feeds/subidas\", message number is %d\n", count);
                }
                //itoa (Data_Read.bajadas,data_to_send,10);
                sprintf(payload, data_to_send);
                if ((rc = MQTTPublish(&client, "EmilioTonix/feeds/bajadas", &message)) != 0) 
                {
                    printf("Return code from MQTT publish is %d\n", rc);
                } 
                else 
                {
                    printf("MQTT publish topic \"EmilioTonix/feeds/bajadas\", message number is %d\n", count);
                }
                //itoa (Data_Read.bajadas,pasajeros,10);
                sprintf(payload, data_to_send);
                if ((rc = MQTTPublish(&client, "EmilioTonix/feeds/Pasajeros", &message)) != 0) 
                {
                    printf("Return code from MQTT publish is %d\n", rc);
                } 
                else 
                {
                    printf("MQTT publish topic \"EmilioTonix/feeds/Pasajeros\", message number is %d\n", count);
                }

            }
            xSemaphoreGive(MQTT_semaphore);
        }
        vTaskDelay(1000 / portTICK_RATE_MS);  //send every 1 seconds
    }   
        

    printf("mqtt_client_thread going to be deleted\n");
    vTaskDelete(NULL);
    return;
}

void user_conn_init(void)
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
    
    ret = xTaskCreate(mqtt_client_thread,
                      MQTT_CLIENT_THREAD_NAME,
                      MQTT_CLIENT_THREAD_STACK_WORDS,
                      NULL,
                      3,
                      &mqttc_client_handle);

    if (ret != pdPASS)  
    {
        printf("mqtt create client thread %s failed\n", MQTT_CLIENT_THREAD_NAME);
    }
}
