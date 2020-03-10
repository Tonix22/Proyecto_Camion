#ifndef __GPIO_CONFIG_H__
#define __GPIO_CONFIG_H__
#include "esp_common.h"
#include "espconn.h"
#include "gpio.h"

#define DEBUG
#ifdef DEBUG
#define BARRA_DER GPIO_Pin_12
#define BARRA_IZQ GPIO_Pin_10
#define DER_GPIO 12
#define IZQ_GPIO 10
#else
#define BARRA_DER GPIO_Pin_0
#define BARRA_IZQ GPIO_Pin_5
#define DER_GPIO 0
#define IZQ_GPIO 5
#endif
void io_intr_handler(void);
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