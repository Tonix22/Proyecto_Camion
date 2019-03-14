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

#include "MQTT_task.h"
#include "../../include/lwip/lwip/sockets.h"
#include "../../include/lwip/lwip/netdb.h"
#include "../../include/lwip/lwip/multi-threads/sockets_mt.h"
#include "network.h"

#define MQTT_PORT    1883             /* MQTT Port*/

char MQTT_BROKER[] = "io.adafruit.com\0";
uint8_t AIO_USERNAME[]    =   "EmilioTonix\0";
uint8_t AIO_KEY[]         =  "8281440a417b4e16be3b67ba126247d0\0";
uint8_t PROV_NAME[]       = "MyFirstMQTT\0";

QueueHandle_t MQTT_Queue = NULL;
SemaphoreHandle_t MQTT_semaphore = NULL;

typedef struct sockaddr_in sock_addr;

unsigned char sendbuf[80];
unsigned char readbuf[80];
bool MQTT_READY ; 


MQTTClient client;
int rc = 0, count = 0;
Network network;

int mqtt_lat = 0;
int mqtt_lon = 0;
bool INIT = true;
extern SemaphoreHandle_t Scan_semaphore;

void messageArrived(MessageData* data)
{
    printf("Message arrived: %s\n", data->message->payload);
}

void mqtt_client_thread(void* pvParameters)
{
    rc = NetworkConnect(&network, MQTT_BROKER, MQTT_PORT);
    printf("NetworkConnect is %i\r\n", rc);
    if(MQTT_READY == false)
    {
        MQTTPacket_connectData connectData = MQTTPacket_connectData_initializer;
        connectData.username.cstring = AIO_USERNAME;
        connectData.password.cstring = AIO_KEY;
        connectData.MQTTVersion = 3;
        connectData.clientID.cstring = "MyFirstMQTT";// HOW I WILL BE CALLED
        rc = MQTTConnect(&client, &connectData);
        printf("MQTTConnect is %i\r\n", rc);
        if(rc ==0)
        {
            MQTT_READY = TRUE;
        }
    }

    if(rc == 0)
    {
        if(xSemaphoreTake(MQTT_semaphore, ( TickType_t ) 1000 ) == pdTRUE)
        {
            if(xQueueReceive(MQTT_Queue, &(mqtt_lat), ( TickType_t ) 0) == pdPASS)
            {
                xQueueReceive(MQTT_Queue, &(mqtt_lon), ( TickType_t ) 0);

                MQTTMessage* message = malloc(sizeof(MQTTMessage));
                char* payload   = malloc(sizeof(char)*50);
                sprintf(payload,"{\r\n\"lat\":20.%d,\r\n\"lon\":-103.%d\r\n}",mqtt_lat,mqtt_lon);
                message->qos = QOS2;
                message->retained = 0;
                message->payload = payload;
                message->payloadlen = strlen(payload);
                printf("message: %s\r\n",payload);
                rc = MQTTPublish(&client, "EmilioTonix/feeds/GPS", message);

                if (rc != 0) 
                {
                    printf("SUB NAK\r\n");
                    MQTT_READY = FALSE;
                } 
                else 
                {
                    printf("SUB AKK\r\n");
                }

                free(payload);
                free(message);
                xSemaphoreGive(MQTT_semaphore);
            }
        }
    }   
    close(network.my_socket);
    vTaskDelay(5000/portTICK_RATE_MS);
    xSemaphoreGive(Scan_semaphore);
    vTaskDelete(NULL);
}


int mqtt_init(void)
{
    int ret;
    MQTT_Queue     = xQueueCreate(2, sizeof(int));
    MQTT_semaphore = xSemaphoreCreateMutex();
    
    if ((NULL != MQTT_Queue) && (NULL != MQTT_semaphore) ) 
	{
        xSemaphoreTake( MQTT_semaphore, ( TickType_t ) 0 );
		printf("Mqtt sema and queue\r\n");
	} 
	else 
	{
		// Return error 
		printf("Mqtt sema and queue error\r\n");
	}
    
    NetworkInit(&network);

    MQTTClientInit(&client, &network, 30000, sendbuf, sizeof(sendbuf), readbuf, 80);
    //xSemaphoreGive(Scan_semaphore);
   return ret;
}
