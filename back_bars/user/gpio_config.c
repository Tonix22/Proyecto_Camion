
#include "freeRTOS_wrapper.h"
#include "user_config.h"
#include "esp_common.h"
#include "../include/gpio_config.h"
/**********************************SAMPLE CODE*****************************/
#define ETS_GPIO_INTR_ENABLE()  _xt_isr_unmask(1 << ETS_GPIO_INUM)  //ENABLE INTERRUPTS
#define CLEAR() 
#define Release(SEMA) xSemaphoreGiveFromISR(SEMA,NULL);

QueueHandle_t gpio_state_queue = NULL;
SemaphoreHandle_t gpio_bar_semaphore = NULL;
gpio_action_t action;

void io_intr_handler(void)
{
    uint32 status = GPIO_REG_READ(GPIO_STATUS_ADDRESS);          //READ STATUS OF INTERRUPT
	/*Barras*/
	if (status & BARRA_DER) 
	{
		action = barra_derecha;
		Release(gpio_bar_semaphore);
	}
	if (status & BARRA_IZQ) 
	{
		action = barra_izquierda;
		Release(gpio_bar_semaphore);
	}
	xQueueSendFromISR(gpio_state_queue,( void * ) &action,NULL);
	//should not add print in interruption, except that we want to debug something
    GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, status);       //CLEAR THE STATUS IN THE W1 INTERRUPT REGISTER
}

static void gpio_personal_config(uint16 esp_pin)
{
	GPIO_ConfigTypeDef io_in_conf;
    io_in_conf.GPIO_IntrType = GPIO_PIN_INTR_NEGEDGE;
    io_in_conf.GPIO_Mode = GPIO_Mode_Input;
    io_in_conf.GPIO_Pin = esp_pin;
    io_in_conf.GPIO_Pullup = GPIO_PullUp_EN;
    gpio_config(&io_in_conf);
}
void GPIO_init(void)
{
	/*sensores de paso*/
    gpio_personal_config(BARRA_DER);
	gpio_personal_config(BARRA_IZQ);
	
	/* Create a queue capable of containing 1 unsigned char */
	gpio_state_queue   = xQueueCreate(1, sizeof(uint8_t));
	gpio_bar_semaphore = xSemaphoreCreateMutex();

	if ((NULL != gpio_state_queue) && (NULL != gpio_bar_semaphore)) 
	{
		printf("gpio Queue created\r\n");
	} 
	else 
	{
		/* Return error */
		printf("gpio Queue error created\r\n");
	}
	xSemaphoreTake( gpio_bar_semaphore, ( TickType_t ) 0 );	
	gpio_intr_handler_register(io_intr_handler, NULL);
	ETS_GPIO_INTR_ENABLE();
}
