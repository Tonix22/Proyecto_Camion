#include "Provisioning.h"
#include "freeRTOS_wrapper.h"
#include "../include/user_config.h"
//http server
#include "../web_services/http_server.h"
//Flash driver
#include "../application_drivers/Flash_driver.h"
//Files setting up 
#include "../web_services/Tcp_mail.h"
#include "../interfaces/printer.h"
//GPIO
#include "../application_drivers/gpio_config.h"

SemaphoreHandle_t Flash_Ready;
QueueHandle_t     Flash_Flag;
FlashData* Configuration;

extern SemaphoreHandle_t Provising;

/*this funcion setup semaphores and queues
used for the internal Provising logic*/
void Provisioning_init(void)
{
    Flash_Ready = xSemaphoreCreateMutex();
    Flash_Flag = xQueueCreate(1,sizeof(FlashData*));
    xSemaphoreTake(Flash_Ready,( TickType_t ) 0);

    xTaskCreate(Flash_thread,"Flash_thread",2048,NULL,4,NULL);
    while(xSemaphoreTake(Flash_Ready,( TickType_t ) milli_sec(200)) == pdFALSE );
}
void provisioning(void)
{
    http_server_netconn_init();
    /*Wait semaphore to finish provising task*/
    while(xSemaphoreTake( Provising, ( TickType_t ) 10000/portTICK_RATE_MS ) == pdFALSE);
    /*Task is deleted after provising*/
}
void Flash_thread(void* pvParameters)
{
    bool provising_flag = false;
    /*Flash pointers setup*/
    Flash_init();
    Configuration = Flash_read();
    DEBUG_MAIN("Flash data: Configuration->Saved: %d\r\n",Configuration->Saved);
    /*
        Check if data had been written before,
        if the answer is True Flash data 
        is ready to be used, else
        set up html provising mode
    */
    xQueueSend(Flash_Flag, ( void * )&(Configuration),( portTickType ) 10);
    xSemaphoreGive(Flash_Ready);
    

    if(SYSTEM_ERRASED == Configuration->Saved )
    {
        provising_flag = true;
        /******************************************************************************
        * FunctionName : http_server_netconn_init
        * Description  : Request wifi and printer data and save it into flash
        * HTTP page
        * Flash save system
        *******************************************************************************/
        DEBUG_MAIN("provising data\r\n");
        LED_YELLOW;

        /*save date gotten from html request page*/
        provisioning();

        /*read flash again*/
        Configuration = Flash_read();
    }

    /******************************************************************************
     * FunctionName : printer_init
     * Description  : Initialize UART1, and create printer task
     * NTP_Request-->Semaphore 
     *******************************************************************************/
    DEBUG_MAIN("configuration setup begin..\r\n");
	printer_init(Configuration);
    email_setup(Configuration);
    set_mail_time(Configuration);
    DEBUG_MAIN("configuration setup end..\r\n");

    if(provising_flag == true)
    {
        //update finished
        system_restart();
    }
    vTaskDelete(NULL);
}

void Avoid_Provisioning(FlashData* INFO)
{   
    #ifndef FIRST_TIME_SETUP
    /* Forced first time setup for testing purposes*/
    strcpy(INFO->SSID_DATA,   WIFI_SSID);
    strcpy(INFO->PASS_DATA,   WIFI_PASS);
    strcpy(INFO->RUTA_DATA,   ROUTE);
    strcpy(INFO->UNIDAD_DATA, BUS_ID);
    strcpy(INFO->COSTO_DATA,  NORMAL_TICKET);
    strcpy(INFO->EMAIL_DATA,  MAIL_RECIEVER);
    strcpy(INFO->EMAIL_TIME,  MAIL_TIME);

    DEBUG_MAIN("configuration setup begin..\r\n");
    printer_init(INFO);
    email_setup(INFO);
    set_mail_time(INFO);
    //DEBUG_MAIN("WIFI: %s\r\n",INFO->SSID_DATA);
    //DEBUG_MAIN("PASS: %s\r\n",INFO->PASS_DATA);
    DEBUG_MAIN("configuration setup end..\r\n");
    #endif
}