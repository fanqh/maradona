#include "msp.h"

static void* passby = 0;

TEST_GROUP(MSP);

TEST_SETUP(MSP)
{}
	
TEST_TEAR_DOWN(MSP)
{}

/** this test is problematic **/	
UARTEX_HandleTypeDef* mock_ll_huartex_create(	GPIO_ClockProviderTypeDef* 	gpio_clk,
																							DMA_ClockProviderTypeDef*		dma_clk,
																							IRQ_HandleRegistryTypeDef*	irq_registry,
																							const UARTEX_ConfigTypeDef* uartex_configs)
{
	struct msp_factory* msp = (struct msp_factory*)passby;
	assert_param(msp);
	
	TEST_ASSERT_EQUAL_HEX32(msp->gpio_clk, gpio_clk);
	TEST_ASSERT_EQUAL_HEX32(msp->dma_clk, dma_clk);
	TEST_ASSERT_EQUAL_HEX32(msp->irq_registry, irq_registry);
	TEST_ASSERT_EQUAL_HEX32(msp->board_config->uart2, uartex_configs);
	
	return (UARTEX_HandleTypeDef*)(0xDEADBEEF);
};

TEST(MSP, CreateHuartEx)
{
	UARTEX_HandleTypeDef* h;

	UARTEX_ConfigTypeDef	uart2_cfg = {
		.uart = &UART2_DefaultConfig,
		.rxpin = &PD6_As_Uart2Rx_DefaultConfig,
		.txpin = &PD5_As_Uart2Tx_DefaultConfig,
		.dmarx = &DMA_Uart2Rx_DefaultConfig,
		.dmarx_irq = &IRQ_Uart2RxDMA_DefaultConfig,
		.dmatx = &DMA_Uart2Tx_DefaultConfig,
		.dmatx_irq = &IRQ_Uart2TxDMA_DefaultConfig,
		.uart_irq = &IRQ_Uart2_DefaultConfig,
		.uartex_ops = &UARTEX_Ops_DefaultConfig,
	};
	
	Board_ConfigTypeDef brd = {
		.uart2 = &uart2_cfg,
	};		
	
	struct msp_factory msp = {
		.gpio_clk = &GPIO_ClockProvider,
		.dma_clk = &DMA_ClockProvider,
		.irq_registry = &IRQ_HandlerObjectRegistry,
		
		.board_config = &brd,
	};
	
	h = msp_create_huartex(&msp, 2);
		
	TEST_ASSERT_NOT_NULL(h);
	
	TEST_ASSERT_NOT_NULL(h->hdmaex_rx);
	TEST_ASSERT_EQUAL_HEX32(uart2_cfg.dmarx->Instance, h->hdmaex_rx->hdma.Instance);
	TEST_ASSERT_EQUAL(uart2_cfg.dmarx_irq->irqn, h->hdmaex_rx->hirq->irqn);
	
	TEST_ASSERT_NOT_NULL(h->hdmaex_tx);
	if (h->hdmaex_tx)
	{
		TEST_ASSERT_EQUAL_HEX32(uart2_cfg.dmatx->Instance, h->hdmaex_tx->hdma.Instance);
		TEST_ASSERT_EQUAL(uart2_cfg.dmatx_irq->irqn, h->hdmaex_tx->hirq->irqn);
	}
	
	TEST_ASSERT_NOT_NULL(h->rxpin);
	if (h->rxpin)
	{
		TEST_ASSERT_EQUAL(uart2_cfg.rxpin->instance, h->rxpin->instance);
		TEST_ASSERT_EQUAL(uart2_cfg.rxpin->init.Pin, h->rxpin->init.Pin);
	}
	
	TEST_ASSERT_NOT_NULL(h->txpin);
	if (h->txpin)
	{
		TEST_ASSERT_EQUAL(uart2_cfg.txpin->instance, h->txpin->instance);
		TEST_ASSERT_EQUAL(uart2_cfg.txpin->init.Pin, h->txpin->init.Pin);
	}
	
	TEST_ASSERT_NOT_NULL(h->hirq);
	if (h->hirq)
	{
		TEST_ASSERT_EQUAL(uart2_cfg.uart_irq->irqn, h->hirq->irqn);
	}

	TEST_ASSERT_EQUAL_MEMORY(&UARTEX_Ops_DefaultConfig, &h->ops, sizeof(h->ops));
	
	TEST_ASSERT_EQUAL(uart2_cfg.uart->Instance, h->huart.Instance);

	DMAEX_Handle_FactoryDestroy(h->hdmaex_rx);
	DMAEX_Handle_FactoryDestroy(h->hdmaex_tx);
	free(h->rxpin);
	free(h->txpin);
	IRQ_Handle_Dtor(h->hirq);
	
	free(h);

}


TEST(MSP, CreateUARTEXHandle)
{
	UARTEX_ConfigTypeDef uart_cfg;
	Board_ConfigTypeDef board = { .uart2 = &uart_cfg, };
	struct msp_factory msp = { .board_config = &board, .ll_huartex_create = mock_ll_huartex_create, };
	
	passby = &msp;
	
	UARTEX_HandleTypeDef* h = MSP_Create_UARTEX_Handle(&msp, 2);
	TEST_ASSERT_EQUAL_HEX32(0xDEADBEEF, h);
	
	passby = 0;
}

TEST(MSP, DestroyUARTEXHandle)
{
	
}

TEST_GROUP_RUNNER(MSP)
{
	RUN_TEST_CASE(MSP, CreateUARTEXHandle);
	RUN_TEST_CASE(MSP, CreateHuartEx);
}



