#ifndef __MQTTECHO_H__
#define __MQTTECHO_H__
#include "mqtt/MQTTClient.h"

#define MQTT_CLIENT_THREAD_NAME         "mqtt_client_thread"
#define MQTT_CLIENT_THREAD_STACK_WORDS  2048
#define MQTT_CLIENT_THREAD_PRIO         8

static void messageArrived(MessageData* data);
void mqtt_client_thread(void* pvParameters);
void mqtt_init(void);
static void number_to_string(uint16_t val,uint8_t *string);
#endif