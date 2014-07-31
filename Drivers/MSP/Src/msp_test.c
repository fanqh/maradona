#include "msp.h"

static void* passby = 0;

TEST_GROUP(MSP);

TEST_SETUP(MSP)
{}
	
TEST_TEAR_DOWN(MSP)
{}
	
UARTEX_HandleTypeDef* mock_ll_huartex_create(GPIO_ClockProviderTypeDef* 	gpio_clk,
																					DMA_ClockProviderTypeDef*		dma_clk,
																					IRQ_HandleRegistryTypeDef*	irq_registry,
																					const UARTEX_ConfigTypeDef* uartex_configs)
{
	MSP_TypeDef* msp = (MSP_TypeDef*)passby;
	assert_param(msp);
	
	TEST_ASSERT_EQUAL_HEX32(msp->gpio_clk, gpio_clk);
	TEST_ASSERT_EQUAL_HEX32(msp->dma_clk, dma_clk);
	TEST_ASSERT_EQUAL_HEX32(msp->irq_registry, irq_registry);
	TEST_ASSERT_EQUAL_HEX32(msp->board_config->uart2, uartex_configs);
	
	return (UARTEX_HandleTypeDef*)(0xDEADBEEF);
};
	
TEST(MSP, CreateUARTEX2Handle)
{
	UARTEX_ConfigTypeDef uart_cfg;
	Board_ConfigTypeDef board = { .uart2 = &uart_cfg, };
	MSP_TypeDef msp = { .board_config = &board, .ll_huartex_create = mock_ll_huartex_create, };
	
	passby = &msp;
	
	UARTEX_HandleTypeDef* h = MSP_Create_UART2EX_Handle(&msp);
	TEST_ASSERT_EQUAL_HEX32(0xDEADBEEF, h);
	
	passby = 0;
}

TEST_GROUP_RUNNER(MSP)
{
	RUN_TEST_CASE(MSP, CreateUARTEX2Handle);
}



