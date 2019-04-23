#ifndef __NETWORK_H__
#define __NETWORK_H__
#include "Flash_driver.h"
void conn_AP_Init(uint8_t flash_state);
void wifi_init(FlashData* Conection_data);
void wifi_setup(FlashData* Conection_data);
#endif