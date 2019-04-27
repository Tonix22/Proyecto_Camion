#ifndef __COMMON_LOGIC_H__
#define __COMMON_LOGIC_H__
#include "esp_common.h"
void SSID_space_parse(char * word);
void half_of_string_number(char *ACTUAL,char* HALF);
void mail_data_parse(uint8_t *string,uint16_t value);
void mail_parse_time(uint8_t *data,uint16_t time);
#endif