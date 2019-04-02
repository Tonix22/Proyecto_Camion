/* Copyright(c) 2017, Spectrum Brands
 *
 * All Rights Reserved
 * Spectrum Brands Confidential
 */
/*
 * freeRTOS_wrapper.h
 *
 *  Created on: jan, 2018
 *      Author: A2e Technologies
 * Description: Definitions to fix discrepances between the NXP version of
 * FreeRTOS and the version used by Espressif.
 *
 */
#ifndef FREERTOS_WRAPPER_H_
#define FREERTOS_WRAPPER_H_

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/timers.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_libc.h"


typedef xSemaphoreHandle            SemaphoreHandle_t;
typedef xQueueHandle                QueueHandle_t;
typedef xTimerHandle                TimerHandle_t;
typedef uint32_t                    TickType_t;
typedef xTaskHandle                 TaskHandle_t;

#define spb_malloc(s)               os_malloc(s)
#define spb_free(s)                 os_free(s)

#define spb_portTICK_PERIOD_MS      portTICK_RATE_MS

#endif /* FREERTOS_WRAPPER_H_ */
