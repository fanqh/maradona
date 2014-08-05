#include "errno_ex.h"
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

UARTEX_HandleTypeDef* msp_create_huartex(struct msp_factory* msp, int port)																							
{
	int ret;
	GPIOEX_TypeDef* rxpinH = NULL;
	GPIOEX_TypeDef* txpinH = NULL;
	DMAEX_HandleTypeDef* dmaExRxH = NULL;
	DMAEX_HandleTypeDef* dmaExTxH = NULL;
	IRQ_HandleTypeDef* irqH = NULL;
	UARTEX_HandleTypeDef* h = NULL;
	UARTEX_ConfigTypeDef* cfg = NULL;

	/** validate object **/
	if (msp == NULL || msp->board_config == NULL || msp->dma_clk == NULL || msp->gpio_clk == NULL || msp->irq_registry == NULL)
		return NULL;
	
	switch(port)
	{
		case 1:
			if (msp->board_config->uart1 != NULL)	
				cfg = msp->board_config->uart1;
			break;
			
		case 2:
			if (msp->board_config->uart2 != NULL)	
				cfg = msp->board_config->uart2;
			break;
			
		case 3:
			if (msp->board_config->uart3 != NULL)	
				cfg = msp->board_config->uart3;
			break;

		case 4:
			if (msp->board_config->uart4 != NULL)	
				cfg = msp->board_config->uart4;
			break;			

		case 5:
			if (msp->board_config->uart5 != NULL)	
				cfg = msp->board_config->uart5;
			break;

		case 6:
			if (msp->board_config->uart6 != NULL)	
				cfg = msp->board_config->uart6;
			break;			
		
		default:
			return NULL;
	}	
	
	rxpinH = malloc(sizeof(*rxpinH));
	if (rxpinH == NULL) { 
		errno = ENOMEM;
		goto fail0;
	}
	
	ret = GPIOEX_Init(rxpinH, cfg->rxpin->instance, &cfg->rxpin->init, msp->gpio_clk);
	if (ret != 0) {
		errno = -ret;
		goto fail1;
	}
	
	txpinH = malloc(sizeof(*txpinH));
	if (txpinH == NULL)
	{
		errno = ENOMEM;
		goto fail1;
	}
	
	ret = GPIOEX_Init(txpinH, cfg->txpin->instance, &cfg->txpin->init, msp->gpio_clk);
	if (ret != 0)
	{
		errno = -ret;
		goto fail2;
	}
	
	// if (hdmarx && dmarx_irq_config)
	if (cfg->dmarx && cfg->dmarx_irq)
	{
		dmaExRxH = DMAEX_Handle_FactoryCreate(msp->dma_clk, msp->irq_registry, cfg->dmarx, cfg->dmarx_irq);
		if (dmaExRxH == NULL)
			goto fail2;
	}
	
	// if (hdmatx && dmatx_irq_config)
	if (cfg->dmatx && cfg->dmatx_irq)
	{
		dmaExTxH = DMAEX_Handle_FactoryCreate(msp->dma_clk, msp->irq_registry, cfg->dmatx, cfg->dmatx_irq);
		if (dmaExTxH == NULL)
			goto fail3;
	}
	
	// if (hirq_uart)
	if (cfg->uart_irq)
	{
		// irqH = IRQ_Handle_Ctor(hirq_uart->irqn, hirq_uart->preempt_priority, hirq_uart->sub_priority, factory->registry);
		irqH = IRQ_Handle_CtorByConfig(cfg->uart_irq, msp->irq_registry);
		if (irqH == NULL)
			goto fail4;
	}
	
	h = UARTEX_Handle_Ctor(cfg->uart->Instance, &cfg->uart->Init, rxpinH, txpinH, dmaExRxH, dmaExTxH, irqH, cfg->uartex_ops);
	if (h == NULL)
		goto fail5;
	
	return h;
	
	fail5:	if (cfg->uart_irq) IRQ_Handle_Dtor(irqH);
	fail4:	if (cfg->dmarx && cfg->dmatx_irq) DMAEX_Handle_FactoryDestroy(dmaExTxH);
	fail3:	if (cfg->dmatx && cfg->dmarx_irq) DMAEX_Handle_FactoryDestroy(dmaExRxH);
	fail2: 	free(txpinH);
	fail1:	free(rxpinH);
	fail0:	return NULL;
}
