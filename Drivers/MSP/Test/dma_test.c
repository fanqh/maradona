#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "errno_ex.h"
#include "stm32f4xx_hal.h"
#include "dma.h"

/******************************************************************************

analyze the problem

OR-ing Switch Model

1. There should be 8 switches for either DMA1 or DMA2;
2. For either DMA, if 1 or more switch turn on, the clock should be on.
3. For eiterh DMA, if no switch turned on, the clock should be off.

Then Test Plan:
1. clock off, turn A on, clock on.
2. clock on, switch A on, turn A off, clock off.
3. clock on, switch A on, switch B on, turn A off, clock on.

******************************************************************************/

static DMA_ClockProviderTypeDef dma_clk = {0,0};

static bool dma1_clock_enabled(void)
{
	return (RCC->AHB1ENR & RCC_AHB1ENR_DMA1EN) ? true : false;
}

TEST_GROUP(DMA_Clock);

TEST_SETUP(DMA_Clock)
{
	memset(&dma_clk, 0, sizeof(dma_clk));
	__DMA1_CLK_DISABLE();
	__DMA2_CLK_DISABLE();
}

TEST_TEAR_DOWN(DMA_Clock)
{
	__DMA1_CLK_ENABLE();
	__DMA2_CLK_ENABLE();
}

/******************************************************************************
#define __DMA1_CLK_ENABLE()          (RCC->AHB1ENR |= (RCC_AHB1ENR_DMA1EN))
#define __DMA2_CLK_ENABLE()          (RCC->AHB1ENR |= (RCC_AHB1ENR_DMA2EN))
******************************************************************************/

TEST(DMA_Clock, ClockOffTurnOneOnClockOn)
{
	DMA_Clock_Get(&dma_clk, DMA1_Stream5);
	TEST_ASSERT_TRUE(dma1_clock_enabled());
}

TEST(DMA_Clock, ClockOffTurnOneOnBitSet)
{	
	DMA_Clock_Get(&dma_clk, DMA1_Stream5);
	TEST_ASSERT_TRUE(DMA_Clock_Status(&dma_clk, DMA1_Stream5));
}

TEST(DMA_Clock, ClockOnTurnOffClockOff)
{
	DMA_Clock_Get(&dma_clk, DMA1_Stream5);
	DMA_Clock_Put(&dma_clk, DMA1_Stream5);
	TEST_ASSERT_FALSE(dma1_clock_enabled());
}

TEST(DMA_Clock, ClockOnTurnOffBitClear)
{
	DMA_Clock_Get(&dma_clk, DMA1_Stream5);
	DMA_Clock_Put(&dma_clk, DMA1_Stream5);
	TEST_ASSERT_FALSE(DMA_Clock_Status(&dma_clk, DMA1_Stream5));
}

TEST_GROUP_RUNNER(DMA_Clock)
{
	RUN_TEST_CASE(DMA_Clock, ClockOffTurnOneOnClockOn);
	RUN_TEST_CASE(DMA_Clock, ClockOffTurnOneOnBitSet);
	RUN_TEST_CASE(DMA_Clock, ClockOnTurnOffClockOff);
	RUN_TEST_CASE(DMA_Clock, ClockOnTurnOffBitClear);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////



TEST_GROUP(DMAEX_Handle);
TEST_SETUP(DMAEX_Handle)
{
}

TEST_TEAR_DOWN(DMAEX_Handle)
{	
}

/** int DMAEX_Handle_Init(DMAEX_HandleTypeDef* h, DMA_Stream_TypeDef *stream, const DMA_InitTypeDef *init,
	DMA_ClockProviderTypeDef *clk, IRQ_HandleTypeDef *hirq); **/
TEST(DMAEX_Handle, HandleInitInvalidArgs)
{
	int ret;
	
	DMAEX_HandleTypeDef hdmaex;
	DMA_ClockProviderTypeDef clk;
	IRQ_HandleTypeDef	irq;
	const DMA_ConfigTypeDef * config = &DMA_Uart2Rx_DefaultConfig;
	
	ret = DMAEX_Handle_Init(NULL, config->Instance, &config->Init, &clk, &irq);
	TEST_ASSERT_EQUAL(-EINVAL, ret);
	
	ret = DMAEX_Handle_Init(&hdmaex, (DMA_Stream_TypeDef*)0xDEADBEEF, &config->Init, &clk, &irq);
	TEST_ASSERT_EQUAL(-EINVAL, ret);
	
	ret = DMAEX_Handle_Init(&hdmaex, config->Instance, NULL, &clk, &irq);
	TEST_ASSERT_EQUAL(-EINVAL, ret);
	
	ret = DMAEX_Handle_Init(&hdmaex, config->Instance, &config->Init, NULL, &irq);
	TEST_ASSERT_EQUAL(-EINVAL, ret);
	
	ret = DMAEX_Handle_Init(&hdmaex, config->Instance, &config->Init, &clk, NULL);
	TEST_ASSERT_EQUAL(-EINVAL, ret);
	
}

TEST(DMAEX_Handle, HandleInitSuccess)
{
	int ret;
	
	DMAEX_HandleTypeDef h;
	DMA_ClockProviderTypeDef clk;
	IRQ_HandleTypeDef	irq;
	const DMA_ConfigTypeDef * config = &DMA_Uart2Rx_DefaultConfig;
	
	ret = DMAEX_Handle_Init(&h, config->Instance, &config->Init, &clk, &irq);	

	TEST_ASSERT_EQUAL(0, ret);
	TEST_ASSERT_EQUAL_HEX32(&clk, h.clk);
	TEST_ASSERT_EQUAL_HEX32(config->Instance, h.hdma.Instance);
	TEST_ASSERT_EQUAL_MEMORY(&config->Init, &h.hdma.Init, sizeof(h.hdma.Init));
	TEST_ASSERT_EQUAL_HEX32(&irq, h.hirq);
	TEST_ASSERT_EQUAL(DMAEX_HANDLE_STATE_RESET, h.state);
	
	// all other field should be zero
	TEST_ASSERT_EQUAL(0, h.hdma.ErrorCode);
	TEST_ASSERT_EQUAL(0, h.hdma.Lock);
	TEST_ASSERT_EQUAL(0, h.hdma.Parent);
	TEST_ASSERT_EQUAL(0, h.hdma.State);
	TEST_ASSERT_EQUAL(0, h.hdma.XferCpltCallback);
	TEST_ASSERT_EQUAL(0, h.hdma.XferErrorCallback);
	TEST_ASSERT_EQUAL(0, h.hdma.XferHalfCpltCallback);
	TEST_ASSERT_EQUAL(0, h.hdma.XferM1CpltCallback);
}

/**
int DMAEX_Handle_InitByConfig(DMAEX_HandleTypeDef* h, const DMA_ConfigTypeDef* config, 
	DMA_ClockProviderTypeDef *clk, IRQ_HandleTypeDef *hirq); **/
TEST(DMAEX_Handle, HandleInitByConfigInvalidArgs)
{
	int ret;
	
	DMAEX_HandleTypeDef h;
	DMA_ClockProviderTypeDef clk;
	IRQ_HandleTypeDef	irq;
	
	ret = DMAEX_Handle_InitByConfig(&h, NULL, &clk, &irq);
	
	TEST_ASSERT_EQUAL(-EINVAL, ret);
}

TEST(DMAEX_Handle, HandleInitByConfigSuccess)
{
	int ret;
	
	DMAEX_HandleTypeDef h;
	DMA_ClockProviderTypeDef clk;
	IRQ_HandleTypeDef	irq;
	const DMA_ConfigTypeDef * config = &DMA_Uart2Rx_DefaultConfig;
	
	ret = DMAEX_Handle_InitByConfig(&h, config, &clk, &irq);	

	TEST_ASSERT_EQUAL(0, ret);
	TEST_ASSERT_EQUAL_HEX32(&clk, h.clk);
	TEST_ASSERT_EQUAL_HEX32(config->Instance, h.hdma.Instance);
	TEST_ASSERT_EQUAL_MEMORY(&config->Init, &h.hdma.Init, sizeof(h.hdma.Init));
	TEST_ASSERT_EQUAL_HEX32(&irq, h.hirq);
	TEST_ASSERT_EQUAL(DMAEX_HANDLE_STATE_RESET, h.state);
	
	// all other field should be zero
	TEST_ASSERT_EQUAL(0, h.hdma.ErrorCode);
	TEST_ASSERT_EQUAL(0, h.hdma.Lock);
	TEST_ASSERT_EQUAL(0, h.hdma.Parent);
	TEST_ASSERT_EQUAL(0, h.hdma.State);
	TEST_ASSERT_EQUAL(0, h.hdma.XferCpltCallback);
	TEST_ASSERT_EQUAL(0, h.hdma.XferErrorCallback);
	TEST_ASSERT_EQUAL(0, h.hdma.XferHalfCpltCallback);
	TEST_ASSERT_EQUAL(0, h.hdma.XferM1CpltCallback);
}

TEST(DMAEX_Handle, DMAEX_HAL_Init)
{
	DMAEX_HandleTypeDef hdmaex;
	IRQ_HandleTypeDef irq;
	
	const DMA_ConfigTypeDef * dmacfg = &DMA_Uart2Rx_DefaultConfig;
	const IRQ_ConfigTypeDef * irqcfg = &IRQ_Uart2RxDMA_DefaultConfig;
	DMA_ClockProviderTypeDef * clk_provider = &DMA_ClockProvider;
	IRQ_HandleRegistryTypeDef * registry = &IRQ_HandlerObjectRegistry;
	
	IRQ_Handle_InitByConfig(&irq, irqcfg, registry);
	DMAEX_Handle_InitByConfig(&hdmaex, dmacfg, clk_provider, &irq);
	
	DMAEX_HAL_Init(&hdmaex);
	
	TEST_ASSERT_TRUE(DMA_Clock_Status(hdmaex.clk, hdmaex.hdma.Instance));
	TEST_ASSERT_EQUAL(HAL_DMA_STATE_READY, hdmaex.hdma.State);
	TEST_ASSERT_EQUAL(&hdmaex.hdma, hdmaex.hirq->irqh_obj);
	TEST_ASSERT_EQUAL(IRQ_HANDLE_STATE_SET, hdmaex.hirq->state);
	TEST_ASSERT_EQUAL(DMAEX_HANDLE_STATE_SET, hdmaex.state);

}

TEST(DMAEX_Handle, DMAEX_HAL_DeInit)
{
	DMAEX_HandleTypeDef hdmaex;
	IRQ_HandleTypeDef irq;
	
	const DMA_ConfigTypeDef * dmacfg = &DMA_Uart2Rx_DefaultConfig;
	const IRQ_ConfigTypeDef * irqcfg = &IRQ_Uart2RxDMA_DefaultConfig;
	DMA_ClockProviderTypeDef * clk_provider = &DMA_ClockProvider;
	IRQ_HandleRegistryTypeDef * registry = &IRQ_HandlerObjectRegistry;
	
	IRQ_Handle_InitByConfig(&irq, irqcfg, registry);
	DMAEX_Handle_InitByConfig(&hdmaex, dmacfg, clk_provider, &irq);
	
	DMAEX_HAL_Init(&hdmaex);
	DMAEX_HAL_DeInit(&hdmaex);
	
	TEST_ASSERT_FALSE(DMA_Clock_Status(hdmaex.clk, hdmaex.hdma.Instance));
	TEST_ASSERT_EQUAL(HAL_DMA_STATE_RESET, hdmaex.hdma.State);
	TEST_ASSERT_NULL(hdmaex.hirq->irqh_obj);
	TEST_ASSERT_EQUAL(IRQ_HANDLE_STATE_RESET, hdmaex.hirq->state);
	TEST_ASSERT_EQUAL(DMAEX_HANDLE_STATE_RESET, hdmaex.state);
}

TEST_GROUP_RUNNER(DMAEX_Handle)
{
	RUN_TEST_CASE(DMAEX_Handle, HandleInitInvalidArgs);
	RUN_TEST_CASE(DMAEX_Handle, HandleInitSuccess);
	RUN_TEST_CASE(DMAEX_Handle, HandleInitByConfigInvalidArgs);
	RUN_TEST_CASE(DMAEX_Handle, HandleInitByConfigSuccess);
	
	RUN_TEST_CASE(DMAEX_Handle, DMAEX_HAL_Init);
	RUN_TEST_CASE(DMAEX_Handle, DMAEX_HAL_DeInit);
}

TEST_GROUP_RUNNER(DMA_All)
{
	RUN_TEST_GROUP(DMA_Clock);
	RUN_TEST_GROUP(DMAEX_Handle);
}


