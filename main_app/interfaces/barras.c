#include "c_types.h"
#include "esp_common.h"
#include "user_config.h"
#include "../application_drivers/gpio_config.h"
#include "freeRTOS_wrapper.h"
#include "../interfaces/barras.h"
#include "../database/data_base.h"
#include "../web_services/MQTTEcho.h"
#include "../include/user_config.h"

#define GPIO_READ GPIO_INPUT_GET

extern QueueHandle_t bar_state_queue;
extern SemaphoreHandle_t gpio_bar_semaphore;

extern QueueHandle_t MQTT_Queue;
extern SemaphoreHandle_t MQTT_semaphore;
extern xTaskHandle mqttc_client_handle;

QueueHandle_t Back_Bar = NULL;
barras_t Back_bar_counter;
SemaphoreHandle_t Clear_back_bar_server = NULL;

static bool subir_flag    = false;
static bool bajar_flag    = false;
static bool obs_flag      = false;
static bool finish 		  = false;
barras_t barras_data;
uint8_t obstruccion;
os_timer_t false_move;
os_timer_t obs_check;
#define False_Timer() os_timer_arm(&false_move, 8000, 0)
#define Obst_Timer()  os_timer_arm(&obs_check, 250, 0)
#define Clear_bar_flags() \
		subir_flag    = false;\
		bajar_flag    = false;\
		obs_flag      = false;
#define Set_timer(timer,func)  \
		os_timer_disarm(&timer);\
		os_timer_setfn(&timer,(os_timer_func_t *)func, NULL);

void barras_delanteras_task(void *pvParameters)
{
	gpio_action_t barra;
	uint8_t access;

	Set_timer(false_move,false_move_check);
	Set_timer(obs_check,obs_check_function);

	Back_Bar  = xQueueCreate(1, sizeof(barras_t));
	Clear_back_bar_server = xSemaphoreCreateMutex();
	if(Back_Bar == NULL || Clear_back_bar_server == NULL)
	{
		DEBUG_BARRAS("sync back bar error\r\n");
	}
	else
	{
		xSemaphoreTake( Clear_back_bar_server, ( TickType_t ) 0 );
	}

	DEBUG_BARRAS("barras_system_init\r\n");
	
	while(gpio_bar_semaphore == NULL)
	{
		vTaskDelay(1000/portTICK_RATE_MS);
	}

	/*
		after reset the GPIO could detect false edges generating errors in
		sempahores and queues
	*/
	after_reset_enable();  

	while(1)
	{
		if(xSemaphoreTake( gpio_bar_semaphore, ( TickType_t ) 100 ) == pdTRUE)
		{
			if(xQueueReceive(bar_state_queue, &(barra), ( TickType_t ) 0 ) == pdPASS)
			{ 
				access = barra;
				if(access == barra_derecha || access == barra_izquierda)
				{
					BAR_CHECK(access);
				}
			}
		}
		if(xQueueReceive(Back_Bar, &(Back_bar_counter), ( TickType_t ) 100 ) == pdPASS)
		{
			barras_data.subidas+=Back_bar_counter.subidas;
			barras_data.bajadas+=Back_bar_counter.bajadas;
			xSemaphoreGive( Clear_back_bar_server);
			//pending obstruction
		}
	}
}
static void BAR_CHECK (gpio_action_t action)
{
	if(action == barra_izquierda)//S1
	{
		DEBUG_BARRAS("S1\r\n");
		if(bajar_flag == false && subir_flag == false)//nadie habia sido activado
		{
			//False_Timer();
			subir_flag = true;
			
		}
		if(bajar_flag == true)
		{
			obs_check_function();
			
		}
	}

	if(action == barra_derecha)//S2
	{
		DEBUG_BARRAS("S2\r\n");
		if(bajar_flag == false && subir_flag == false)//nadie habia sido activado
		{
			bajar_flag = true;
		}
		if(subir_flag == true)//s1 ya habia sido activado
		{
			obs_check_function();
		}
	}
}

void false_move_check (void)
{
	if(GPIO_READ(10) ^ GPIO_READ(12))
	{
		barras_data.obs+=8;
		DEBUG_BARRAS("false sec: %d \n",barras_data.obs);
		False_Timer();
	}
}
void obs_check_function (void)
{
	if((GPIO_READ(10) == 0) && (GPIO_READ(12)==0))
	{
		obstruccion++;
		if(obstruccion == 8)
		{
			barras_data.obs+=2;
			obstruccion = 0;
			DEBUG_BARRAS("obs sec: %d \n",barras_data.obs);
		}
		Obst_Timer();
	}
	else
	{ 
		//Pasanger pass
		if(bajar_flag == true)
		{
			barras_data.bajadas++;
			DEBUG_BARRAS("bajada: %d \n",barras_data.bajadas);
		}
		if(subir_flag == true)
		{
			barras_data.subidas++;
			DEBUG_BARRAS("subida: %d \n",barras_data.subidas);
		}
		obstruccion   = 0;
		Clear_bar_flags();
			
		if(MQTT_Queue!=NULL)
		{
			xQueueOverwrite(MQTT_Queue, &barras_data);
		}
		if(MQTT_semaphore!=NULL)
		{
			xSemaphoreGive(MQTT_semaphore);
			xTaskCreate ( mqtt_client_thread, MQTT_CLIENT_THREAD_NAME,
						MQTT_CLIENT_THREAD_STACK_WORDS,
						NULL,
						2,
						&mqttc_client_handle);
		}
	}	
}