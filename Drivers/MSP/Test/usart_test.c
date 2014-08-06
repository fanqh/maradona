#include <string.h>
#include "errno_ex.h"
#include "stm32f4xx_hal.h"
#include "irq.h"
#include "dma.h"
#include "msp.h"
#include "usart.h"
#include "usart_test.h"

static UARTEX_HandleTypeDef* huartex_dma;
static UARTEX_HandleTypeDef* huartex;

#define HUARTEX_DMA 	(huartex_dma)
#define HUART_DMA			(&huartex_dma->huart)

#define HUARTEX				(huartex)
#define HUART					(&huartex->huart)

const char alphabet[] = "abcdefghijklmnopqrstuvwxyz";

///////////////////////////////////////////////////////////////////////////////
/** Analysis

1. what happens in initialization and what is the 'states' to be asserted?

IP Blocks

PORT: Clock, Config; No IRQ, No Enable
DMA: Clock, Config; Enable used but not in init; IRQ Handler, Config & Enable
UART: Clock, Config, Enable; IRQ Handler, Config & Enable

HAL Layers

UART: State, Error
DMA: State, Error

Initialization Sequence:
		Actions											Consequence								
		MX_GPIO_Init() 						
[x]		...												PORT Clock Enabled 
		MX_DMA_Init()							
[x]		...												DMA Clock Enabled
[x]		...												DMA IRQ Config & Enabled;
		MX_USART2_UART_Init() 
			HAL_UART_Init() 	
				HAL_UART_MspInit()			
[x]				...										UART Clock Enabled
[x]				HAL_GPIO_Init()				PORT(GPIO) Config
[x]				HAL_DMA_Init()				rx DMA Config, dma State -> READY
[x]				...										uart_rx/dma mutual reference
[x]				HAL_DMA_Init()				tx DMA Config, dma State -> READY
[x]				...										uart_tx/dma mutual reference
[x]				...										UART IRQ Config & Enabled
[x]			UART_SetConfig()				UART Config
[x]			...											uart State -> READY
[x]			...											UART Enabled

Init assertion:									reverted in DeInit?				potential power problem?
	port clock enabled						no												yes
	dma clock enabled							no												yes
	dma irq config & enabled			no												???
	dma handler exist							no												???
	uart clock enabled						yes												yes
	port configured	(not reset)		yes
	dma state -> READY 	x 2				yes
	rx/dma linked				x	2				no
	uart irq config & enabled			yes (config not cleared)
	uart configured								no
	uart state->READY							yes (config not cleared)
	uart enabled									no!
	
*******************************************************************************

Deinitialization Sequence:

		HAL_UART_DeInit()
			HAL_UART_MspDeInit()
[x]			...											UART Clock Disabled.
[x]			HAL_GPIO_DeInit()				PORT GPIO reset
				HAL_DMA_DeInit()				rxdma
[x]				...										Disable DMA, clear config/flags
[x]				...										dma State -> RESET
				HAL_DMA_DeInit()				txdma
[x]				...										Disable DMA, clear config/flags
[x]				...										dma State -> RESET
[x]			...											UART IRQ disabled
[x]		...												UART state -> RESET

Deinit Assertion:

		uart clock disabled.
		port gpio reset.
		rxdma State -> RESET
		txdma State -> RESET
		UART IRQ -> Disabled
		UART State -> RESET
	
******************************************************************************/	

///////////////////////////////////////////////////////////////////////////////
// internal functions (most are obsolete)

bool gpio_modes_all_input(GPIO_TypeDef* gpiox, uint32_t pins)
{
	uint32_t pos;
	for (pos = 0; pos < 16; pos++)
	{
		if (((uint32_t)1 << pos) & pins)
		{
			if ((gpiox->MODER & (GPIO_MODER_MODER0 << (pos * 2))) != 0)
				return false;
		}
	}
	return true;
}

bool gpio_modes_all_noninput(GPIO_TypeDef* gpiox, uint32_t pins)
{
	uint32_t pos;
	for (pos = 0; pos < 16; pos++)
	{
		if (((uint32_t)1 << pos) & pins)
		{
			if ((gpiox->MODER & (GPIO_MODER_MODER0 << (pos * 2))) == 0)
				return false;
		}
	}
	return true;
}


bool dma1_clock_enabled(void) 
{	
// #define __DMA1_CLK_ENABLE()          (RCC->AHB1ENR |= (RCC_AHB1ENR_DMA1EN))
// #define __DMA2_CLK_ENABLE()          (RCC->AHB1ENR |= (RCC_AHB1ENR_DMA2EN))	
	return (RCC->AHB1ENR & RCC_AHB1ENR_DMA1EN) ? true : false;
}

bool dma2_clock_enabled(void) 
{
	return (RCC->AHB1ENR & RCC_AHB1ENR_DMA2EN) ? true : false;
}

bool irq_enabled(IRQn_Type IRQn) 
{
	uint32_t reg;
	
	/** or NVIC->ISER[(uint32_t)((int32_t)IRQn) >> 5]; **/
	reg = NVIC->ISER[(uint32_t)((int32_t)IRQn) >> 5];
	return (reg & (1 << ((uint32_t)(IRQn) & 0x1F))) ? true : false;
}


bool uart_mode_configured(UART_HandleTypeDef* huart)
{
	if (huart->Init.Mode == 0) return false;
	return ((huart->Instance->CR1 & (USART_CR1_TE | USART_CR1_RE)) == huart->Init.Mode) ? true : false;
}

bool uart_mode_not_configured(UART_HandleTypeDef* huart)
{
	return ((huart->Instance->CR1 & (USART_CR1_TE | USART_CR1_RE)) == 0) ? true : false;
}

bool uart_enabled(UART_HandleTypeDef* huart)
{
	/** ((__HANDLE__)->Instance->CR1 |=  USART_CR1_UE) **/	
	return (huart->Instance->CR1 & USART_CR1_UE) ? true : false;
}

bool __uart_dmat_enabled(UART_HandleTypeDef* huart)
{
	return (huart->Instance->CR3 & USART_CR3_DMAT) ? true : false;
}

bool __uart_dmar_enabled(UART_HandleTypeDef* huart)
{
	return (huart->Instance->CR3 & USART_CR3_DMAR) ? true : false;
}

bool __dma_sxcr_enabled(DMA_HandleTypeDef* hdma)
{
// #define __HAL_DMA_ENABLE(__HANDLE__)      ((__HANDLE__)->Instance->CR |=  DMA_SxCR_EN)
	return (hdma->Instance->CR & DMA_SxCR_EN) ? true : false;
}

bool uart_txdma_linked(UART_HandleTypeDef* huart, DMA_HandleTypeDef* tx)
{
	if (huart == 0) return false;
	if (tx == 0) return false;
	
	if (huart->hdmatx == 0) return false;
	if (huart->hdmatx != tx) return false;
	if (huart->hdmatx->Parent == 0) return false;
	if (huart->hdmatx->Parent != huart) return false;
		
	return true;
}

bool uart_rxdma_linked(UART_HandleTypeDef* huart, DMA_HandleTypeDef* rx)
{
	if (huart == 0) return false;
	if (rx == 0) return false;
	
	if (huart->hdmarx == 0) return false;
	if (huart->hdmarx != rx) return false;
	if (huart->hdmarx->Parent == 0) return false;
	if (huart->hdmarx->Parent != huart) return false;
	
	return true;
}




///////////////////////////////////////////////////////////////////////////////
// create and destroy test data
//
static void CreateTestData(void)
{

	UARTEX_ConfigTypeDef	cfg =
	{
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
	
	UARTEX_ConfigTypeDef	cfg_nodma_noirq =
	{
		.uart = &UART2_DefaultConfig,
		.rxpin = &PD6_As_Uart2Rx_DefaultConfig,
		.txpin = &PD5_As_Uart2Tx_DefaultConfig,
		.uartex_ops = &UARTEX_Ops_DefaultConfig,
	};	
	
//	UARTEX_ConfigTypeDef	cfg =
//	{
//		.uart = &UART2_DefaultConfig,
//		.rxpin = &PD6_As_Uart2Rx_DefaultConfig,
//		.txpin = &PD5_As_Uart2Tx_DefaultConfig,
//		.dmarx = &DMA_Uart2Rx_DefaultConfig,
//		.dmarx_irq = &IRQ_Uart2RxDMA_DefaultConfig,
//		.dmatx = &DMA_Uart2Tx_DefaultConfig,
//		.dmatx_irq = &IRQ_Uart2TxDMA_DefaultConfig,
//		.uart_irq = &IRQ_Uart2_DefaultConfig,
//		.uartex_ops = &UARTEX_Ops_DefaultConfig,
//	};	
	
	struct msp_factory msp = {
		
		.gpio_clk = &GPIO_ClockProvider,
		.dma_clk = &DMA_ClockProvider,
		.irq_registry = &IRQ_HandlerObjectRegistry,
		
		.create_dmaex_handle = msp_create_dmaex_handle,
		.gpioex_init_by_config = GPIOEX_InitByConfig,		
		.irq_handle_init_by_config = IRQ_Handle_InitByConfig,
		.uartex_handle_init_by_config = UARTEX_Handle_InitByConfig,
	};
	
	HUARTEX_DMA = msp_create_uartex_handle(&msp, &cfg);		
	HUARTEX = msp_create_uartex_handle(&msp, &cfg_nodma_noirq);
	
	assert_param(HUARTEX_DMA);
	assert_param(HUARTEX);
}

static void DestroyTestData(void)
{	
	UARTEX_Handle_FactoryDestroy(huartex_dma);
	UARTEX_Handle_FactoryDestroy(huartex);	
}

///////////////////////////////////////////////////////////////////////////////
// UARTEX_Handle Tests Ctor, Dtor, Factories
//
TEST_GROUP(UARTEX_Handle);
TEST_SETUP(UARTEX_Handle){}
TEST_TEAR_DOWN(UARTEX_Handle){}
	
TEST(UARTEX_Handle, Ctor)
{
	UARTEX_HandleTypeDef* h;
	GPIOEX_TypeDef rx = PD6_As_Uart2Rx_Default;
	GPIOEX_TypeDef tx = PD5_As_Uart2Tx_Default;
	DMAEX_HandleTypeDef hdmaex_rx;
	DMAEX_HandleTypeDef	hdmaex_tx;
	IRQ_HandleTypeDef	hirq;
	struct UARTEX_Operations ops;
	
	memset(&ops, 0xA5, sizeof(ops));
	
	h = UARTEX_Handle_Ctor(USART2, &UART_Handle_Uart2_Default.Init, &rx, &tx, &hdmaex_rx, &hdmaex_tx, &hirq, &ops);
	
	TEST_ASSERT_NOT_NULL(h);
	TEST_ASSERT_EQUAL_HEX32(&rx, h->rxpin);
	TEST_ASSERT_EQUAL_HEX32(&tx, h->txpin);
	TEST_ASSERT_EQUAL(USART2, h->huart.Instance);
	TEST_ASSERT_EQUAL_MEMORY(&UART_Handle_Uart2_Default.Init, &h->huart.Init, sizeof(UART_InitTypeDef));
	TEST_ASSERT_EQUAL_HEX32(&hdmaex_rx, h->hdmaex_rx);
	TEST_ASSERT_EQUAL_HEX32(&hdmaex_tx, h->hdmaex_tx);
	TEST_ASSERT_EQUAL_HEX32(&hirq, h->hirq);
	TEST_ASSERT_EQUAL_MEMORY(&ops, &h->ops, sizeof(ops));
	TEST_ASSERT_EQUAL_HEX32(0, h->testdata);
	
	if (h) free(h);
}

TEST(UARTEX_Handle, HandleInitInvalidArgs)
{
  int ret;
	UARTEX_HandleTypeDef h;
	GPIOEX_TypeDef rx = PD6_As_Uart2Rx_Default;
	GPIOEX_TypeDef tx = PD5_As_Uart2Tx_Default;
	DMAEX_HandleTypeDef hdmaex_rx;
	DMAEX_HandleTypeDef	hdmaex_tx;
	IRQ_HandleTypeDef	hirq;
	struct UARTEX_Operations ops;
	
	memset(&ops, 0xA5, sizeof(ops));
	
	/** must have **/
	ret = UARTEX_Handle_Init(NULL, USART2, &UART_Handle_Uart2_Default.Init, &rx, &tx, &hdmaex_rx, &hdmaex_tx, &hirq, &ops);
	TEST_ASSERT_EQUAL(-EINVAL, ret);
	
	ret = UARTEX_Handle_Init(&h, (USART_TypeDef*)0xDEADBEEF, &UART_Handle_Uart2_Default.Init, &rx, &tx, &hdmaex_rx, &hdmaex_tx, &hirq, &ops);
	TEST_ASSERT_EQUAL(-EINVAL, ret);
	
	ret = UARTEX_Handle_Init(&h, USART2, NULL, &rx, &tx, &hdmaex_rx, &hdmaex_tx, &hirq, &ops);
	TEST_ASSERT_EQUAL(-EINVAL, ret);
	
	ret = UARTEX_Handle_Init(&h, USART2, &UART_Handle_Uart2_Default.Init, NULL, &tx, &hdmaex_rx, &hdmaex_tx, &hirq, &ops);
	TEST_ASSERT_EQUAL(-EINVAL, ret);
	
	ret = UARTEX_Handle_Init(&h, USART2, &UART_Handle_Uart2_Default.Init, &rx, NULL, &hdmaex_rx, &hdmaex_tx, &hirq, &ops);
	TEST_ASSERT_EQUAL(-EINVAL, ret);
	
	ret = UARTEX_Handle_Init(&h, USART2, &UART_Handle_Uart2_Default.Init, &rx, &tx, &hdmaex_rx, &hdmaex_tx, &hirq, NULL);
	TEST_ASSERT_EQUAL(-EINVAL, ret);
	
	/** all or none **/
	ret = UARTEX_Handle_Init(&h, USART2, &UART_Handle_Uart2_Default.Init, &rx, &tx, NULL, &hdmaex_tx, &hirq, &ops);
	TEST_ASSERT_EQUAL(-EINVAL, ret);
	
	ret = UARTEX_Handle_Init(&h, USART2, &UART_Handle_Uart2_Default.Init, &rx, &tx, &hdmaex_rx, NULL, &hirq, &ops);
	TEST_ASSERT_EQUAL(-EINVAL, ret);
	
	ret = UARTEX_Handle_Init(&h, USART2, &UART_Handle_Uart2_Default.Init, &rx, &tx, &hdmaex_rx, &hdmaex_tx, NULL, &ops);
	TEST_ASSERT_EQUAL(-EINVAL, ret);
	
	ret = UARTEX_Handle_Init(&h, USART2, &UART_Handle_Uart2_Default.Init, &rx, &tx, NULL, NULL, &hirq, &ops);
	TEST_ASSERT_EQUAL(-EINVAL, ret);
	
	ret = UARTEX_Handle_Init(&h, USART2, &UART_Handle_Uart2_Default.Init, &rx, &tx, NULL, &hdmaex_tx, NULL, &ops);
	TEST_ASSERT_EQUAL(-EINVAL, ret);
	
	ret = UARTEX_Handle_Init(&h, USART2, &UART_Handle_Uart2_Default.Init, &rx, &tx, &hdmaex_rx, NULL, NULL, &ops);
	TEST_ASSERT_EQUAL(-EINVAL, ret);
}

TEST(UARTEX_Handle, HandleInitSuccess)
{
  int ret;
	UARTEX_HandleTypeDef h;
	GPIOEX_TypeDef rx = PD6_As_Uart2Rx_Default;
	GPIOEX_TypeDef tx = PD5_As_Uart2Tx_Default;
	DMAEX_HandleTypeDef hdmaex_rx;
	DMAEX_HandleTypeDef	hdmaex_tx;
	IRQ_HandleTypeDef	hirq;
	struct UARTEX_Operations ops;
	
	memset(&ops, 0xA5, sizeof(ops));
	
	ret = UARTEX_Handle_Init(&h, USART2, &UART_Handle_Uart2_Default.Init, &rx, &tx, &hdmaex_rx, &hdmaex_tx, &hirq, &ops);
	
	TEST_ASSERT_EQUAL(0, ret);
	TEST_ASSERT_EQUAL_HEX32(&rx, h.rxpin);
	TEST_ASSERT_EQUAL_HEX32(&tx, h.txpin);
	TEST_ASSERT_EQUAL(USART2, h.huart.Instance);
	TEST_ASSERT_EQUAL_MEMORY(&UART_Handle_Uart2_Default.Init, &h.huart.Init, sizeof(UART_InitTypeDef));
	TEST_ASSERT_EQUAL_HEX32(&hdmaex_rx, h.hdmaex_rx);
	TEST_ASSERT_EQUAL_HEX32(&hdmaex_tx, h.hdmaex_tx);
	TEST_ASSERT_EQUAL_HEX32(&hirq, h.hirq);
	TEST_ASSERT_EQUAL_MEMORY(&ops, &h.ops, sizeof(ops));
	TEST_ASSERT_EQUAL_HEX32(0, h.testdata);
}

TEST(UARTEX_Handle, CtorByConfig)
{
	UARTEX_HandleTypeDef* h;
	GPIOEX_TypeDef rx = PD6_As_Uart2Rx_Default;
	GPIOEX_TypeDef tx = PD5_As_Uart2Tx_Default;
	DMAEX_HandleTypeDef hdmaex_rx;
	DMAEX_HandleTypeDef	hdmaex_tx;
	IRQ_HandleTypeDef	hirq;
	struct UARTEX_Operations ops;
	
	UART_ConfigTypeDef config;
	
	memset(&ops, 0xA5, sizeof(ops));
	memset(&config, 0xA5, sizeof(config));
	
	h = UARTEX_Handle_CtorByConfig(&config, &rx, &tx, &hdmaex_rx, &hdmaex_tx, &hirq, &ops);
	
	TEST_ASSERT_NOT_NULL(h);
	TEST_ASSERT_EQUAL_HEX32(&rx, h->rxpin);
	TEST_ASSERT_EQUAL_HEX32(&tx, h->txpin);
	TEST_ASSERT_EQUAL(config.Instance, h->huart.Instance);
	TEST_ASSERT_EQUAL_MEMORY(&config.Init, &h->huart.Init, sizeof(config.Init));
	TEST_ASSERT_EQUAL_HEX32(&hdmaex_rx, h->hdmaex_rx);
	TEST_ASSERT_EQUAL_HEX32(&hdmaex_tx, h->hdmaex_tx);
	TEST_ASSERT_EQUAL_HEX32(&hirq, h->hirq);
	TEST_ASSERT_EQUAL_MEMORY(&ops, &h->ops, sizeof(ops));
	TEST_ASSERT_EQUAL_HEX32(0, h->testdata);
	
	if (h) free(h);
}

TEST(UARTEX_Handle, HandleInitByConfigInvalidArgs)
{
	int ret;
	
	UARTEX_HandleTypeDef h;
	GPIOEX_TypeDef rx = PD6_As_Uart2Rx_Default;
	GPIOEX_TypeDef tx = PD5_As_Uart2Tx_Default;
	DMAEX_HandleTypeDef hdmaex_rx;
	DMAEX_HandleTypeDef	hdmaex_tx;
	IRQ_HandleTypeDef	hirq;
	struct UARTEX_Operations ops;
	
	UART_ConfigTypeDef config;
	
	memset(&ops, 0xA5, sizeof(ops));
	memset(&config, 0xA5, sizeof(config));
	config.Instance = USART2;
	
	ret = UARTEX_Handle_InitByConfig(NULL, &config, &rx, &tx, &hdmaex_rx, &hdmaex_tx, &hirq, &ops);
	TEST_ASSERT_EQUAL(-EINVAL, ret);
	
	ret = UARTEX_Handle_InitByConfig(&h, NULL, &rx, &tx, &hdmaex_rx, &hdmaex_tx, &hirq, &ops);
	TEST_ASSERT_EQUAL(-EINVAL, ret);
}

TEST(UARTEX_Handle, HandleInitByConfigSuccess)
{
	int ret;
	
	UARTEX_HandleTypeDef h;
	GPIOEX_TypeDef rx = PD6_As_Uart2Rx_Default;
	GPIOEX_TypeDef tx = PD5_As_Uart2Tx_Default;
	DMAEX_HandleTypeDef hdmaex_rx;
	DMAEX_HandleTypeDef	hdmaex_tx;
	IRQ_HandleTypeDef	hirq;
	struct UARTEX_Operations ops;
	
	UART_ConfigTypeDef config;
	
	memset(&ops, 0xA5, sizeof(ops));
	memset(&config, 0xA5, sizeof(config));
	config.Instance = USART2;
	
	ret = UARTEX_Handle_InitByConfig(&h, &config, &rx, &tx, &hdmaex_rx, &hdmaex_tx, &hirq, &ops);
	
	TEST_ASSERT_EQUAL(0, ret);
	TEST_ASSERT_EQUAL_HEX32(&rx, h.rxpin);
	TEST_ASSERT_EQUAL_HEX32(&tx, h.txpin);
	TEST_ASSERT_EQUAL(config.Instance, h.huart.Instance);
	TEST_ASSERT_EQUAL_MEMORY(&config.Init, &h.huart.Init, sizeof(config.Init));
	TEST_ASSERT_EQUAL_HEX32(&hdmaex_rx, h.hdmaex_rx);
	TEST_ASSERT_EQUAL_HEX32(&hdmaex_tx, h.hdmaex_tx);
	TEST_ASSERT_EQUAL_HEX32(&hirq, h.hirq);
	TEST_ASSERT_EQUAL_MEMORY(&ops, &h.ops, sizeof(ops));
	TEST_ASSERT_EQUAL_HEX32(0, h.testdata);
}


TEST(UARTEX_Handle, CtorInvalidArgs)
{
	UARTEX_HandleTypeDef* h;
	GPIOEX_TypeDef rx = PD6_As_Uart2Rx_Default;
	GPIOEX_TypeDef tx = PD5_As_Uart2Tx_Default;
	DMAEX_HandleTypeDef hdmaex_rx;
	DMAEX_HandleTypeDef	hdmaex_tx;
	IRQ_HandleTypeDef	hirq;
	struct UARTEX_Operations ops;
	
	// h = UARTEX_Handle_Ctor(USART2, &UART_Handle_Uart2_Default.Init, &rx, &tx, &hdmaex_rx, &hdmaex_tx, &hirq, &ops);
	h = UARTEX_Handle_Ctor(0, &UART_Handle_Uart2_Default.Init, &rx, &tx, &hdmaex_rx, &hdmaex_tx, &hirq, &ops);
	TEST_ASSERT_NULL(h);
	
	h = UARTEX_Handle_Ctor(USART2, 0, &rx, &tx, &hdmaex_rx, &hdmaex_tx, &hirq, &ops);
	TEST_ASSERT_NULL(h);
	
	h = UARTEX_Handle_Ctor(USART2, &UART_Handle_Uart2_Default.Init, 0, &tx, &hdmaex_rx, &hdmaex_tx, &hirq, &ops);
	TEST_ASSERT_NULL(h);
	
	h = UARTEX_Handle_Ctor(USART2, &UART_Handle_Uart2_Default.Init, &rx, 0, &hdmaex_rx, &hdmaex_tx, &hirq, &ops);
	TEST_ASSERT_NULL(h);
	
	h = UARTEX_Handle_Ctor(USART2, &UART_Handle_Uart2_Default.Init, &rx, &tx, &hdmaex_rx, &hdmaex_tx, &hirq, 0);
	TEST_ASSERT_NULL(h);
}

TEST(UARTEX_Handle, Dtor)
{
	UARTEX_HandleTypeDef* h;
	GPIOEX_TypeDef rx = PD6_As_Uart2Rx_Default;
	GPIOEX_TypeDef tx = PD5_As_Uart2Tx_Default;
	DMAEX_HandleTypeDef hdmaex_rx;
	DMAEX_HandleTypeDef	hdmaex_tx;
	IRQ_HandleTypeDef	hirq;
	
	h = UARTEX_Handle_Ctor(USART2, &UART_Handle_Uart2_Default.Init, &rx, &tx, &hdmaex_rx, &hdmaex_tx, &hirq, 0);
	if (h) UARTEX_Handle_Dtor(h);
}

TEST(UARTEX_Handle, FactoryCreate)
{
	UARTEX_HandleTypeDef* h;


	UARTEX_ConfigTypeDef	cfg =
	{
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
	

	h = UARTEX_Handle_FactoryCreate(
		&GPIO_ClockProvider,	
		&DMA_ClockProvider,
		&IRQ_HandlerObjectRegistry,
		&cfg);
		
	TEST_ASSERT_NOT_NULL(h);
	if (h) {
		
		TEST_ASSERT_NOT_NULL(h->hdmaex_rx);
		if (h->hdmaex_rx)
		{
			TEST_ASSERT_EQUAL_HEX32(cfg.dmarx->Instance, h->hdmaex_rx->hdma.Instance);
			TEST_ASSERT_EQUAL(cfg.dmarx_irq->irqn, h->hdmaex_rx->hirq->irqn);
		}
		
		TEST_ASSERT_NOT_NULL(h->hdmaex_tx);
		if (h->hdmaex_tx)
		{
			TEST_ASSERT_EQUAL_HEX32(cfg.dmatx->Instance, h->hdmaex_tx->hdma.Instance);
			TEST_ASSERT_EQUAL(cfg.dmatx_irq->irqn, h->hdmaex_tx->hirq->irqn);
		}
		
		TEST_ASSERT_NOT_NULL(h->rxpin);
		
		if (h->rxpin)
		{
			TEST_ASSERT_EQUAL(cfg.rxpin->instance, h->rxpin->instance);
			TEST_ASSERT_EQUAL(cfg.rxpin->init.Pin, h->rxpin->init.Pin);
		}
		
		TEST_ASSERT_NOT_NULL(h->txpin);
		if (h->txpin)
		{
			TEST_ASSERT_EQUAL(cfg.txpin->instance, h->txpin->instance);
			TEST_ASSERT_EQUAL(cfg.txpin->init.Pin, h->txpin->init.Pin);
		}
		
		TEST_ASSERT_NOT_NULL(h->hirq);
		if (h->hirq)
		{
			TEST_ASSERT_EQUAL(cfg.uart_irq->irqn, h->hirq->irqn);
		}
	
		TEST_ASSERT_EQUAL_MEMORY(&UARTEX_Ops_DefaultConfig, &h->ops, sizeof(h->ops));
		
		TEST_ASSERT_EQUAL(cfg.uart->Instance, h->huart.Instance);
	
		DMAEX_Handle_FactoryDestroy(h->hdmaex_rx);
		DMAEX_Handle_FactoryDestroy(h->hdmaex_tx);
		free(h->rxpin);
		free(h->txpin);
		free(h->hirq);
		
		free(h);
	}
}

TEST(UARTEX_Handle, FactoryCreateWithInvalidFactory)
{
	UARTEX_HandleTypeDef* h;
	
	GPIO_ClockProviderTypeDef* gpio_clk = &GPIO_ClockProvider;
	DMA_ClockProviderTypeDef* dma_clk = &DMA_ClockProvider;
	IRQ_HandleRegistryTypeDef* irq_registry =	&IRQ_HandlerObjectRegistry;

	UARTEX_ConfigTypeDef	cfg =
	{
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
	
//	const UART_ConfigTypeDef* uart_config = &UART2_DefaultConfig;
//	const GPIO_ConfigTypeDef* rxpin = &PD6_As_Uart2Rx_DefaultConfig;
//	const GPIO_ConfigTypeDef* txpin = &PD5_As_Uart2Tx_DefaultConfig;

//	const DMA_ConfigTypeDef* dmarx_config = &DMA_Uart2Rx_DefaultConfig;
//	const IRQ_ConfigTypeDef* dmarx_irq_config = &IRQ_Uart2RxDMA_DefaultConfig;
//	const DMA_ConfigTypeDef* dmatx_config = &DMA_Uart2Tx_DefaultConfig;
//	const IRQ_ConfigTypeDef* dmatx_irq_config = &IRQ_Uart2TxDMA_DefaultConfig;
//	const IRQ_ConfigTypeDef* uart_irq_config = &IRQ_Uart2_DefaultConfig;
//	const struct UARTEX_Operations* uart_ops = &UARTEX_Ops_DefaultConfig;
	
	// fac.dma_clk = NULL;
	// h = UARTEX_Handle_FactoryCreate(&fac, huart, rxpin, txpin, hdmarx, hirq_dmarx, hdmatx, hirq_dmatx, hirq_uart);
	// h = UARTEX_Handle_FactoryCreate(&fac, huart, rxpin, txpin, hdmarx, dmarx_irq_config, hdmatx, dmatx_irq_config, hirq_uart);
	h = UARTEX_Handle_FactoryCreate(NULL, dma_clk, irq_registry, &cfg);
		// uart_config, rxpin, txpin, dmarx_config, dmarx_irq_config, dmatx_config, dmatx_irq_config, uart_irq_config, uart_ops);
	TEST_ASSERT_NULL(h);
	
//	fac = factory;
//	fac.gpio_clk = NULL;
	// h = UARTEX_Handle_FactoryCreate(&fac, huart, rxpin, txpin, hdmarx, hirq_dmarx, hdmatx, hirq_dmatx, hirq_uart);
	// h = UARTEX_Handle_FactoryCreate(&fac, huart, rxpin, txpin, hdmarx, dmarx_irq_config, hdmatx, dmatx_irq_config, hirq_uart);
//	h = UARTEX_Handle_FactoryCreate(&fac, uart_config, rxpin, txpin, dmarx_config, dmarx_irq_config, dmatx_config, dmatx_irq_config, uart_irq_config);
	h = UARTEX_Handle_FactoryCreate(gpio_clk, NULL, irq_registry, &cfg);
		// uart_config, rxpin, txpin, dmarx_config, dmarx_irq_config, dmatx_config, dmatx_irq_config, uart_irq_config, uart_ops);

	TEST_ASSERT_NULL(h);

//	fac = factory;
//	fac.registry = NULL;
	// h = UARTEX_Handle_FactoryCreate(&fac, huart, rxpin, txpin, hdmarx, hirq_dmarx, hdmatx, hirq_dmatx, hirq_uart);
	// h = UARTEX_Handle_FactoryCreate(&fac, huart, rxpin, txpin, hdmarx, dmarx_irq_config, hdmatx, dmatx_irq_config, hirq_uart);
	// h = UARTEX_Handle_FactoryCreate(&fac, uart_config, rxpin, txpin, dmarx_config, dmarx_irq_config, dmatx_config, dmatx_irq_config, uart_irq_config);
	h = UARTEX_Handle_FactoryCreate(gpio_clk, dma_clk, NULL, &cfg);
		// uart_config, rxpin, txpin, dmarx_config, dmarx_irq_config, dmatx_config, dmatx_irq_config, uart_irq_config, uart_ops);

	TEST_ASSERT_NULL(h);

//	fac = factory;
//	fac.uart_ops = NULL;
	// h = UARTEX_Handle_FactoryCreate(&fac, huart, rxpin, txpin, hdmarx, hirq_dmarx, hdmatx, hirq_dmatx, hirq_uart);
	// h = UARTEX_Handle_FactoryCreate(&fac, huart, rxpin, txpin, hdmarx, dmarx_irq_config, hdmatx, dmatx_irq_config, hirq_uart);
//	h = UARTEX_Handle_FactoryCreate(&fac, uart_config, rxpin, txpin, dmarx_config, dmarx_irq_config, dmatx_config, dmatx_irq_config, uart_irq_config);
	h = UARTEX_Handle_FactoryCreate(gpio_clk, dma_clk, irq_registry, NULL);
		// uart_config, rxpin, txpin, dmarx_config, dmarx_irq_config, dmatx_config, dmatx_irq_config, uart_irq_config, NULL);

	TEST_ASSERT_NULL(h);
}

TEST(UARTEX_Handle, FactoryCreateWithoutDMAIRQ)
{
	UARTEX_HandleTypeDef* h;

	GPIO_ClockProviderTypeDef* gpio_clk = &GPIO_ClockProvider;
	DMA_ClockProviderTypeDef* dma_clk = &DMA_ClockProvider;
	IRQ_HandleRegistryTypeDef* irq_registry =	&IRQ_HandlerObjectRegistry;	

	UARTEX_ConfigTypeDef	cfg_nodma_noirq =
	{
		.uart = &UART2_DefaultConfig,
		.rxpin = &PD6_As_Uart2Rx_DefaultConfig,
		.txpin = &PD5_As_Uart2Tx_DefaultConfig,
		.uartex_ops = &UARTEX_Ops_DefaultConfig,
	};		

//	const UART_ConfigTypeDef* uart_config = &UART2_DefaultConfig;
//	const GPIO_ConfigTypeDef* rxpin = &PD6_As_Uart2Rx_DefaultConfig;
//	const GPIO_ConfigTypeDef* txpin = &PD5_As_Uart2Tx_DefaultConfig;
//	
//	const struct UARTEX_Operations* uart_ops = &UARTEX_Ops_DefaultConfig;
	
	h = UARTEX_Handle_FactoryCreate( /** &factory, **/
		gpio_clk, dma_clk, irq_registry, &cfg_nodma_noirq);
//	uart_config, rxpin, txpin, NULL, NULL, NULL, NULL, NULL, uart_ops);
	
	TEST_ASSERT_NOT_NULL(h);
	if (h) {
		
		TEST_ASSERT_NULL(h->hdmaex_rx);		
		TEST_ASSERT_NULL(h->hdmaex_tx);		
		TEST_ASSERT_NULL(h->hirq);
		
		free(h->rxpin);
		free(h->txpin);
		
		free(h);
	}	
}

TEST(UARTEX_Handle, FactoryDestroy)
{
	UARTEX_HandleTypeDef* h;
	
	GPIO_ClockProviderTypeDef* gpio_clk = &GPIO_ClockProvider;
	DMA_ClockProviderTypeDef* dma_clk = &DMA_ClockProvider;
	IRQ_HandleRegistryTypeDef* irq_registry =	&IRQ_HandlerObjectRegistry;	
	
	UARTEX_ConfigTypeDef	cfg =
	{
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
	
//	const struct UARTEX_Operations* uart_ops = &UARTEX_Ops_DefaultConfig;
	
//	UARTEX_Handle_FactoryTypeDef factory;
//	
//	factory.dma_clk = &DMA_ClockProvider;
//	factory.gpio_clk = &GPIO_ClockProvider;
//	factory.registry = &IRQ_HandlerObjectRegistry;
	
//	const UART_ConfigTypeDef* uart_config = &UART2_DefaultConfig;
//	const GPIO_ConfigTypeDef* rxpin = &PD6_As_Uart2Rx_DefaultConfig;
//	const GPIO_ConfigTypeDef* txpin = &PD5_As_Uart2Tx_DefaultConfig;
//	const DMA_ConfigTypeDef* dmarx_config = &DMA_Uart2Rx_DefaultConfig;
//	const IRQ_ConfigTypeDef* dmarx_irq_config = &IRQ_Uart2RxDMA_DefaultConfig;
//	const DMA_ConfigTypeDef* dmatx_config = &DMA_Uart2Tx_DefaultConfig;
//	const IRQ_ConfigTypeDef* dmatx_irq_config = &IRQ_Uart2TxDMA_DefaultConfig;
//	const IRQ_ConfigTypeDef* uart_irq_config = &IRQ_Uart2_DefaultConfig;
	
	h = UARTEX_Handle_FactoryCreate(gpio_clk, dma_clk, irq_registry, &cfg);
//		uart_config, rxpin, txpin, 
//		dmarx_config,			/** hdmarx, 		**/
//		dmarx_irq_config, /** hirq_dmarx, **/
//		dmatx_config, 		/**	hdmatx, 		**/
//		dmatx_irq_config, /** hirq_dmatx, **/
//		uart_irq_config, 
//		uart_ops);
	
	UARTEX_Handle_FactoryDestroy(h);
}

TEST_GROUP_RUNNER(UARTEX_Handle)
{
	RUN_TEST_CASE(UARTEX_Handle, HandleInitSuccess);
	RUN_TEST_CASE(UARTEX_Handle, HandleInitInvalidArgs);
	
	RUN_TEST_CASE(UARTEX_Handle, HandleInitByConfigInvalidArgs);
	RUN_TEST_CASE(UARTEX_Handle, HandleInitByConfigSuccess);
	
	RUN_TEST_CASE(UARTEX_Handle, Ctor);
	RUN_TEST_CASE(UARTEX_Handle, CtorByConfig);
	RUN_TEST_CASE(UARTEX_Handle, CtorInvalidArgs);
	RUN_TEST_CASE(UARTEX_Handle, Dtor);
	
	RUN_TEST_CASE(UARTEX_Handle, FactoryCreate);
	RUN_TEST_CASE(UARTEX_Handle, FactoryCreateWithInvalidFactory);
	RUN_TEST_CASE(UARTEX_Handle, FactoryCreateWithoutDMAIRQ);
	RUN_TEST_CASE(UARTEX_Handle, FactoryDestroy);
}


///////////////////////////////////////////////////////////////////////////////
// The first several case are learning tests
// The following tests requires hardware connections, short usart 2 tx and rx
//
TEST_GROUP(UART_DMA_TxRx);
TEST_SETUP(UART_DMA_TxRx)
{	
	UARTEX_ConfigTypeDef	cfg =
	{
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
	
	struct msp_factory msp = {
		
		.gpio_clk = &GPIO_ClockProvider,
		.dma_clk = &DMA_ClockProvider,
		.irq_registry = &IRQ_HandlerObjectRegistry,
		
		.create_dmaex_handle = msp_create_dmaex_handle,
		.gpioex_init_by_config = GPIOEX_InitByConfig,		
		.irq_handle_init_by_config = IRQ_Handle_InitByConfig,
		.uartex_handle_init_by_config = UARTEX_Handle_InitByConfig,
	};
	
	HUARTEX_DMA = msp_create_uartex_handle(&msp, &cfg);	
	HAL_UART_Init(HUART_DMA);	
}

TEST_TEAR_DOWN(UART_DMA_TxRx)
{
	volatile uint32_t dr;
	
//	UARTEX_Handle_FactoryTypeDef factory;
//	factory.dma_clk = &DMA_ClockProvider;
//	factory.gpio_clk = &GPIO_ClockProvider;
//	factory.registry = &IRQ_HandlerObjectRegistry;
	
	HAL_UART_DMAStop(HUART_DMA);
	/** read to clear ORE **/
	if (HUART_DMA->Instance->SR & USART_SR_RXNE)
		dr = HUART_DMA->Instance->DR;
	HAL_UART_DeInit(HUART_DMA);
	
	UARTEX_Handle_FactoryDestroy(HUARTEX_DMA);	
}

void loopBackSixBytesDMA(void) 	// reuse this op to demonstrate the usart is fine after certain operation 
{
		
	char rx[7];
	char tx[7];
	char token[6] = "@#$%^&";

	/** prepare **/
	memset(rx, 0, 7);
	memset(tx, 0, 7);
	memmove(tx, token, strlen(token));
	
	TEST_ASSERT_EQUAL(HAL_UART_STATE_READY, HUART_DMA->State);
	TEST_ASSERT_EQUAL(HAL_UART_ERROR_NONE, HUART_DMA->ErrorCode);
	
	/** recv **/
	HAL_UART_Receive_DMA(HUART_DMA, (uint8_t*)rx, strlen(tx));
	TEST_ASSERT_EQUAL(HAL_UART_STATE_BUSY_RX, HUART_DMA->State);
	
	/** send **/
	HAL_UART_Transmit_DMA(HUART_DMA, (uint8_t*)tx, strlen(tx));
	TEST_ASSERT_EQUAL_HEX8(HAL_UART_STATE_BUSY_TX_RX, HUART_DMA->State);	/** this may fail if compiler optimization level low **/
	
	/** wait and check **/
	HAL_Delay(100);
	TEST_ASSERT_EQUAL_HEX8(HAL_UART_STATE_READY, HUART_DMA->State);
	TEST_ASSERT_EQUAL(HAL_UART_ERROR_NONE, HUART_DMA->ErrorCode);
	TEST_ASSERT_EQUAL_STRING(token, rx);	
}

TEST(UART_DMA_TxRx, LoopBackSixBytesTwice)
{
	loopBackSixBytesDMA();
	loopBackSixBytesDMA();
}

TEST(UART_DMA_TxRx, ReceiveMultipleBytesInWaitStopCheckManner)
{
	char rx[64];
	char tx[64];
	
	/** prepare **/
	memset(rx, 0, sizeof(rx));
	memset(tx, 0, sizeof(tx));
	snprintf(tx, sizeof(tx),  "i love test driven development.");
	TEST_ASSERT_EQUAL(HAL_UART_STATE_READY, HUART_DMA->State);

	/** receive **/
	HAL_UART_Receive_DMA(HUART_DMA, (uint8_t*)rx, sizeof(rx));
	TEST_ASSERT_EQUAL(HAL_UART_STATE_BUSY_RX, HUART_DMA->State);
	
	/** send **/
	HAL_UART_Transmit_DMA(HUART_DMA, (uint8_t*)tx, strlen(tx));
	TEST_ASSERT_EQUAL(HAL_UART_STATE_BUSY_TX_RX, HUART_DMA->State);
	
	/** wait **/
	HAL_Delay(100);
	TEST_ASSERT_EQUAL(HAL_UART_STATE_BUSY_RX, HUART_DMA->State);
	TEST_ASSERT_EQUAL(sizeof(rx) - strlen(tx), __HAL_DMA_GET_COUNTER(HUART_DMA->hdmarx));
	TEST_ASSERT_EQUAL_STRING(tx, rx);
		
	/** 
	#define __HAL_UART_ENABLE(__HANDLE__)               ((__HANDLE__)->Instance->CR1 |=  USART_CR1_UE)
	#define __HAL_UART_DISABLE(__HANDLE__)              ((__HANDLE__)->Instance->CR1 &=  ~USART_CR1_UE) 
	**/		
	
	/** stop **/
	HAL_UART_DMAStop(HUART_DMA);
	TEST_ASSERT_FALSE(HUART_DMA->Instance->CR1 & USART_CR1_UE);
	__HAL_UART_ENABLE(HUART_DMA);		/** <-- fix the problem **/
	TEST_ASSERT_TRUE(HUART_DMA->Instance->CR1 & USART_CR1_UE);
	
	TEST_ASSERT_EQUAL(HAL_UART_STATE_READY, HUART_DMA->State);
	TEST_ASSERT_EQUAL(sizeof(rx) - strlen(tx), __HAL_DMA_GET_COUNTER(HUART_DMA->hdmarx));

	loopBackSixBytesDMA();
}

TEST(UART_DMA_TxRx, ReceiveMultipleBytesInSuspendResumeManner)
{
	char rx[64];
	char tx[64];
	char fill[64];
	int	recv_cnt;
	uint32_t timestamp, timestamp2;
	
	/** prepare **/
	recv_cnt = 0;
	memset(rx, 0, sizeof(rx));
	memset(fill, 0, sizeof(fill));
	snprintf(tx, sizeof(tx), "i love test driven development. i love test driven development");
	TEST_ASSERT_EQUAL(HAL_UART_STATE_READY, HUART_DMA->State);
	
	/** receive **/
	HAL_UART_Receive_DMA(HUART_DMA, (uint8_t*)rx, sizeof(rx));
	TEST_ASSERT_EQUAL(HAL_UART_STATE_BUSY_RX, HUART_DMA->State);

	/** send **/
	HAL_UART_Transmit_DMA(HUART_DMA, (uint8_t*)tx, strlen(tx));
	TEST_ASSERT_EQUAL(HAL_UART_STATE_BUSY_TX_RX, HUART_DMA->State);

	/** timeout to prevent dead loop **/
	timestamp = HAL_GetTick();
	for (;;) {
		
		if (HAL_GetTick() - timestamp > 110) {	// fail occasionally, don't know why. change this value and
																						// recompile probably fix the problem.
			TEST_FAIL_MESSAGE(fill);
			return;
		}
		
		if (__HAL_DMA_GET_COUNTER(HUART_DMA->hdmarx) == sizeof(rx)) 
			continue;
		
		/** suspend, disable tc intr before clear EN bit **/
		HAL_UART_DMAPause(HUART_DMA);
		__HAL_DMA_DISABLE_IT(HUART_DMA->hdmarx, DMA_IT_TC);
		__HAL_DMA_DISABLE(HUART_DMA->hdmarx);	/** this clear DMA_SxCR_EN bit**/
		
		/* timeout to prevent dead loop **/
		timestamp2 = HAL_GetTick();
		for (;;) {
			
			if (HAL_GetTick() - timestamp2 > 100) {
				TEST_FAIL_MESSAGE("TIMEOUT when receiving data.");
				return;
			}
			
			if ((HUART_DMA->hdmarx->Instance->CR & DMA_SxCR_EN) == 0)
				break;
		}
		
		recv_cnt = sizeof(rx) - __HAL_DMA_GET_COUNTER(HUART_DMA->hdmarx);

		/** prevent buffer overflow **/
		TEST_ASSERT_TRUE(strlen(fill) + strlen(rx) < sizeof(fill) - 1);
		strcat(fill, rx);
		memset(rx, 0, recv_cnt);
		
		/** not necessarily, just prove the register writable now **/
		HUART_DMA->hdmarx->Instance->M0AR = (uint32_t)rx;			
		/** not necessarily if previous value reused **/
		__HAL_DMA_SET_COUNTER(HUART_DMA->hdmarx, sizeof(rx));

		/** clear tc flag and resume **/
		__HAL_DMA_CLEAR_FLAG(HUART_DMA->hdmarx, __HAL_DMA_GET_TC_FLAG_INDEX(HUART_DMA->hdmarx));
		__HAL_DMA_ENABLE(HUART_DMA->hdmarx);
		__HAL_DMA_ENABLE_IT(HUART_DMA->hdmarx, DMA_IT_TC);
		HAL_UART_DMAResume(HUART_DMA);
		
		/** make sure fill IS a string **/
		TEST_ASSERT_TRUE(0 == fill[sizeof(fill) - 1]);
		if (0 == strcmp(fill, tx)) {
			break;
		}
	}
}


static void receiveBufferOverflow(int rx_buf_size, int by_num) {

	char rx[64];

	/** prepare **/
	memset(rx, 0, sizeof(rx));
	TEST_ASSERT_EQUAL_HEX8(HAL_UART_STATE_READY, HUART_DMA->State);

	/** start receive **/
	HAL_UART_Receive_DMA(HUART_DMA, (uint8_t*)rx, rx_buf_size);
	TEST_ASSERT_EQUAL_HEX8(HAL_UART_STATE_BUSY_RX, HUART_DMA->State);
		
	/** send and overflow **/
	HAL_UART_Transmit_DMA(HUART_DMA, (uint8_t*)alphabet, rx_buf_size + by_num);
	TEST_ASSERT_EQUAL_HEX8(HAL_UART_STATE_BUSY_TX_RX, HUART_DMA->State);
		
	/** wait **/
	HAL_Delay(200);
	TEST_ASSERT_EQUAL_HEX8(HAL_UART_STATE_READY, HUART_DMA->State);	/** state ready 				**/
	TEST_ASSERT_EQUAL(0, __HAL_DMA_GET_COUNTER(HUART_DMA->hdmarx));	/** counter reg reach 0 **/
	TEST_ASSERT_TRUE(HUART_DMA->Instance->CR1 & USART_CR1_UE);				/** uart not disabled 	**/
	TEST_ASSERT_EQUAL_MEMORY(alphabet, rx, rx_buf_size);				/** mem equal						**/	
}

TEST(UART_DMA_TxRx, ReceiveMultipleBytesAfterRxOverflowByZero)
{
	uint32_t usart_sr;
	
	char rx[64];

	/** prepare **/
	memset(rx, 0, sizeof(rx));
	TEST_ASSERT_EQUAL_HEX8(HAL_UART_STATE_READY, HUART_DMA->State);

	/** start receive **/
	HAL_UART_Receive_DMA(HUART_DMA, (uint8_t*)rx, 8 /** rx_buf_size **/);
	TEST_ASSERT_EQUAL_HEX8(HAL_UART_STATE_BUSY_RX, HUART_DMA->State);
		
	/** send and overflow **/
	HAL_UART_Transmit_DMA(HUART_DMA, (uint8_t*)alphabet, 8 /** rx_buf_size + by_num **/);
	TEST_ASSERT_EQUAL_HEX8(HAL_UART_STATE_BUSY_TX_RX, HUART_DMA->State);
		
	/** wait **/
	HAL_Delay(200);
	TEST_ASSERT_EQUAL_HEX8(HAL_UART_STATE_READY, HUART_DMA->State);	/** state ready 				**/
	TEST_ASSERT_EQUAL(0, __HAL_DMA_GET_COUNTER(HUART_DMA->hdmarx));	/** counter reg reach 0 **/
	TEST_ASSERT_TRUE(HUART_DMA->Instance->CR1 & USART_CR1_UE);				/** uart not disabled 	**/
	TEST_ASSERT_EQUAL_MEMORY(alphabet, rx, 8 /* rx_buf_size */);				/** mem equal						**/		
	
	usart_sr = HUART_DMA->Instance->SR;
	TEST_ASSERT_FALSE(usart_sr & USART_SR_ORE);		/* no overrun */
	TEST_ASSERT_FALSE(usart_sr & USART_SR_RXNE);	/* no rx data */
}

TEST(UART_DMA_TxRx, ReceiveMultipleBytesAfterRxOverflowByOne)
{
	char chr;
	int buf_size = 8;
	uint32_t usart_sr;
	
	receiveBufferOverflow(buf_size, 1);
	
	usart_sr = HUART_DMA->Instance->SR;
	TEST_ASSERT_FALSE(usart_sr & USART_SR_ORE);		/* no overrun 					*/
	TEST_ASSERT_TRUE(usart_sr & USART_SR_RXNE);		/* has rx data in DR 		*/
	
	chr = 0;
	HAL_UART_Receive_DMA(HUART_DMA, (uint8_t*)&chr, 1);
	
	HAL_Delay(10);
	TEST_ASSERT_EQUAL(HAL_UART_STATE_READY, HUART_DMA->State);				/** state ready 				**/
	TEST_ASSERT_EQUAL(0, __HAL_DMA_GET_COUNTER(HUART_DMA->hdmarx));	/** counter reg reach 0 **/
	
	usart_sr = HUART_DMA->Instance->SR;
	TEST_ASSERT_FALSE(usart_sr & USART_SR_ORE);		/* overrun flag cleared */
	TEST_ASSERT_FALSE(usart_sr & USART_SR_RXNE);	/* no rx data now 			*/
	TEST_ASSERT_EQUAL(alphabet[buf_size], chr);
}

TEST(UART_DMA_TxRx, ReceiveMultipleBytesAfterRxOverflowByTwo)
{
	char chr;
	int buf_size = 8;
	uint32_t usart_sr;
	
	receiveBufferOverflow(buf_size, 2);
	
	usart_sr = HUART_DMA->Instance->SR;
	TEST_ASSERT_TRUE(usart_sr & USART_SR_ORE);		/* has overrun 					*/
	TEST_ASSERT_TRUE(usart_sr & USART_SR_RXNE);		/* has rx data in DR 		*/
	
	chr = 0;
	HAL_UART_Receive_DMA(HUART_DMA, (uint8_t*)&chr, 1);
	
	HAL_Delay(10);
	TEST_ASSERT_EQUAL(HAL_UART_STATE_READY, HUART_DMA->State);				/** state ready 				**/
	TEST_ASSERT_EQUAL(0, __HAL_DMA_GET_COUNTER(HUART_DMA->hdmarx));	/** counter reg reach 0 **/
	
	usart_sr = HUART_DMA->Instance->SR;
	TEST_ASSERT_FALSE(usart_sr & USART_SR_ORE);		/* overrun flag cleared */
	TEST_ASSERT_FALSE(usart_sr & USART_SR_RXNE);	/* no rx data now 			*/
	TEST_ASSERT_EQUAL(alphabet[buf_size], chr);		
}


TEST(UART_DMA_TxRx, SwapRxDMABufferInvalidArgs)
{
	TEST_ASSERT_EQUAL(HAL_ERROR, HAL_UART_SwapRxDMABuffer(0, (uint8_t*)1, 1, 0, 0));					/** null handle **/
	TEST_ASSERT_EQUAL(HAL_ERROR, HAL_UART_SwapRxDMABuffer(HUART_DMA, 0, 1, 0, 0));						/** null buf pointer **/
	TEST_ASSERT_EQUAL(HAL_ERROR, HAL_UART_SwapRxDMABuffer(HUART_DMA, (uint8_t*)1, 0, 0, 0));	/** zero pointer size **/
}

TEST(UART_DMA_TxRx, SwapRxDMABufferInvalidUartHalState)
{
	UART_HandleTypeDef handle;
	char chr;
	
	memset(&handle, 0, sizeof(handle));
	
	/** treat RESET as TIMEOUT since it can not proceed either, see below **/
	handle.State = HAL_UART_STATE_RESET;
	TEST_ASSERT_EQUAL(HAL_TIMEOUT, HAL_UART_SwapRxDMABuffer(&handle, (uint8_t*)&chr, 1, 0, 0));
	
	/** 
	For Firmware 1.1.0, UART HAL do *NOT* really use this state. 
	
	The error policy seems to be:
	(1) If the error does NOT prevent hardware from working, the state 
			is READY and ErrorCode is set. The read / write operation can proceed.
	(2) The only error that can not proceed is TIMEOUT, which occurs when set/clear
			register bit and no response. 
	
	So here we treat HAL_UART_STATE_ERROR as non-recoverable error, just as TIMEOUT
	**/
	handle.State = HAL_UART_STATE_ERROR;
	TEST_ASSERT_EQUAL(HAL_TIMEOUT, HAL_UART_SwapRxDMABuffer(&handle, (uint8_t*)&chr, 1, 0, 0));
	
	handle.State = HAL_UART_STATE_BUSY;
	TEST_ASSERT_EQUAL(HAL_BUSY, HAL_UART_SwapRxDMABuffer(&handle, (uint8_t*)&chr, 1, 0, 0));
	
	handle.State = HAL_UART_STATE_TIMEOUT;
	TEST_ASSERT_EQUAL(HAL_TIMEOUT, HAL_UART_SwapRxDMABuffer(&handle, (uint8_t*)&chr, 1, 0, 0));
}

TEST(UART_DMA_TxRx, SwapRxDMABufferReplaceEmptyBuffer)
{
	char original[16];
	char replace[16];
	uint32_t m0addr;
	int count;
	HAL_StatusTypeDef status;
	
	/** prepare **/
	TEST_ASSERT_EQUAL(HAL_UART_STATE_READY, HUART_DMA->State);
	TEST_ASSERT_EQUAL(HAL_OK, HAL_UART_Receive_DMA(HUART_DMA, (uint8_t*)original, sizeof(original)));
	TEST_ASSERT_EQUAL_HEX8(HAL_UART_STATE_BUSY_RX, HUART_DMA->State);
	
	/** do **/
	status  = HAL_UART_SwapRxDMABuffer(HUART_DMA, (uint8_t*)replace, sizeof(replace), &m0addr, &count);
	
	/** check **/
	TEST_ASSERT_EQUAL(HAL_OK, status);
	TEST_ASSERT_EQUAL(HAL_UART_STATE_BUSY_RX, HUART_DMA->State);
	TEST_ASSERT_EQUAL_HEX32((uint32_t)original, m0addr);
	TEST_ASSERT_EQUAL(sizeof(original), count);
	
	/** verify registers **/
	TEST_ASSERT_EQUAL_HEX32((uint32_t)replace, HUART_DMA->hdmarx->Instance->M0AR);
	TEST_ASSERT_EQUAL_HEX32((uint32_t)sizeof(replace), __HAL_DMA_GET_COUNTER(HUART_DMA->hdmarx));
}

TEST_GROUP_RUNNER(UART_DMA_TxRx)
{
	/** these are learning tests. **/
	RUN_TEST_CASE(UART_DMA_TxRx, LoopBackSixBytesTwice);
	RUN_TEST_CASE(UART_DMA_TxRx, ReceiveMultipleBytesInWaitStopCheckManner);
	RUN_TEST_CASE(UART_DMA_TxRx, ReceiveMultipleBytesInSuspendResumeManner);
	RUN_TEST_CASE(UART_DMA_TxRx, ReceiveMultipleBytesAfterRxOverflowByZero);
	RUN_TEST_CASE(UART_DMA_TxRx, ReceiveMultipleBytesAfterRxOverflowByOne);
	RUN_TEST_CASE(UART_DMA_TxRx, ReceiveMultipleBytesAfterRxOverflowByTwo);
	
	/** these are 'real' tests. **/
	RUN_TEST_CASE(UART_DMA_TxRx, SwapRxDMABufferInvalidArgs);
	RUN_TEST_CASE(UART_DMA_TxRx, SwapRxDMABufferInvalidUartHalState);
	RUN_TEST_CASE(UART_DMA_TxRx, SwapRxDMABufferReplaceEmptyBuffer);
}






///////////////////////////////////////////////////////////////////////////////
// TEST GROUP - UART Msp
//
TEST_GROUP(UART_Msp);
TEST_SETUP(UART_Msp)
{
	CreateTestData();
}

TEST_TEAR_DOWN(UART_Msp)
{
	DestroyTestData();
}

TEST(UART_Msp, InitUartClockShouldBeEnabled)
{
	HAL_UART_MspInit(HUART);
	TEST_ASSERT_TRUE(HAL_UART_ClockIsEnabled(HUART->Instance));
	HAL_UART_MspDeInit(HUART);
}


TEST(UART_Msp, InitRxPinGpioShouldBeInitialized)
{
	HAL_UART_MspInit(HUART);
	TEST_ASSERT_EQUAL(GPIOEX_STATE_SET, HUARTEX->rxpin->state);
	HAL_UART_MspDeInit(HUART);
}

TEST(UART_Msp, InitTxPinGpioShouldBeInitialized)
{
	HAL_UART_MspInit(HUART);
	TEST_ASSERT_EQUAL(GPIOEX_STATE_SET, HUARTEX->txpin->state);
	HAL_UART_MspDeInit(HUART);
}

TEST(UART_Msp, InitRxDMAShouldBeLinked)
{
	HAL_UART_MspInit(HUART_DMA);
	TEST_ASSERT_TRUE(uart_rxdma_linked(HUART_DMA, &HUARTEX_DMA->hdmaex_rx->hdma));
	HAL_UART_MspDeInit(HUART_DMA);
}

TEST(UART_Msp, InitRxDMAEXShouldBeInitialized)
{
	HAL_UART_MspInit(HUART_DMA);
	TEST_ASSERT_EQUAL(DMAEX_HANDLE_STATE_SET, HUARTEX_DMA->hdmaex_rx->state);
	HAL_UART_MspDeInit(HUART_DMA);
}


TEST(UART_Msp, InitTxDMAShouldBeLinked)
{
	HAL_UART_MspInit(HUART_DMA);
	TEST_ASSERT_TRUE(uart_txdma_linked(HUART_DMA, &HUARTEX_DMA->hdmaex_tx->hdma));
	HAL_UART_MspDeInit(HUART_DMA);
}

TEST(UART_Msp, InitTxDMAEXShouldBeInitialized)
{
	HAL_UART_MspInit(HUART_DMA);
	TEST_ASSERT_EQUAL(DMAEX_HANDLE_STATE_SET, HUARTEX_DMA->hdmaex_tx->state);
	HAL_UART_MspDeInit(HUART_DMA);
}

TEST(UART_Msp, InitUartIRQHandleShouldBeInitialized)
{
	HAL_UART_MspInit(HUART_DMA);
	TEST_ASSERT_EQUAL(IRQ_HANDLE_STATE_SET, HUARTEX_DMA->hirq->state);
	HAL_UART_MspDeInit(HUART_DMA);
}

TEST(UART_Msp, DeInitUartIRQHandleShouldBeDeInitialized)
{
	HAL_UART_MspInit(HUART_DMA);
	HAL_UART_MspDeInit(HUART_DMA);
	TEST_ASSERT_EQUAL(IRQ_HANDLE_STATE_RESET, HUARTEX_DMA->hirq->state);
}

TEST(UART_Msp, DeInitTxDMAEXShouldBeDeInitialized)
{
	HAL_UART_MspInit(HUART_DMA);
	HAL_UART_MspDeInit(HUART_DMA);
	TEST_ASSERT_EQUAL(DMAEX_HANDLE_STATE_RESET, HUARTEX_DMA->hdmaex_tx->state);
}

TEST(UART_Msp, DeInitTxDMAShouldBeUnlinked)
{
	HAL_UART_MspInit(HUART_DMA);
	HAL_UART_MspDeInit(HUART_DMA);
	TEST_ASSERT_NULL(HUART_DMA->hdmatx);
	TEST_ASSERT_NULL(HUARTEX_DMA->hdmaex_tx->hdma.Parent);
}

TEST(UART_Msp, DeInitRxDMAEXShouldBeDeInitialized)
{
	HAL_UART_MspInit(HUART_DMA);
	HAL_UART_MspDeInit(HUART_DMA);
	TEST_ASSERT_EQUAL(DMAEX_HANDLE_STATE_RESET, HUARTEX_DMA->hdmaex_rx->state);
}

TEST(UART_Msp, DeInitRxDMAShouldBeUnlinked)
{
	HAL_UART_MspInit(HUART_DMA);
	HAL_UART_MspDeInit(HUART_DMA);
	TEST_ASSERT_NULL(HUART_DMA->hdmarx);
	TEST_ASSERT_NULL(HUARTEX_DMA->hdmaex_rx->hdma.Parent);
}

TEST(UART_Msp, DeInitRxTxPinGpioShouldBeDeInitialized)
{
	HAL_UART_MspInit(HUART_DMA);
	HAL_UART_MspDeInit(HUART_DMA);
	TEST_ASSERT_EQUAL(GPIOEX_STATE_RESET, HUARTEX_DMA->rxpin->state);
	TEST_ASSERT_EQUAL(GPIOEX_STATE_RESET, HUARTEX_DMA->txpin->state);
}

TEST_GROUP_RUNNER(UART_Msp)
{
	RUN_TEST_CASE(UART_Msp, InitUartClockShouldBeEnabled);
	RUN_TEST_CASE(UART_Msp, InitRxPinGpioShouldBeInitialized);
	RUN_TEST_CASE(UART_Msp, InitTxPinGpioShouldBeInitialized);
	RUN_TEST_CASE(UART_Msp, InitRxDMAShouldBeLinked);
	RUN_TEST_CASE(UART_Msp, InitRxDMAEXShouldBeInitialized);
	RUN_TEST_CASE(UART_Msp, InitTxDMAShouldBeLinked);
	RUN_TEST_CASE(UART_Msp, InitTxDMAEXShouldBeInitialized);
	RUN_TEST_CASE(UART_Msp, InitUartIRQHandleShouldBeInitialized);

	RUN_TEST_CASE(UART_Msp, DeInitUartIRQHandleShouldBeDeInitialized);
	RUN_TEST_CASE(UART_Msp, DeInitTxDMAEXShouldBeDeInitialized);
	RUN_TEST_CASE(UART_Msp, DeInitTxDMAShouldBeUnlinked);
	RUN_TEST_CASE(UART_Msp, DeInitRxDMAEXShouldBeDeInitialized);
	RUN_TEST_CASE(UART_Msp, DeInitRxDMAShouldBeUnlinked);
	RUN_TEST_CASE(UART_Msp, DeInitRxTxPinGpioShouldBeDeInitialized);
}





















///////////////////////////////////////////////////////////////////////////////
// hardware independent, and hardware dependent
// 
TEST_GROUP_RUNNER(UART_All_HI)
{
	RUN_TEST_GROUP(UARTEX_Handle);	
	RUN_TEST_GROUP(UART_Msp);	
}

TEST_GROUP_RUNNER(UART_All_HD)
{
	RUN_TEST_GROUP(UART_DMA_TxRx);
}








