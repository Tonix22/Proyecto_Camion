#include "esp_common.h"
#include "user_config.h"
#include "camera.h"
#include "gpio.h"
#include "freeRTOS_wrapper.h"

#define  LED_IO_MUX  PERIPHS_IO_MUX_MTDO_U

#define  LED_IO_NUM  13
#define  LED_IO_FUNC FUNC_GPIO13
#define  LED_IO_PIN  GPIO_Pin_13

#define SHUTTER 13
#define OFF 1 
#define ON 0

uint32_t frac[] = {1, 1, 2, 2, 2, 3, 4, 5, 6, 8, 10, 12, 16, 20, 25, 33, 40, 50, 66, 76, 100, 125, 166, 200, 250, 333, 416, 500, 600, 800, 1000, 1300, 1600, 2000, 2500, 3200, 4000, 5000, 6000, 8000, 10000, 13000, 15000, 20000, 25000, 30000, 0};

QueueHandle_t Camera_frame;
void set_gpio(uint16 var)
{
    GPIO_ConfigTypeDef io_out_conf;
    io_out_conf.GPIO_IntrType = GPIO_PIN_INTR_DISABLE;
    io_out_conf.GPIO_Mode = GPIO_Mode_Output;
    io_out_conf.GPIO_Pin = var;
    io_out_conf.GPIO_Pullup = GPIO_PullUp_DIS;
    gpio_config(&io_out_conf);
}

void Shutter_task(void *pvParameters)
{
    Camera data;
    uint32_t time_sleep;
    int32_t shots;
    uint32_t photo_speed;
    bool toogle = true;
    while(1)
    {
        if( xQueueReceive( Camera_frame, &( data ), ( TickType_t ) portMAX_DELAY ) )
        {
            printf("frame recieved:%d,%d,%d\r\n",data.sleep,data.shots,data.speed);
            if(data.speed != 46)
            {
                shots = (int32_t) data.shots;
                time_sleep = data.sleep*1000;
                vTaskDelay(time_sleep/portTICK_RATE_MS );
                toogle = 0;
                do
                {
                    GPIO_OUTPUT_SET(SHUTTER,ON);
                    photo_speed = frac[data.speed];
                    printf("speed: %d\r\n",photo_speed);
                    shots--;
                    vTaskDelay(photo_speed/portTICK_RATE_MS );
                    GPIO_OUTPUT_SET(SHUTTER,OFF);
                    vTaskDelay(500/portTICK_RATE_MS );
                }while(shots>=0);
            }
            else if(data.speed == 46)
            {
                if(toogle==true)
                {
                    toogle = 0;
                }
                else
                {
                    toogle = 1;
                }
                GPIO_OUTPUT_SET(SHUTTER,toogle);
            }
        }
        vTaskDelay(500/portTICK_RATE_MS);
    }
    vTaskDelete(NULL);
}
void Camara_init(void)
{
        set_gpio(GPIO_Pin_13);//R
        GPIO_OUTPUT_SET(SHUTTER,OFF);
        xTaskCreate(Shutter_task, (signed char *)"Task2", 256,NULL, 3,NULL);
        Camera_frame = xQueueCreate( 1, sizeof(Camera) );
}
