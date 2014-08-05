#ifndef MSP_H
#define MSP_H

/** not sure if this module should be named as msp, or board, or hal. **/

#include "gpio.h"
#include "dma.h"
#include "irq.h"
#include "usart.h"
#include "board_config.h"


struct msp_factory
{
	const Board_ConfigTypeDef*					board_config;
	
	GPIO_ClockProviderTypeDef*					gpio_clk;
	DMA_ClockProviderTypeDef*						dma_clk;	
	IRQ_HandleRegistryTypeDef* 					irq_registry;
	
	/////////////////////////////////////////////////////////////////////////////
	// these are provided methods (called by user)
	UARTEX_HandleTypeDef* (*huartex_create)(struct msp_factory * msp, int num);
	void (*huratex_destroy)(UARTEX_HandleTypeDef* huartex);
	
	/////////////////////////////////////////////////////////////////////////////
	// these are required methods (called by me, myself)
	// ll_ prefix for low level, sorry that i didn't find a better name
	UARTEX_HandleTypeDef* (*ll_huartex_create)(GPIO_ClockProviderTypeDef* 	gpio_clk,
																					DMA_ClockProviderTypeDef*		dma_clk,
																					IRQ_HandleRegistryTypeDef*	irq_registry,
																					const UARTEX_ConfigTypeDef* uartex_configs);
																										
	void (*ll_huartex_destroy)(UARTEX_HandleTypeDef* h);	
	/////////////////////////////////////////////////////////////////////////////
	// test data
	void * testdata;
};

UARTEX_HandleTypeDef*	msp_create_huartex(struct msp_factory * msp, int port);

UARTEX_HandleTypeDef* MSP_Create_UARTEX_Handle(struct msp_factory * msp, int port);
void MSP_Destroy_UARTEX_Handle(UARTEX_HandleTypeDef* huartex);

extern struct msp_factory MSP;

#endif

