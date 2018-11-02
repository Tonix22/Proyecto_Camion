#ifndef __BARRAS_H__
#define __BARRAS_H__
#include "freeRTOS_wrapper.h"
void false_move_check (void);
void obs_check_function (void);
static void BAR_CHECK (gpio_action_t action);
void barras_delanteras_task(void *pvParameters);

#endif