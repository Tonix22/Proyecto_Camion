/*
 * printer.h
 *
 *  Created on: 04/07/2018
 *      Author: EmilioTonix
 */

#ifndef __PRINTER_H__
#define __PRINTER_H__
#include "gpio_config.h"

void printer_init(void);
static void Centrar(uint8_t ok);
static void Negritas(uint8_t enable);
void FEED(uint8_t feeds);
void printer_print_TITLE(void *arg);
void printer_print_leftover(gpio_action_t ticket_type );
void printer_task(void *pvParameters);
typedef struct
{
	uint8_t hora[9];
	uint8_t fecha[9];
}ticket_time;
#endif /* INCLUDE_PRINTER_H_ */

