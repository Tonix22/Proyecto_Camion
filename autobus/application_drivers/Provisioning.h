#ifndef __PROVISIONING_H__
#define __PROVISIONING_H__
#include "../application_drivers/Flash_driver.h"
void Provisioning_init(void);
void provisioning(void);
void Flash_thread(void* pvParameters);
void Avoid_Provisioning(FlashData* INFO);
#endif