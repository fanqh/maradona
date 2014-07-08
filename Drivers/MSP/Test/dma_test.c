#include <stdbool.h>
#include <string.h>
#include "stm32f4xx_hal.h"
#include "dma.h"
#include "unity_fixture.h"

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

static DMA_ClockTypeDef dma_clock = {0,0};

static bool dma1_clock_enabled(void)
{
	return (RCC->AHB1ENR & RCC_AHB1ENR_DMA1EN) ? true : false;
}

TEST_GROUP(DMA_Clock);

TEST_SETUP(DMA_Clock)
{
	memset(&dma_clock, 0, sizeof(dma_clock));
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
	// assume all clocks off.
	DMA_Clock_Get(&dma_clock, DMA1_Stream5);
	TEST_ASSERT_TRUE(dma1_clock_enabled());
}

TEST(DMA_Clock, ClockOffTurnOneOnBitSet)
{	
	DMA_Clock_Get(&dma_clock, DMA1_Stream5);
	TEST_ASSERT_TRUE(DMA_Clock_Status(&dma_clock, DMA1_Stream5));
}

TEST(DMA_Clock, ClockOnTurnOffClockOff)
{
	DMA_Clock_Get(&dma_clock, DMA1_Stream5);
	DMA_Clock_Put(&dma_clock, DMA1_Stream5);
	TEST_ASSERT_FALSE(dma1_clock_enabled());
}

TEST(DMA_Clock, ClockOnTurnOffBitClear)
{
	DMA_Clock_Get(&dma_clock, DMA1_Stream5);
	DMA_Clock_Put(&dma_clock, DMA1_Stream5);
	TEST_ASSERT_FALSE(DMA_Clock_Status(&dma_clock, DMA1_Stream5));
}

TEST_GROUP_RUNNER(DMA_Clock)
{
	RUN_TEST_CASE(DMA_Clock, ClockOffTurnOneOnClockOn);
	RUN_TEST_CASE(DMA_Clock, ClockOffTurnOneOnBitSet);
	RUN_TEST_CASE(DMA_Clock, ClockOnTurnOffClockOff);
	RUN_TEST_CASE(DMA_Clock, ClockOnTurnOffBitClear);
}

TEST_GROUP(DMAEX_Init);

static DMAEX_HandleTypeDef hdmaex;

#define DMAEX	(&hdmaex)

TEST_SETUP(DMAEX_Init)
{
	hdmaex = DMAEX_Handle_Uart2Rx_Default;
	IRQ_DeInit(&hdmaex.hirq);
	DMAEX_DeInit(&hdmaex);
	DMA_Clock_Put(hdmaex.clk, hdmaex.hdma.Instance);	
}

TEST_TEAR_DOWN(DMAEX_Init)
{
	hdmaex = DMAEX_Handle_Uart2Rx_Default;
	IRQ_DeInit(&hdmaex.hirq);
	DMAEX_DeInit(&hdmaex);
	DMA_Clock_Put(hdmaex.clk, hdmaex.hdma.Instance);		
}

TEST(DMAEX_Init, DMAClockBitShouldBeSet)
{
	DMAEX_Init(DMAEX);
	TEST_ASSERT_TRUE(DMA_Clock_Status(DMAEX->clk, DMAEX->hdma.Instance));
}

TEST(DMAEX_Init, DMAHALShouldBeInitialized)
{
	DMAEX_Init(DMAEX);
	TEST_ASSERT_EQUAL(HAL_DMA_STATE_READY, DMAEX->hdma.State);
}

TEST(DMAEX_Init, DMAHandleRegisteredForIRQHandler)
{
	DMAEX_Init(DMAEX);
	TEST_ASSERT_EQUAL(&DMAEX->hdma, DMAEX->hirq.hdata);
}

TEST(DMAEX_Init, DMAIRQShouldBeInitialized)
{
	DMAEX_Init(DMAEX);
	TEST_ASSERT_EQUAL(IRQ_HANDLE_STATE_SET, DMAEX->hirq.state);
}

TEST(DMAEX_Init, DMAEXHandleStateShouldBeSet)
{
	DMAEX_Init(DMAEX);
	TEST_ASSERT_EQUAL(DMAEX_HANDLE_STATE_SET, DMAEX->state);
}

TEST(DMAEX_Init, DMAEX_DeInit)
{
	DMAEX_Init(DMAEX);
	DMAEX_DeInit(DMAEX);
	
	TEST_ASSERT_FALSE(DMA_Clock_Status(DMAEX->clk, DMAEX->hdma.Instance));
	TEST_ASSERT_EQUAL(HAL_DMA_STATE_RESET, DMAEX->hdma.State);
	TEST_ASSERT_NULL(DMAEX->hirq.hdata);
	TEST_ASSERT_EQUAL(IRQ_HANDLE_STATE_RESET, DMAEX->hirq.state);
	TEST_ASSERT_EQUAL(DMAEX_HANDLE_STATE_RESET, DMAEX->state);
}

TEST_GROUP_RUNNER(DMAEX_Init)
{
	RUN_TEST_CASE(DMAEX_Init, DMAClockBitShouldBeSet);
	RUN_TEST_CASE(DMAEX_Init, DMAHALShouldBeInitialized);
	RUN_TEST_CASE(DMAEX_Init, DMAHandleRegisteredForIRQHandler);
	RUN_TEST_CASE(DMAEX_Init, DMAIRQShouldBeInitialized);
	RUN_TEST_CASE(DMAEX_Init, DMAEXHandleStateShouldBeSet);
	RUN_TEST_CASE(DMAEX_Init, DMAEX_DeInit);
}



