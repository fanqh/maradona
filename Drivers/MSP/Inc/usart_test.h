#ifndef USART_TEST_H
#define USART_TEST_H

#include <stdbool.h>
#include "stm32f4xx_hal.h"

/******************************************************************************

Init assertion:									reverted in DeInit?		potential power problem?
	port clock enabled						no										yes
	dma clock enabled							no										yes
	dma irq config & enabled			no										???
	dma handler exist							no										???
	uart clock enabled						yes										yes
	port configured	(not reset)		yes
	dma state -> READY 	x 2				yes
	rx/dma linked				x	2				no
	uart irq config & enabled			yes (config not cleared)
	uart configured, state->READY yes (config not cleared)
	uart enabled									no!
	
******************************************************************************/	

bool port_clock_enabled(GPIO_TypeDef* gpio_port);
bool gpio_modes_all_input(GPIO_TypeDef* gpiox, uint32_t pins);
bool gpio_modes_all_noninput(GPIO_TypeDef* gpiox, uint32_t pins);

bool dma1_clock_enabled(void);
bool dma2_clock_enabled(void);
bool irq_enabled(IRQn_Type IRQn);
// bool dma_handle_installed();
// bool dma_state_is_ready();



// bool uart_dmareg_enabled();
// bool dma_enabled();






#endif


