#ifndef __MQTTECHO_H__
#define __MQTTECHO_H__
#include "mqtt/MQTTClient.h"
static void messageArrived(MessageData* data);
static void mqtt_client_thread(void* pvParameters);
void user_conn_init(void);
void number_to_string(uint16_t val,uint8_t *string);
#endif