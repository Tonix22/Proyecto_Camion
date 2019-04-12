#ifndef __NETWORK_H__
#define __NETWORK_H__
#include "Flash_driver.h"
void conn_AP_Init(void);
void wifi_init(void);
void wifi_setup(FlashData* Conection_data);
#endif