#ifndef __BARRAS_H__
#define __BARRAS_H__
#include "gpio_config.h"
void false_move_check (void);
void obs_check_function (void);
static void BAR_CHECK (gpio_action_t action);
void barras_delanteras_task(void *pvParameters);
typedef struct bars
{
	uint16_t subidas;
	uint16_t bajadas;
	uint16_t obs;
}barras_t;	
#endif