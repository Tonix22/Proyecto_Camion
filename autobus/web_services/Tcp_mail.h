#ifndef __TCP_MAIL_H__
#define __TCP_MAIL_H__
#include "esp_common.h"
#include "espconn.h"
#include "../application_drivers/Flash_driver.h"
void email_setup(FlashData *Setup);
static void user_tcp_connect_cb(void *arg);
static void user_tcp_recon_cb(void *arg, sint8 err);
static void user_tcp_recv_cb(void *arg, char *pusrdata, unsigned short length);
static void user_tcp_sent_cb(void *arg);
static void user_tcp_discon_cb(void *arg);
void user_send_data(struct espconn *pespconn, uint8_t *data,uint8_t pack_zise);
void sending_mail(void);


#endif