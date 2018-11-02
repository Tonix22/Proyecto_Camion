#ifndef __BARRAS_H__
#define __BARRAS_H__
#include "freeRTOS_wrapper.h"
#include "gpio_config.h"
void false_move_check (void);
void obs_check_function (void);
static void BAR_CHECK (gpio_action_t action);
void barras_traseras_task(void *pvParameters);
typedef struct
{
    uint16_t subidas;
    uint16_t bajadas;
    uint16_t  obs;
}barras_t;
typedef enum
{
    BAJADA = 0,
    SUBIDA,
    OBSTRUCCION
}event_to_send_t;
#endif