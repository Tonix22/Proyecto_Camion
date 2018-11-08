#include "esp_common.h"
#include "freeRTOS_wrapper.h"
#include "timer_admin.h"
#include "hw_timer.h"

#define REG_WRITE(_r,_v)    (*(volatile uint32 *)(_r)) = (_v)
#define REG_READ(_r)        (*(volatile uint32 *)(_r))
#define WDEV_NOW()          REG_READ(0x3ff20c00)
#define OBS_TIME_MATCH 2000
#define FALSE_TIME_MATCH 500


uint32 tick_now2 = 0;
timer_t Timers[MAX_NUM_OF_TIMERS];

void hw_test_timer_cb(void)
{
    static uint16 j = 0;
    j++;
	uint8_t i;
	for(i = 0; i < MAX_NUM_OF_TIMERS; i ++)
	{
		if(Timers[i].active == true)
		{
			Timers[i].time_counter++;
		}
	}
	//refesh
    if ((WDEV_NOW() - tick_now2) >= 100000) 
	{
        static uint32 idx = 1;
        tick_now2 = WDEV_NOW();
        j = 0;
    }	
}
void Time_admin_task (void *pvParameters)
{
	while(1)
	{
		vTaskDelay(100/portTICK_RATE_MS);
		uint8_t i;
		for(i = 0; i < MAX_NUM_OF_TIMERS; i ++)
		{
			if(Timers[i].active == true)
			{
				if(Timers[i].time_counter >= Timers[i].time_compare)
				{
					Timers[i].time_counter = 0;
					Timers[i].call_back();
				}	
			}
		}
	}
}
void timer_start(uint8_t name)
{
	Timers[name].active = true;
}
void timer_stop(uint8_t name)
{
	Timers[name].active = false;
}
void time_reset(uint8_t name)
{
	Timers[name].time_counter = 0;
}
void timer_add(uint8_t name, uint16_t max_time, uint16_t init_time,bool on_off,Function where_to_go)
{	
	timer_t data;
	data.ID           = name;
	data.time_compare = max_time;
	data.time_counter = init_time;
	data.active       = on_off;
	data.call_back    = where_to_go;
	if(name<MAX_NUM_OF_TIMERS)
	{
		Timers[name]=data;
		printf("timer init: %d\r\n",name);
	}
	else
	{
		printf("Not enoght timer\r\n");
	}
}
void timer_init()
{
	hw_timer_init(1);
    hw_timer_set_func(hw_test_timer_cb);
    hw_timer_arm(100000);
	xTaskCreate(Time_admin_task,"Barras delanteras",256,NULL,4,NULL);
}