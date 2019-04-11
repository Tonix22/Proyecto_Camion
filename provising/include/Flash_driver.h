#ifndef __FLASH_DRIVER_H__
#define __FLASH_DRIVER_H__

#include "esp_common.h"
#define WIFI_FLASH_DATA_SIZE 30

#define RUTA_DATA_SIZE 10
#define UNIDAD_DATA_SIZE 10
#define COSTO_DATA_SIZE 10

#define EMAIL_DATA_SIZE 40
#define TIME_DATA_SIZE 10

typedef struct
{
    bool Saved;
    uint8_t SSID_DATA [WIFI_FLASH_DATA_SIZE];
    uint8_t PASS_DATA [WIFI_FLASH_DATA_SIZE];

    uint8_t RUTA_DATA [UNIDAD_DATA_SIZE];
    uint8_t UNIDAD_DATA [RUTA_DATA_SIZE];
    uint8_t COSTO_DATA [COSTO_DATA_SIZE];

    uint8_t EMAIL_DATA [EMAIL_DATA_SIZE];
    uint8_t EMAIL_TIME [TIME_DATA_SIZE];

}FlashData;

void Flash_init(void);
void Flash_write(void);
FlashData* Flash_read(void);
void set_FLASH_SSID(char* NAME_SSID);
void set_FLASH_PASS(char* PASS_WIFI);
void set_FLASH_RUTA(char* RUTA_INFO);
void set_FLASH_UNIDAD(char* UNIDAD);
void set_FLASH_COSTO(char* COSTO);
void set_FLASH_EMAIL(char* EMAIL);
void set_FLASH_EMAIL_TIME(char* EMAIL);

#endif