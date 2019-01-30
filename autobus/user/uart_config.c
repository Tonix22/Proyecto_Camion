

#include "uart_config.h"
void uart_user_init(void)
{
	uart_init_new();
	UART_ConfigTypeDef* uart_config = malloc(sizeof(UART_ConfigTypeDef));
    uart_config->baud_rate    = BIT_RATE_9600;
    uart_config->data_bits    = UART_WordLength_8b;
    uart_config->parity       = USART_Parity_None;
    uart_config->stop_bits    = USART_StopBits_1;
    uart_config->flow_ctrl    = USART_HardwareFlowControl_None;
    uart_config->UART_RxFlowThresh = 120;
    uart_config->UART_InverseMask = UART_None_Inverse;
    UART_ParamConfig(UART1, uart_config);
}