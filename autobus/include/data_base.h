
#ifndef __DATA_BASE_H__
#define __DATA_BASE_H__
#include "freertos/task.h"
#define ALL_size 190
void data_base_task (void *pvParameters);
static void string_parse(uint8_t *string,uint16_t value);
static void parse_time(uint8_t *data,uint16_t time);
typedef struct {
    uint16_t normal;
	uint16_t mitad;
	uint16_t transvale;
	uint16_t folio;
}ticket_base_t;


#endif