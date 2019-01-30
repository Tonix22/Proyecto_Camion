
#include "gpio_config.h"
#include "freeRTOS_wrapper.h"
/**********************************SAMPLE CODE*****************************/
#define ETS_GPIO_INTR_ENABLE()  _xt_isr_unmask(1 << ETS_GPIO_INUM)  //ENABLE INTERRUPTS
#define PRINTER_COMMAND_QUEUE_SIZE 1U
#define Release(SEMA) xSemaphoreGiveFromISR(SEMA,NULL);
#define Release_Normal(SEMA) xSemaphoreGive(SEMA);
#define NORMAL_QUE_SEND(QUE,data) xQueueSend( QUE, ( void * ) &data, ( TickType_t ) 0 );
#define CLEAR_ISR_FLAG GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, status);
#define CLEAR_PRINTER_QUEUE xQueueSendFromISR(printer_state_queue,( void * ) &action, &xHigherPriorityTaskWoken);
#define CLEAR_BAR_QUEUE xQueueSendFromISR(bar_state_queue,( void * ) &action, &xHigherPriorityTaskWoken);

QueueHandle_t printer_state_queue = NULL;
QueueHandle_t bar_state_queue = NULL;

SemaphoreHandle_t gpio_printer_semaphore = NULL;
SemaphoreHandle_t gpio_bar_semaphore = NULL;
os_timer_t gpio_handler;
gpio_action_t action;
static bool debouncer = false;

void io_intr_handler(void)
{
    uint32 status = GPIO_REG_READ(GPIO_STATUS_ADDRESS);          //READ STATUS OF INTERRUPT
	portBASE_TYPE xHigherPriorityTaskWoken;
	/*Botones*/
	if(debouncer == false)
	{
		if ((status & GPIO_Pin_0) && debouncer == false )
		{
			action    = normal;
			debouncer = true;
			Release(gpio_printer_semaphore);
			os_timer_arm(&gpio_handler,500,0);
		}
		if ((status & GPIO_Pin_4) && debouncer == false)
		{
			action    = mitad;
			debouncer = true;
			Release(gpio_printer_semaphore);
			os_timer_arm(&gpio_handler,500,0);
		}
		if ((status & GPIO_Pin_5) && debouncer == false)
		{
			action    = transvale;
			debouncer = true;
			Release(gpio_printer_semaphore);
			os_timer_arm(&gpio_handler,500,0);
		}
		
		CLEAR_PRINTER_QUEUE;
	}

	//Barras
	if (status & GPIO_Pin_10) 
	{
		action = barra_derecha;
		//Release(gpio_bar_semaphore);
		CLEAR_BAR_QUEUE;
	}
	if (status & GPIO_Pin_12) 
	{
		action = barra_izquierda;
		//Release(gpio_bar_semaphore);
		CLEAR_BAR_QUEUE;
	}

	CLEAR_ISR_FLAG;
}
void gpio_release_and_send(void)
{
	debouncer = false;
}
void gpio_personal_config(uint16 esp_pin)
{
	GPIO_ConfigTypeDef *io_in_conf =(GPIO_ConfigTypeDef*)zalloc(sizeof(GPIO_ConfigTypeDef));
    io_in_conf->GPIO_IntrType = GPIO_PIN_INTR_NEGEDGE;
    io_in_conf->GPIO_Mode = GPIO_Mode_Input;
    io_in_conf->GPIO_Pin = esp_pin;
    io_in_conf->GPIO_Pullup = GPIO_PullUp_EN;
    gpio_config(io_in_conf);
}
void GPIO_init(void)
{
	/*sensores de paso*/
    gpio_personal_config(GPIO_Pin_12);
	gpio_personal_config(GPIO_Pin_10);
	/*Botones*/
	gpio_personal_config(GPIO_Pin_5);
	gpio_personal_config(GPIO_Pin_4);
	gpio_personal_config(GPIO_Pin_0);
	
	/* Create a queue capable of containing 1 unsigned char */
	printer_state_queue = xQueueCreate(PRINTER_COMMAND_QUEUE_SIZE, sizeof(uint8_t));
	bar_state_queue = xQueueCreate(PRINTER_COMMAND_QUEUE_SIZE, sizeof(uint8_t));

	gpio_printer_semaphore = xSemaphoreCreateMutex();
	gpio_bar_semaphore     = xSemaphoreCreateMutex();

	/*Each half second the botton could read a signal*/
 	os_timer_setfn(&gpio_handler,(os_timer_func_t *)gpio_release_and_send, NULL);

	if(printer_state_queue!=NULL && bar_state_queue!=NULL)
	{
		printf("Queue created\r\n");
	}

	if ((NULL != gpio_printer_semaphore) && (NULL != gpio_bar_semaphore)) 
	{
		printf("gpio sema created\r\n");
	} 
	else 
	{
		/* Return error */
		printf("gpio Queue error created\r\n");
	}
	/*block semaphore wating them for being realised*/
	xSemaphoreTake( gpio_printer_semaphore, ( TickType_t ) 0 );
	vTaskDelay(100/portTICK_RATE_MS);
	xSemaphoreTake( gpio_bar_semaphore, ( TickType_t ) 0 );
	
	gpio_intr_handler_register(io_intr_handler, NULL);
	ETS_GPIO_INTR_ENABLE();
}
