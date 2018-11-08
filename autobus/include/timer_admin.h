#ifndef __TIMER_ADMIN_H__
#define __TIMER_ADMIN_H__

#define MAX_NUM_OF_TIMERS 4

typedef void (*Function)();
void timer_start(uint8_t name);
void timer_stop(uint8_t name);
void time_reset(uint8_t name);
void timer_add(uint8_t name, uint16_t max_time, uint16_t init_time,bool on_off,Function where_to_go);
typedef struct 
{
	uint8_t ID;
	uint16_t time_compare;
	uint16_t time_counter;
	bool 	 active;
	Function call_back;	
}timer_t;

#endif