#ifndef MSP_H
#define MSP_H

#include "gpio.h"
#include "dma.h"
#include "irq.h"


typedef struct 
{
	GPIO_ClockProviderTypeDef*					gpio_clk;
	DMA_ClockProviderTypeDef*						dma_clk;	
	IRQ_HandlerObjectRegistryTypeDef* 	registry;	
} MSP_TypeDef;

extern MSP_TypeDef MSP;

#endif

