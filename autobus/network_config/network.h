#ifndef __NETWORK_H__
#define __NETWORK_H__
#include "../application_drivers/Flash_driver.h"
void conn_AP_Init(uint8_t flash_state);
void wifi_init(FlashData* Conection_data);
#endif