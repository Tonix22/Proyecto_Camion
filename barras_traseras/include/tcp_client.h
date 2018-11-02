#ifndef __TCP_CLIENT_H__
#define __TCP_CLIENT_H__
void tcp_client_task(void *pvParameters);
void user_esp_platform_check_ip(void);
static void user_send_data(uint8_t *data,uint8_t pack_zise);
#endif 