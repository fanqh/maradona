#include "stm32f4xx_hal.h"
#include "msp.h"
#include "unity_fixture.h"

UARTEX_HandleTypeDef* MSP_Create_UARTEX_Handle(struct msp_factory * msp, int port) {
	
	if (msp == NULL || msp->board_config == NULL) 
			return NULL;
	
	switch(port)
	{
		case 1:
			if (msp->board_config->uart1 != NULL)	
				return msp->ll_huartex_create(msp->gpio_clk, msp->dma_clk, msp->irq_registry, msp->board_config->uart1);
			break;
			
		case 2:
			if (msp->board_config->uart2 != NULL)	
				return msp->ll_huartex_create(msp->gpio_clk, msp->dma_clk, msp->irq_registry, msp->board_config->uart2);
			break;
			
		case 3:
			if (msp->board_config->uart3 != NULL)	
				return msp->ll_huartex_create(msp->gpio_clk, msp->dma_clk, msp->irq_registry, msp->board_config->uart3);
			break;

		case 4:
			if (msp->board_config->uart4 != NULL)	
				return msp->ll_huartex_create(msp->gpio_clk, msp->dma_clk, msp->irq_registry, msp->board_config->uart4);
			break;			

		case 5:
			if (msp->board_config->uart5 != NULL)	
				return msp->ll_huartex_create(msp->gpio_clk, msp->dma_clk, msp->irq_registry, msp->board_config->uart5);
			break;

		case 6:
			if (msp->board_config->uart6 != NULL)	
				return msp->ll_huartex_create(msp->gpio_clk, msp->dma_clk, msp->irq_registry, msp->board_config->uart6);
			break;			
		
		default:
			break;
	}
	
	return NULL;
}
