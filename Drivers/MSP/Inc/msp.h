#ifndef MSP_H
#define MSP_H

/** not sure if this module should be named as msp, or board, or hal. **/

#include "gpio.h"
#include "dma.h"
#include "irq.h"
#include "usart.h"
#include "board_config.h"

typedef struct 
{
	const Board_ConfigTypeDef*					board_config;
	
	GPIO_ClockProviderTypeDef*					gpio_clk;
	DMA_ClockProviderTypeDef*						dma_clk;	
	IRQ_HandleRegistryTypeDef* 					irq_registry;
	
	
	
	
	///////////////////////////////////////////////////////////////
	// the class uses (a.k.a. depends on) the following functions.
	// ll_ prefix for low level, sorry that i didn't find a better name
	UARTEX_HandleTypeDef* (*ll_huartex_create)(GPIO_ClockProviderTypeDef* 	gpio_clk,
																					DMA_ClockProviderTypeDef*		dma_clk,
																					IRQ_HandleRegistryTypeDef*	irq_registry,
																					const UARTEX_ConfigTypeDef* uartex_configs);
																										
	void (*ll_huartex_destroy)(UARTEX_HandleTypeDef* h);	
	
} MSP_TypeDef;

// UARTEX_HandleTypeDef* MSP_Create_UART1EX_Handle(MSP_TypeDef * msp);
UARTEX_HandleTypeDef* MSP_Create_UART2EX_Handle(MSP_TypeDef * msp);

extern MSP_TypeDef MSP;

#endif

