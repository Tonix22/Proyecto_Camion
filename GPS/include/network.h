#ifndef __NETWORK_H__
#define __NETWORK_H__
#include "c_types.h"
static void conn_AP_Init(void);
void wifi_init(void);
void scan_done(void *arg, STATUS status);
void Scan_Task (void *pvParameters);
#endif