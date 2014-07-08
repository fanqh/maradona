#include <string.h>
#include "stm32f4xx_hal.h"
#include "usart.h"
#include "usart_test.h"
#include "unity_fixture.h"


#define HUARTEX_DMA					(&huartex2_dma)
#define HDMA_RX_UART				(&hdma_rx_uart2)
#define HDMA_TX_UART				(&hdma_tx_uart2)
#define HUART_DMA						(&huartex2_dma.huart)

const char alphabet[] = "abcdefghijklmnopqrstuvwxyz";

// static UARTEX_HandleTypeDef	huartex2;
static UARTEX_HandleTypeDef huartex2_dma;

/******************************************************************************

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


/******************************************************************************
#define __GPIOA_CLK_ENABLE()         (RCC->AHB1ENR |= (RCC_AHB1ENR_GPIOAEN))
#define __GPIOB_CLK_ENABLE()         (RCC->AHB1ENR |= (RCC_AHB1ENR_GPIOBEN))
#define __GPIOC_CLK_ENABLE()         (RCC->AHB1ENR |= (RCC_AHB1ENR_GPIOCEN))
#define __GPIOD_CLK_ENABLE()         (RCC->AHB1ENR |= (RCC_AHB1ENR_GPIODEN))
#define __GPIOE_CLK_ENABLE()         (RCC->AHB1ENR |= (RCC_AHB1ENR_GPIOEEN))
#define __GPIOH_CLK_ENABLE()         (RCC->AHB1ENR |= (RCC_AHB1ENR_GPIOHEN))
******************************************************************************/
bool port_clock_enabled(GPIO_TypeDef* gpio_port)
{
	if (gpio_port == GPIOA) {
		return (RCC->AHB1ENR & RCC_AHB1ENR_GPIOAEN) ? true : false;
	}
	else if (gpio_port == GPIOB) {
		return (RCC->AHB1ENR & RCC_AHB1ENR_GPIOBEN) ? true : false;
	}
	else if (gpio_port == GPIOC) {
		return (RCC->AHB1ENR & RCC_AHB1ENR_GPIOCEN) ? true : false;
	}
	else if (gpio_port == GPIOD) {
		return (RCC->AHB1ENR & RCC_AHB1ENR_GPIODEN) ? true : false;
	}
	else if (gpio_port == GPIOE) {
		return (RCC->AHB1ENR & RCC_AHB1ENR_GPIOEEN) ? true : false;
	}
	else if (gpio_port == GPIOH) {
		return (RCC->AHB1ENR & RCC_AHB1ENR_GPIOHEN) ? true : false;
	}
	else {
		/** gpio f, g, & i fall here **/
		assert_param(false);
	}
}

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

// #define __DMA1_CLK_ENABLE()          (RCC->AHB1ENR |= (RCC_AHB1ENR_DMA1EN))
// #define __DMA2_CLK_ENABLE()          (RCC->AHB1ENR |= (RCC_AHB1ENR_DMA2EN))
bool dma1_clock_enabled(void) 
{	
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

// bool dma_handle_installed(); hard to test, not finished yet
// bool dma_state_is_ready(); no need for a new function

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

bool uart_dma_linked(UART_HandleTypeDef* huart)
{
	if (huart->hdmatx == 0) return false;
	if (huart->hdmatx->Parent == 0) return false;
	if (huart->hdmatx->Parent != huart) return false;
	
	if (huart->hdmarx == 0) return false;
	if (huart->hdmarx->Parent == 0) return false;
	if (huart->hdmarx->Parent != huart) return false;	
	
	return true;
}



/*** this group is used to test mspinit ***/
static GPIO_InitTypeDef gpio_init_usart2_pd5_pd6 =
{
	.Pin = GPIO_PIN_5|GPIO_PIN_6,
	.Mode = GPIO_MODE_AF_PP,
	.Pull = GPIO_NOPULL,
	.Speed = GPIO_SPEED_LOW,
	.Alternate = GPIO_AF7_USART2,
};

static UARTEX_HandleTypeDef huartex2_pd5_pd6 = 
{
	.gpiox = GPIOD,
	.gpio_init = &gpio_init_usart2_pd5_pd6,
	.huart = 
	{
		.Instance = USART2,
		.Init = 
		{
			.BaudRate = 115200,
			.WordLength = UART_WORDLENGTH_8B,
			.StopBits = UART_STOPBITS_1,
			.Parity = UART_PARITY_NONE,
			.Mode = UART_MODE_TX_RX,
			.HwFlowCtl = UART_HWCONTROL_NONE,
			.OverSampling = UART_OVERSAMPLING_16,
		},
	},
};


static UARTEX_HandleTypeDef huartex2_dma;

static DMA_HandleTypeDef hdma_rx_uart2 =
{
	.Instance = DMA1_Stream5,
	.Init = 
	{
		.Channel = DMA_CHANNEL_4,
    .Direction = DMA_PERIPH_TO_MEMORY,
    .PeriphInc = DMA_PINC_DISABLE,
    .MemInc = DMA_MINC_ENABLE,
    .PeriphDataAlignment = DMA_PDATAALIGN_BYTE,
    .MemDataAlignment = DMA_MDATAALIGN_BYTE,
    .Mode = DMA_NORMAL,
    .Priority = DMA_PRIORITY_LOW,
    .FIFOMode = DMA_FIFOMODE_DISABLE,
	},
	.Parent = &huartex2_dma.huart,	/* statically linked */
};


static DMA_HandleTypeDef hdma_tx_uart2 = 
{
	.Instance = DMA1_Stream6,
	.Init = 
	{
		.Channel = DMA_CHANNEL_4,
		.Direction = DMA_MEMORY_TO_PERIPH,
		.PeriphInc = DMA_PINC_DISABLE,
		.MemInc = DMA_MINC_ENABLE,
		.PeriphDataAlignment = DMA_PDATAALIGN_BYTE,
		.MemDataAlignment = DMA_MDATAALIGN_BYTE,
		.Mode = DMA_NORMAL,
		.Priority = DMA_PRIORITY_LOW,
		.FIFOMode = DMA_FIFOMODE_DISABLE,
	},
	.Parent = &huartex2_dma.huart,	/* statically linked */
};

static IRQ_ConfigTypeDef dmatx_irq_config =
{
	.irqn = DMA1_Stream6_IRQn,
};

static IRQ_ConfigTypeDef dmarx_irq_config = 
{
	.irqn = DMA1_Stream5_IRQn,
};

static IRQ_ConfigTypeDef uart2_irq_config =
{
	.irqn = USART2_IRQn,
};

static UARTEX_HandleTypeDef huartex2_dma =
{
	.gpiox = GPIOD,
	.gpio_init = &gpio_init_usart2_pd5_pd6,
	.huart = 
	{
		.Instance = USART2,
		.Init = 
		{
			.BaudRate = 115200,
			.WordLength = UART_WORDLENGTH_8B,
			.StopBits = UART_STOPBITS_1,
			.Parity = UART_PARITY_NONE,
			.Mode = UART_MODE_TX_RX,
			.HwFlowCtl = UART_HWCONTROL_NONE,
			.OverSampling = UART_OVERSAMPLING_16,
		},
		.hdmatx = &hdma_tx_uart2,	/** statically linked **/
		.hdmarx = &hdma_rx_uart2,
	},
	.hdma_tx = &hdma_tx_uart2,
	.hdma_rx = &hdma_rx_uart2,
	
	.dmarx_irq_config = &dmarx_irq_config,
	.dmatx_irq_config = &dmatx_irq_config,
	.uart_irq_config = &uart2_irq_config,
	.dma_clock = &DMA_Clock_Singleton,
};

TEST_GROUP(Usart_DMA_MspInit);

TEST_SETUP(Usart_DMA_MspInit)
{
//	__USART2_FORCE_RESET();
//	__DMA1_FORCE_RESET();
//	
//	__USART2_RELEASE_RESET();
//	__DMA1_FORCE_RESET();
}

TEST_TEAR_DOWN(Usart_DMA_MspInit)
{

}
	
/*** this test case must be executed exactly after the system initialization ***/
TEST(Usart_DMA_MspInit, UartClockShouldBeEnabled)
{
	UART_HandleTypeDef* h = &huartex2_pd5_pd6.huart;
	HAL_UART_MspInit(h);
	TEST_ASSERT_TRUE(HAL_UART_ClockIsEnabled(h->Instance));
}

/** don't use the misleading name, such as "GpioShouldeBeConfigured", since we do NOT fully test the gpio configuration. **/
/** we don't do fully test because we trust the code, we just make sure the function is called. **/
TEST(Usart_DMA_MspInit, GpioShouldBeNonInput)
{
	uint32_t pins = GPIO_PIN_5|GPIO_PIN_6;
	UART_HandleTypeDef* h = &huartex2_pd5_pd6.huart;
	HAL_GPIO_DeInit(GPIOD, pins);
	HAL_UART_MspInit(h);
	TEST_ASSERT_TRUE(gpio_modes_all_noninput(GPIOD, pins));
}

/** without this test, the HAL_DMA_Init function succeed anyway even if clock not enabled. **/
TEST(Usart_DMA_MspInit, RxDMAClockBitShouldBeSet)
{
	DMA_Clock_TypeDef dma_clock, *original;
	memset(&dma_clock, 0, sizeof(dma_clock));
	
	original = huartex2.dma_clock;
	huartex2.dma_clock = &dma_clock;
	
	HAL_UART_MspInit(&huartex2.huart);
	
	TEST_ASSERT_TRUE(DMA_Clock_Status(&dma_clock, huartex2.huart.hdmarx->Instance));

	huartex2.dma_clock = original;
}

TEST(Usart_DMA_MspInit, RxDMAShouldBeInitialized)
{
	HAL_DMA_DeInit(HDMA_RX_UART);
	HDMA_RX_UART->State = HAL_DMA_STATE_RESET;
	HAL_UART_MspInit(HUART_DMA);
	TEST_ASSERT_EQUAL(HAL_DMA_STATE_READY, HDMA_RX_UART->State);
}

TEST(Usart_DMA_MspInit, RxDMAIRQShouldBeEnabled)
{
	HAL_NVIC_DisableIRQ(HUARTEX_DMA->dmarx_irq_config->irqn);
	HAL_UART_MspInit(HUART_DMA);
	TEST_ASSERT_TRUE(irq_enabled(HUARTEX_DMA->dmarx_irq_config->irqn));
}

/**
// without this test, the HAL_DMA_Init function succeed anyway even if clock not enabled. 
TEST(Usart_DMA_MspInit, TxDMAClockBitShouldBeSet)
{
	DMA_Clock_TypeDef dma_clock, *original;
	memset(&dma_clock, 0, sizeof(dma_clock));
	
	original = huartex2.dma_clock;
	huartex2.dma_clock = &dma_clock;
	
	HAL_UART_MspInit(&huartex2.huart);
	
	TEST_ASSERT_TRUE(DMA_Clock_Status(&dma_clock, huartex2.huart.hdmatx->Instance));

	huartex2.dma_clock = original;
}
**/

TEST(Usart_DMA_MspInit, TxDMAShouldBeInitialized)
{
	HAL_DMA_DeInit(HDMA_TX_UART);
	HDMA_RX_UART->State = HAL_DMA_STATE_RESET;
	HAL_UART_MspInit(HUART_DMA);
	TEST_ASSERT_EQUAL(HAL_DMA_STATE_READY, HDMA_TX_UART->State);
}

TEST(Usart_DMA_MspInit, TxDMAIRQShouldBeEnabled)
{
	HAL_NVIC_DisableIRQ(HUARTEX_DMA->dmatx_irq_config->irqn);
	HAL_UART_MspInit(HUART_DMA);
	TEST_ASSERT_TRUE(irq_enabled(HUARTEX_DMA->dmatx_irq_config->irqn));
}

TEST(Usart_DMA_MspInit, UARTIRQShouldBeEnabled)
{
	HAL_NVIC_DisableIRQ(USART2_IRQn);
	HAL_UART_MspInit(HUART_DMA);
	TEST_ASSERT_TRUE(irq_enabled(USART2_IRQn));
}

TEST_GROUP_RUNNER(Usart_DMA_MspInit)
{
	RUN_TEST_CASE(Usart_DMA_MspInit, UartClockShouldBeEnabled);
	RUN_TEST_CASE(Usart_DMA_MspInit, GpioShouldBeNonInput);
	RUN_TEST_CASE(Usart_DMA_MspInit, RxDMAShouldBeInitialized);
	RUN_TEST_CASE(Usart_DMA_MspInit, RxDMAIRQShouldBeEnabled);
	RUN_TEST_CASE(Usart_DMA_MspInit, TxDMAShouldBeInitialized);
	RUN_TEST_CASE(Usart_DMA_MspInit, TxDMAIRQShouldBeEnabled);
	RUN_TEST_CASE(Usart_DMA_MspInit, UARTIRQShouldBeEnabled);
}


/******************************************************************************
	The following cases are Learning Tests

	USART2 tx/rx short required to run these tests.

******************************************************************************/

#ifdef HUART_DMA
#undef HUART_DMA
#endif

extern UARTEX_HandleTypeDef	huartex2;

#define HUART_DMA	(&huartex2.huart)

TEST_GROUP(LT_Usart_DMA_TxRx);

TEST_SETUP(LT_Usart_DMA_TxRx)
{
	// HAL_UART_Init(&huartex2_dma.huart);
	HAL_UART_Init(HUART_DMA);
}

TEST_TEAR_DOWN(LT_Usart_DMA_TxRx)
{
	// HAL_UART_DMAStop(HUART_DMA);
	// HAL_UART_DeInit(HUART_DMA);
	HAL_UART_DMAStop(HUART_DMA);
	HAL_UART_DeInit(HUART_DMA);
}

/** reuse this op to demonstrate the usart is fine after certain operation **/
void loopBackSixBytesDMA(void) {
		
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

/** the very basic send and recv test **/
TEST(LT_Usart_DMA_TxRx, LoopBackSixBytesTwice)
{
	loopBackSixBytesDMA();
	loopBackSixBytesDMA();
}

TEST(LT_Usart_DMA_TxRx, ReceiveMultipleBytesInWaitStopCheckManner)
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

TEST(LT_Usart_DMA_TxRx, ReceiveMultipleBytesInSuspendResumeManner)
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
	snprintf(tx, sizeof(tx), "i love test driven development.");
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
		
		if (HAL_GetTick() - timestamp > 100) {
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

TEST(LT_Usart_DMA_TxRx, ReceiveMultipleBytesAfterRxOverflowByZero)
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

TEST(LT_Usart_DMA_TxRx, ReceiveMultipleBytesAfterRxOverflowByOne)
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

TEST(LT_Usart_DMA_TxRx, ReceiveMultipleBytesAfterRxOverflowByTwo)
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


TEST_GROUP_RUNNER(LT_Usart_DMA_TxRx)
{
	RUN_TEST_CASE(LT_Usart_DMA_TxRx, LoopBackSixBytesTwice);
	RUN_TEST_CASE(LT_Usart_DMA_TxRx, ReceiveMultipleBytesInWaitStopCheckManner);
	RUN_TEST_CASE(LT_Usart_DMA_TxRx, ReceiveMultipleBytesInSuspendResumeManner);
	RUN_TEST_CASE(LT_Usart_DMA_TxRx, ReceiveMultipleBytesAfterRxOverflowByZero);
	RUN_TEST_CASE(LT_Usart_DMA_TxRx, ReceiveMultipleBytesAfterRxOverflowByOne);
	RUN_TEST_CASE(LT_Usart_DMA_TxRx, ReceiveMultipleBytesAfterRxOverflowByTwo);
}



