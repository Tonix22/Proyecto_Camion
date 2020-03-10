/*
 * printer.h
 *
 *  Created on: 04/07/2018
 *      Author: EmilioTonix
 */

#ifndef __PRINTER_H__
#define __PRINTER_H__
#include "../application_drivers/gpio_config.h"
#include "../application_drivers/Flash_driver.h"

void printer_init(FlashData* Configuration);
static void Centrar(uint8_t ok);
static void Negritas(uint8_t enable);
void FEED(uint8_t feeds);
void printer_print_TITLE(void);
void printer_print_leftover(gpio_action_t ticket_type );
void printer_task(void *pvParameters);
typedef struct
{
	char *hora;
	char *fecha;
}ticket_time;
#endif /* INCLUDE_PRINTER_H_ */

