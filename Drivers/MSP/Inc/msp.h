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
	UARTEX_HandleTypeDef* (*ll_huartex_create)(	GPIO_ClockProviderTypeDef* 	gpio_clk,
																							DMA_ClockProviderTypeDef*		dma_clk,
																							IRQ_HandleRegistryTypeDef*	irq_registry,
																							const UARTEX_ConfigTypeDef* uartex_configs);
	
	
	void (*ll_huartex_destroy)(UARTEX_HandleTypeDef* h);	

	DMAEX_HandleTypeDef*	(*create_dmaex_handle)(struct msp_factory * msp, const DMA_ConfigTypeDef * dmacfg, const IRQ_ConfigTypeDef * irqcfg);
	void (*destroy_dmaex_handle)(struct msp_factory * msp, DMAEX_HandleTypeDef* handle);	
	
	/////////////////////////////////////////////////////////////////////////////
	// required functions
	int (*gpioex_init_by_config)(GPIOEX_TypeDef* ge, const GPIO_ConfigTypeDef* config, GPIO_ClockProviderTypeDef* clk);
	int	(*irq_handle_init_by_config)(IRQ_HandleTypeDef* h, const IRQ_ConfigTypeDef* config, IRQ_HandleRegistryTypeDef* registry);
	int	(*uartex_handle_init_by_config)(UARTEX_HandleTypeDef* h, const UART_ConfigTypeDef	*config, GPIOEX_TypeDef	*rxpin, GPIOEX_TypeDef *txpin, 
		DMAEX_HandleTypeDef *hdmaex_rx, DMAEX_HandleTypeDef *hdmaex_tx, IRQ_HandleTypeDef *hirq, const struct UARTEX_Operations		*ops);
	
	/////////////////////////////////////////////////////////////////////////////
	// test data
	void * testdata;
};

UARTEX_HandleTypeDef*	msp_create_huartex(struct msp_factory * msp, int port);
//void DMAEX_Handle_FactoryDestroy(DMAEX_HandleTypeDef* handle);

UARTEX_HandleTypeDef* msp_create_uartex_handle(struct msp_factory * msp, const UARTEX_ConfigTypeDef * cfg);
UARTEX_HandleTypeDef* msp_create_uartex_handle_by_port(struct msp_factory * msp, int port);

void MSP_Destroy_UARTEX_Handle(UARTEX_HandleTypeDef* huartex);



///////////////////////////////////////////////////////////////////////////////
DMAEX_HandleTypeDef*	msp_create_dmaex_handle(struct msp_factory * msp, 
	const DMA_ConfigTypeDef * dmacfg, const IRQ_ConfigTypeDef * irqcfg);

void msp_destroy_dmaex_handle(struct msp_factory * msp, DMAEX_HandleTypeDef* handle);


extern struct msp_factory MSP;

#endif

