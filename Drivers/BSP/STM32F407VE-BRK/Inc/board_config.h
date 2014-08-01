#ifndef BOARD_CONFIG_H
#define BOARD_CONFIG_H

#include "usart.h"

typedef struct {
	
	UARTEX_ConfigTypeDef* uart1;
	UARTEX_ConfigTypeDef*	uart2;
	UARTEX_ConfigTypeDef* uart3;
	UARTEX_ConfigTypeDef*	uart4;
	UARTEX_ConfigTypeDef* uart5;
	UARTEX_ConfigTypeDef*	uart6;	
	
} Board_ConfigTypeDef;

#endif

