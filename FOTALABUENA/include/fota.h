#ifndef __FOTA_H__
#define __FOTA_H__

void FOTA_CALL(void);
void fota_begin(void *pvParameters);
static void upgrade_download(int sta_socket, char *pusrdata, unsigned short length);
static void upgrade_fail(void);
#endif