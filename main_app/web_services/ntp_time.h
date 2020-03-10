
#ifndef __NTP_TIME_H__
#define __NTP_TIME_H__

#include "../application_drivers/Flash_driver.h"
void set_mail_time(FlashData* Setup_time);
void Time_check (void *pvParameters);
#endif
