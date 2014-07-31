#ifndef MSP_H
#define MSP_H

/** not sure if this module should be named as msp, or board, or hal. **/

#include "gpio.h"
#include "dma.h"
#include "irq.h"
#include "usart.h"

typedef struct 
{
	GPIO_ClockProviderTypeDef*					gpio_clk;
	DMA_ClockProviderTypeDef*						dma_clk;	
	IRQ_HandleRegistryTypeDef* 	irq_registry;
} MSP_TypeDef;

UARTEX_HandleTypeDef MSP_Create_UARTEX_Handle(MSP_TypeDef * msp);

extern MSP_TypeDef MSP;

#endif

