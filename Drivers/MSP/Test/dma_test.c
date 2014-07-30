#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
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

//typedef struct {
//	
//	DMA_ClockProviderTypeDef							*clk;
//	IRQ_HandlerObjectRegistryTypeDef			*reg;
//	
//} DMAEX_Handle_FactoryTypeDef;

//DMAEX_HandleTypeDef*	DMAEX_Handle_FactoryCreate(DMAEX_Handle_FactoryTypeDef* factory, 
//																									const DMA_HandleTypeDef* hdma,
//																									const IRQ_HandleTypeDef* hirq);

//const DMA_InitTypeDef DMA_Init_Uart2Rx =
//{
//	.Channel = DMA_CHANNEL_4,
//	.Direction = DMA_PERIPH_TO_MEMORY,
//	.PeriphInc = DMA_PINC_DISABLE,
//	.MemInc = DMA_MINC_ENABLE,
//	.PeriphDataAlignment = DMA_PDATAALIGN_BYTE,
//	.MemDataAlignment = DMA_MDATAALIGN_BYTE,
//	.Mode = DMA_NORMAL,
//	.Priority = DMA_PRIORITY_LOW,
//	.FIFOMode = DMA_FIFOMODE_DISABLE,
//};

TEST_GROUP(DMAEX_Handle);
TEST_SETUP(DMAEX_Handle)
{
}

TEST_TEAR_DOWN(DMAEX_Handle)
{	
}

TEST(DMAEX_Handle, Ctor)
{
	const DMA_HandleTypeDef* dfl = &DMA_Handle_Uart2Rx_Default;
	DMA_ClockProviderTypeDef clk;
	IRQ_HandleTypeDef	irq;

	DMAEX_HandleTypeDef* h = DMAEX_Handle_Ctor(dfl->Instance, &dfl->Init, &clk, &irq);
	TEST_ASSERT_NOT_NULL(h);
	TEST_ASSERT_EQUAL_HEX32(&clk, h->clk);
	TEST_ASSERT_EQUAL_HEX32(dfl->Instance, h->hdma.Instance);
	TEST_ASSERT_EQUAL_MEMORY(&dfl->Init, &h->hdma.Init, sizeof(dfl->Init));
	TEST_ASSERT_EQUAL_HEX32(&irq, h->hirq);
	TEST_ASSERT_EQUAL(DMAEX_HANDLE_STATE_RESET, h->state);
	
	// all other field should be zero
	TEST_ASSERT_EQUAL(0, h->hdma.ErrorCode);
	TEST_ASSERT_EQUAL(0, h->hdma.Lock);
	TEST_ASSERT_EQUAL(0, h->hdma.Parent);
	TEST_ASSERT_EQUAL(0, h->hdma.State);
	TEST_ASSERT_EQUAL(0, h->hdma.XferCpltCallback);
	TEST_ASSERT_EQUAL(0, h->hdma.XferErrorCallback);
	TEST_ASSERT_EQUAL(0, h->hdma.XferHalfCpltCallback);
	TEST_ASSERT_EQUAL(0, h->hdma.XferM1CpltCallback);
	
	if (h) free(h);
}

TEST(DMAEX_Handle, CtorByConfig)
{
	DMA_ClockProviderTypeDef clk;
	IRQ_HandleTypeDef	irq;

	DMAEX_HandleTypeDef* h = DMAEX_Handle_CtorByConfig(&DMAEX_Uart2Rx_DefaultConfig, &clk, &irq);
	TEST_ASSERT_NOT_NULL(h);
	TEST_ASSERT_EQUAL_HEX32(&clk, h->clk);
	TEST_ASSERT_EQUAL_HEX32(DMAEX_Uart2Rx_DefaultConfig.Instance, h->hdma.Instance);
	TEST_ASSERT_EQUAL_MEMORY(&DMAEX_Uart2Rx_DefaultConfig.Init, &h->hdma.Init, sizeof(DMAEX_Uart2Rx_DefaultConfig.Init));
	TEST_ASSERT_EQUAL_HEX32(&irq, h->hirq);
	TEST_ASSERT_EQUAL(DMAEX_HANDLE_STATE_RESET, h->state);
	
	// all other field should be zero
	TEST_ASSERT_EQUAL(0, h->hdma.ErrorCode);
	TEST_ASSERT_EQUAL(0, h->hdma.Lock);
	TEST_ASSERT_EQUAL(0, h->hdma.Parent);
	TEST_ASSERT_EQUAL(0, h->hdma.State);
	TEST_ASSERT_EQUAL(0, h->hdma.XferCpltCallback);
	TEST_ASSERT_EQUAL(0, h->hdma.XferErrorCallback);
	TEST_ASSERT_EQUAL(0, h->hdma.XferHalfCpltCallback);
	TEST_ASSERT_EQUAL(0, h->hdma.XferM1CpltCallback);
	
	if (h) free(h);
}

TEST(DMAEX_Handle, Dtor)
{
	const DMA_HandleTypeDef* dfl = &DMA_Handle_Uart2Rx_Default;
	DMA_ClockProviderTypeDef clk;
	IRQ_HandleTypeDef	irq;

	DMAEX_HandleTypeDef* h = DMAEX_Handle_Ctor(dfl->Instance, &dfl->Init, &clk, &irq);
	DMAEX_Handle_Dtor(h);
	
	// if dtor failed to recycle or overrun, unity will catch the bug.
}

TEST(DMAEX_Handle, FactoryCreate)
{
	DMA_ClockProviderTypeDef					clk;
	IRQ_HandlerObjectRegistryTypeDef	registry;
	DMAEX_Handle_FactoryTypeDef 			factory;
	
	DMA_HandleTypeDef									hdma;
//	IRQ_HandleTypeDef									hirq;
	IRQ_ConfigTypeDef									irq_config;

	DMAEX_HandleTypeDef*							h;
	
	memset(&hdma, 0xA5, sizeof(hdma));
	// memset(&hirq, 0xB5, sizeof(hirq));
	memset(&irq_config, 0xB5, sizeof(irq_config));
	
	factory.clk = &clk;
	factory.reg = &registry;
	
	//	hirq.state = IRQ_HANDLE_STATE_RESET;
	// irq_config.irqn = 0;
	
	// h = DMAEX_Handle_FactoryCreate(&factory, &hdma, &hirq);
	h = DMAEX_Handle_FactoryCreate(&factory, &hdma, &irq_config);
	
	TEST_ASSERT_NOT_NULL(h);
	TEST_ASSERT_EQUAL_HEX32(factory.clk, h->clk);
	
	TEST_ASSERT_EQUAL_HEX32(hdma.Instance, h->hdma.Instance);
	TEST_ASSERT_EQUAL_MEMORY(&hdma.Init, &h->hdma.Init, sizeof(DMA_InitTypeDef));

	TEST_ASSERT_EQUAL(irq_config.irqn, h->hirq->irqn);
	TEST_ASSERT_EQUAL(irq_config.preempt_priority, h->hirq->preempt_priority);
	TEST_ASSERT_EQUAL(irq_config.sub_priority, h->hirq->sub_priority);
	TEST_ASSERT_EQUAL(factory.reg, h->hirq->registry);
	TEST_ASSERT_EQUAL(IRQ_HANDLE_STATE_RESET, h->hirq->state);
	
	TEST_ASSERT_EQUAL(DMAEX_HANDLE_STATE_RESET, h->state);
	
	if (h->hirq) free(h->hirq);
	if (h) free(h);
}

TEST(DMAEX_Handle, FactoryDestroy)
{
	DMA_ClockProviderTypeDef					clk;
	IRQ_HandlerObjectRegistryTypeDef	registry;
	DMAEX_Handle_FactoryTypeDef 			factory;
	
	DMA_HandleTypeDef									hdma;
	IRQ_HandleTypeDef									hirq;
	IRQ_ConfigTypeDef									irq_config;

	DMAEX_HandleTypeDef*							h;
	
	memset(&hdma, 0xA5, sizeof(hdma));
	memset(&hirq, 0xB5, sizeof(hirq));
	
	factory.clk = &clk;
	factory.reg = &registry;
	
	hirq.state = IRQ_HANDLE_STATE_RESET;
	
	// h = DMAEX_Handle_FactoryCreate(&factory, &hdma, &hirq);
	h = DMAEX_Handle_FactoryCreate(&factory, &hdma, &irq_config);
	DMAEX_Handle_FactoryDestroy(&factory, h);
}



TEST(DMAEX_Handle, DMAEX_Init)
{
	IRQ_HandleTypeDef* irq = IRQ_Handle_Ctor(DMA1_Stream5_IRQn, 0, 0, &IRQ_HandlerObjectRegistry);
	DMAEX_HandleTypeDef* hdmaex = DMAEX_Handle_Ctor(DMAEX_Handle_Uart2Rx_Default.hdma.Instance, &DMAEX_Handle_Uart2Rx_Default.hdma.Init, &DMA_ClockProvider, irq);
	
	DMAEX_Init(hdmaex);
	
	TEST_ASSERT_TRUE(DMA_Clock_Status(hdmaex->clk, hdmaex->hdma.Instance));
	TEST_ASSERT_EQUAL(HAL_DMA_STATE_READY, hdmaex->hdma.State);
	TEST_ASSERT_EQUAL(&hdmaex->hdma, hdmaex->hirq->irqh_obj);
	TEST_ASSERT_EQUAL(IRQ_HANDLE_STATE_SET, hdmaex->hirq->state);
	TEST_ASSERT_EQUAL(DMAEX_HANDLE_STATE_SET, hdmaex->state);
	
	DMAEX_Handle_Dtor(hdmaex);
	IRQ_Handle_Dtor(irq);
}

TEST(DMAEX_Handle, DMAEX_DeInit)
{
	IRQ_HandleTypeDef* irq = IRQ_Handle_Ctor(DMA1_Stream5_IRQn, 0, 0, &IRQ_HandlerObjectRegistry);
	DMAEX_HandleTypeDef* hdmaex = DMAEX_Handle_Ctor(DMAEX_Handle_Uart2Rx_Default.hdma.Instance, &DMAEX_Handle_Uart2Rx_Default.hdma.Init, &DMA_ClockProvider, irq);
	
	DMAEX_Init(hdmaex);
	DMAEX_DeInit(hdmaex);
	
	TEST_ASSERT_FALSE(DMA_Clock_Status(hdmaex->clk, hdmaex->hdma.Instance));
	TEST_ASSERT_EQUAL(HAL_DMA_STATE_RESET, hdmaex->hdma.State);
	TEST_ASSERT_NULL(hdmaex->hirq->irqh_obj);
	TEST_ASSERT_EQUAL(IRQ_HANDLE_STATE_RESET, hdmaex->hirq->state);
	TEST_ASSERT_EQUAL(DMAEX_HANDLE_STATE_RESET, hdmaex->state);
	
	DMAEX_Handle_Dtor(hdmaex);
	IRQ_Handle_Dtor(irq);
}

TEST_GROUP_RUNNER(DMAEX_Handle)
{
	RUN_TEST_CASE(DMAEX_Handle, Ctor);	
	RUN_TEST_CASE(DMAEX_Handle, CtorByConfig);
	RUN_TEST_CASE(DMAEX_Handle, Dtor);
	RUN_TEST_CASE(DMAEX_Handle, FactoryCreate);
	RUN_TEST_CASE(DMAEX_Handle, FactoryDestroy);
	RUN_TEST_CASE(DMAEX_Handle, DMAEX_Init);
	RUN_TEST_CASE(DMAEX_Handle, DMAEX_DeInit);
}

TEST_GROUP_RUNNER(DMA_All)
{
	RUN_TEST_GROUP(DMA_Clock);
	RUN_TEST_GROUP(DMAEX_Handle);
}


