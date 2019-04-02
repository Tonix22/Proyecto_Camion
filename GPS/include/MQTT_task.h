#ifndef __MQTT_TASK_H__
#define __MQTT_TASK_H__
#include "esp_common.h"
#define MQTT_CLIENT_THREAD_NAME         "mqtt_client_thread"
#define MQTT_CLIENT_THREAD_STACK_WORDS  2048
#define MQTT_CLIENT_THREAD_PRIO         8

int mqtt_init(void);
void MQTT_set(void);
void mqtt_client_thread(void* pvParameters);

#endif
