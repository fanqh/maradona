#include "stm32f4xx_hal.h"
#include "msp.h"
#include "unity_fixture.h"

UARTEX_HandleTypeDef* MSP_Create_UART2EX_Handle(MSP_TypeDef * msp) {
	
	if (msp == NULL || msp->board_config == NULL || msp->board_config->uart2 == NULL)
			return NULL;
	
	return msp->ll_huartex_create(msp->gpio_clk, msp->dma_clk, msp->irq_registry, msp->board_config->uart2);
}
