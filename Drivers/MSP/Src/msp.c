#include "errno_ex.h"
#include "stm32f4xx_hal.h"
#include "msp.h"
#include "unity_fixture.h"

UARTEX_HandleTypeDef* msp_create_uartex_handle_by_port(struct msp_factory * msp, int port) {
	
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

/** failure analysis 
1. valid args ( at least the ones used in func)
2. rxpin init fail
3. txpin init fail



**/



UARTEX_HandleTypeDef* msp_create_uartex_handle(struct msp_factory* msp, const UARTEX_ConfigTypeDef* cfg)															
{
	int ret;
	GPIOEX_TypeDef* rxpinH = NULL;
	GPIOEX_TypeDef* txpinH = NULL;
	DMAEX_HandleTypeDef* dmaExRxH = NULL;
	DMAEX_HandleTypeDef* dmaExTxH = NULL;
	IRQ_HandleTypeDef* irqH = NULL;
	UARTEX_HandleTypeDef* h = NULL;

	/** validate object **/
	if (msp == NULL || msp->dma_clk == NULL || msp->gpio_clk == NULL || msp->irq_registry == NULL || msp->create_dmaex_handle == NULL
			|| cfg == NULL)
		return NULL;
	
	rxpinH = malloc(sizeof(*rxpinH));
	if (rxpinH == NULL) { 
		errno = ENOMEM;
		goto fail0;
	}
	
	ret = GPIOEX_InitByConfig(rxpinH, cfg->rxpin, msp->gpio_clk);
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
	
	ret = GPIOEX_InitByConfig(txpinH, cfg->txpin, msp->gpio_clk);
	if (ret != 0)
	{
		errno = -ret;
		goto fail2;
	}
	
	if (cfg->dmarx && cfg->dmarx_irq)
	{
		dmaExRxH = msp->create_dmaex_handle(msp, cfg->dmarx, cfg->dmarx_irq);
		if (dmaExRxH == NULL)
			goto fail2;
	}
	
	if (cfg->dmatx && cfg->dmatx_irq)
	{
		dmaExTxH = msp->create_dmaex_handle(msp, cfg->dmatx, cfg->dmatx_irq);
		if (dmaExTxH == NULL)
			goto fail3;
	}
	
	if (cfg->uart_irq)
	{
		irqH = malloc(sizeof(*irqH));
		if (irqH == NULL)
		{
			errno = ENOMEM;
			goto fail4;
		}
		
		ret = IRQ_Handle_InitByConfig(irqH, cfg->uart_irq, msp->irq_registry);
		if (ret != 0)
		{
			errno = -ret;
			goto fail5;
		}
	}
	
	h = UARTEX_Handle_Ctor(cfg->uart->Instance, &cfg->uart->Init, rxpinH, txpinH, dmaExRxH, dmaExTxH, irqH, cfg->uartex_ops);
	if (h == NULL)
		goto fail5;
	
	return h;
	
	fail5:	if (cfg->uart_irq) free(irqH);
	fail4:	if (cfg->dmarx && cfg->dmatx_irq) DMAEX_Handle_FactoryDestroy(dmaExTxH);
	fail3:	if (cfg->dmatx && cfg->dmarx_irq) DMAEX_Handle_FactoryDestroy(dmaExRxH);
	fail2: 	free(txpinH);
	fail1:	free(rxpinH);
	fail0:	return NULL;
}

DMAEX_HandleTypeDef*	msp_create_dmaex_handle(struct msp_factory * msp, 
	const DMA_ConfigTypeDef * dma_config, const IRQ_ConfigTypeDef * irq_config)
{
	int ret;
	
	DMAEX_HandleTypeDef* dmaExH;
	IRQ_HandleTypeDef* irqH;
	
	irqH = malloc(sizeof(*irqH));
	if (irqH == NULL) {
		errno = ENOMEM;
		return NULL;
	}
	
	ret = IRQ_Handle_InitByConfig(irqH, irq_config, msp->irq_registry);
	if (ret != 0)
	{
		errno = -ret;
		free(irqH);
		return NULL;
	}
	
	dmaExH = malloc(sizeof(*dmaExH));
	if (dmaExH == NULL)
	{
		free(irqH);
		errno = ENOMEM;
		return NULL;
	}
	
	ret = DMAEX_Handle_InitByConfig(dmaExH, dma_config, msp->dma_clk, irqH);
	if (ret != 0)
	{
		free(irqH);
		free(dmaExH);
		errno = -ret;
		return NULL;
	}

	return dmaExH;
}

