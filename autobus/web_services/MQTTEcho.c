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
#include "../include/user_config.h"

#include "freeRTOS_wrapper.h"

#include "mqtt/MQTTClient.h"

#include "user_config.h"

#include "../interfaces/barras.h"
#include "MQTTEcho.h"
#include "../../include/lwip/lwip/sockets.h"
#include "../../include/lwip/lwip/netdb.h"
#include "../../include/lwip/lwip/multi-threads/sockets_mt.h"

#define MQTT_PORT    1883             /* MQTT Port*/

char MQTT_BROKER[] = "io.adafruit.com\0";
uint8_t AIO_USERNAME[]    =   "EmilioTonix";
uint8_t AIO_KEY[]         =  "8281440a417b4e16be3b67ba126247d0";
uint8_t PROV_NAME[]       = "MyFirstMQTT";
QueueHandle_t MQTT_Queue = NULL;
SemaphoreHandle_t MQTT_semaphore = NULL;
xTaskHandle mqttc_client_handle;

uint16_t Ten_pow[5]={1,10,100,1000,10000};

typedef struct sockaddr_in sock_addr;

unsigned char sendbuf[80];
unsigned char readbuf[80];
bool MQTT_READY ; 

static barras_t Que_handler;
static barras_t Data_Read;
static uint16_t pasajeros;
MQTTClient client;
int rc = 0, count = 0;
Network network;

void messageArrived(MessageData* data)
{
    DEBUG_MQTT("Message arrived: %s\n", data->message->payload);
}
void MQTT_set(void)
{
    if(MQTT_READY == FALSE)
    {
        MQTTPacket_connectData connectData = MQTTPacket_connectData_initializer;
        connectData.username.cstring = AIO_USERNAME;
        connectData.password.cstring = AIO_KEY;
        connectData.MQTTVersion = 3;
        connectData.clientID.cstring = "MyFirstMQTT";// HOW I WILL BE CALLED
        rc = MQTTConnect(&client, &connectData);
        DEBUG_MQTT("MQTTConnect is %i\r\n", rc);
        if(rc == 0)
        {
            MQTT_READY = TRUE;
        }
    }
}
void mqtt_client_thread(void* pvParameters)
{
    rc = NetworkConnect(&network, MQTT_BROKER, MQTT_PORT);
    DEBUG_MQTT("NetworkConnect is %i\r\n", rc);
    MQTT_set();
    if(rc == 0)
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

                rc = MQTTPublish(&client, "EmilioTonix/feeds/subidas", message);

                while(rc!=0)
                {
                    DEBUG_MQTT("SUB NAK\r\n");
                    MQTT_READY = FALSE;
                    close(network.my_socket);
                    rc = NetworkConnect(&network, MQTT_BROKER, MQTT_PORT);
                    MQTT_set();
                    rc = MQTTPublish(&client, "EmilioTonix/feeds/subidas", message);
                }
                DEBUG_MQTT("SUB AKK\r\n");

                number_to_string(pasajeros,payload);
                rc = MQTTPublish(&client, "EmilioTonix/feeds/Pasajeros", message);

                while(rc!=0)
                {
                    DEBUG_MQTT("SUB NAK\r\n");
                    MQTT_READY = FALSE;
                    close(network.my_socket);
                    rc = NetworkConnect(&network, MQTT_BROKER, MQTT_PORT);
                    MQTT_set();
                    rc = MQTTPublish(&client, "EmilioTonix/feeds/Pasajeros", message);
                }
                DEBUG_MQTT("SUB AKK\r\n");
                
                free(payload);
                free(message);
                xSemaphoreGive(MQTT_semaphore);
            }
        }
    }   
        close(network.my_socket);
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

void mqtt_connect(void)
{
    
}

int mqtt_init(void)
{
    int ret;
    
    MQTT_Queue     = xQueueCreate(1, sizeof(barras_t));
    MQTT_semaphore = xSemaphoreCreateMutex();
    
    if ((NULL != MQTT_Queue) && (NULL != MQTT_semaphore) ) 
	{
        xSemaphoreTake( MQTT_semaphore, ( TickType_t ) 0 );
		DEBUG_MQTT("Mqtt sema and queue\r\n");
	} 
	else 
	{
		// Return error 
		DEBUG_MQTT("Mqtt sema and queue error\r\n");
	}
    
    
    NetworkInit(&network);

    MQTTClientInit(&client, &network, 30000, sendbuf, sizeof(sendbuf), readbuf, sizeof(readbuf));

   return ret;
}
