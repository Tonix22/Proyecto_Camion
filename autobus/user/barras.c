#include "c_types.h"
#include "esp_common.h"
#include "user_config.h"
#include "gpio_config.h"
#include "freeRTOS_wrapper.h"
#include "barras.h"
#include "data_base.h"

#define ON 	1
#define OFF 0
#define FALSE_MOVE_CB 0
#define OBST_CB 1
#define GPIO_READ(gpio_read) GPIO_INPUT_GET(gpio_read)

extern QueueHandle_t gpio_state_queue;
extern SemaphoreHandle_t gpio_bar_semaphore;

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
	printf("barras_system_init\r\n");
	
	while(1)
	{
		if(xSemaphoreTake( gpio_bar_semaphore, ( TickType_t ) 100 ) == pdTRUE)
		{
			if(xQueueReceive(gpio_state_queue, &(barra), ( TickType_t ) 0 ) == pdPASS)
			{ 
				access = barra;
				if(access == barra_derecha || access == barra_izquierda)
				{
					BAR_CHECK(access);
				}
			}
		}
	}
}
static void BAR_CHECK (gpio_action_t action)
{
	if(action == barra_izquierda)//S1
	{
		printf("S1\r\n");
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
		printf("S2\r\n");
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
		printf("false sec: %d \n",barras_data.obs);
		False_Timer();
	}
}
void obs_check_function (void)
{
	//os_timer_disarm(&false_move);
	if((GPIO_READ(10) == 0) && (GPIO_READ(12)==0))
	{
		obstruccion++;
		if(obstruccion == 8)
		{
			barras_data.obs+=2;
			obstruccion = 0;
			printf("obs sec: %d \n",barras_data.obs);
		}
		Obst_Timer();
	}
	else
	{ 
		//Pasanger pass
		if(bajar_flag == true)
		{
			barras_data.bajadas++;
			printf("bajada: %d \n",barras_data.bajadas);
		}
		if(subir_flag == true)
		{
			barras_data.subidas++;
			printf("subida: %d \n",barras_data.subidas);
		}
		obstruccion   = 0;
		Clear_bar_flags();
	}
}