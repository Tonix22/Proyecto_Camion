#ifndef __GPIO_CONFIG_H__
#define __GPIO_CONFIG_H__
#include "esp_common.h"
#include "espconn.h"
#include "gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#define RED 13
#define GREEN 14
#define BLUE 15

static void gpio_input_config(uint16 esp_pin);
static void gpio_output_config(uint16 esp_pin);

void io_intr_handler(void);
void after_reset_enable(void);
void GPIO_init(void);

void RGB_LED(char r,char g, char b);
typedef enum {
    normal    = 0,
	mitad,
	transvale,
	barra_derecha,
	barra_izquierda,

}gpio_action_t;
#endif