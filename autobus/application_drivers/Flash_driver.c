#include "../application_drivers/Flash_driver.h"

FlashData static_read;
FlashData static_save;

FlashData *Data_read;
FlashData *Data_save;

void Flash_init(void)
{
    Data_read = &static_read;
    Data_save = &static_save;
    Data_read->Saved = false;
}
/*Call this until all data is set*/
void Flash_write(void)
{
    //Data setup to write
    Data_save->Saved = TRUE;
    spi_flash_erase_sector(0x8c);
    vTaskDelay(100);
    printf("errase ok\r\n");
    spi_flash_write(0x8c000, (uint32 *) Data_save, sizeof(*Data_save));
    printf("write ok\r\n");
    vTaskDelay(100);
}
FlashData* Flash_read(void)
{
    spi_flash_read(0x8c000, (uint32 *)Data_read, sizeof(*Data_read));
    return Data_read;
    //printf("read ok\r\n");
    //printf("read: %s\r\n",Data_read->SSID_DATA);
}

void set_FLASH_SSID(char* NAME_SSID)
{
    if(Data_save!=NULL)
    {
        strcpy(Data_save->SSID_DATA,NAME_SSID);
    }
}

void set_FLASH_PASS(char* PASS_WIFI)
{
    if(Data_save!=NULL)
    {
        strcpy(Data_save->PASS_DATA,PASS_WIFI);
    }
}

void set_FLASH_RUTA(char* RUTA_INFO)
{
    if(Data_save!=NULL)
    {
        strcpy(Data_save->RUTA_DATA,RUTA_INFO);
    }
}

void set_FLASH_UNIDAD(char* UNIDAD)
{
    if(Data_save!=NULL)
    {
        strcpy(Data_save->UNIDAD_DATA,UNIDAD);
    }
}
void set_FLASH_COSTO(char* COSTO)
{
    if(Data_save!=NULL)
    {
        strcpy(Data_save->COSTO_DATA,COSTO);
    }
}
void set_FLASH_EMAIL(char* EMAIL)
{
    if(Data_save!=NULL)
    {
        strcpy(Data_save->EMAIL_DATA,EMAIL);
    }
}
void set_FLASH_EMAIL_TIME(char* TIME)
{
    if(Data_save!=NULL)
    {
        strcpy(Data_save->EMAIL_TIME,TIME);
    }
}