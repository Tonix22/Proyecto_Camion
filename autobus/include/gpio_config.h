#ifndef __GPIO_CONFIG_H__
#define __GPIO_CONFIG_H__
#include "esp_common.h"
#include "espconn.h"
#include "gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
void io_intr_handler(void);
void after_reset_enable(void);
static void gpio_personal_config(uint16 esp_pin);
void GPIO_init(void);
typedef enum {
    normal    = 0,
	mitad,
	transvale,
	barra_derecha,
	barra_izquierda,

}gpio_action_t;
#endif