#include <stdbool.h>
#include <string.h>
#include "stm32f4xx_hal.h"
#include "gpio.h"
#include "dma.h"
#include "usart.h"
#include "unity_fixture.h"

static DMA_HandleTypeDef 	hdma_usart2_rx =
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
};

static DMA_HandleTypeDef 	hdma_usart2_tx =
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
};

static GPIO_InitTypeDef		gpio_init_usart2 = 
{
	.Pin = GPIO_PIN_5|GPIO_PIN_6,
	.Mode = GPIO_MODE_AF_PP,
	.Pull = GPIO_NOPULL,
	.Speed = GPIO_SPEED_LOW,
	.Alternate = GPIO_AF7_USART2,
};

static UART_IRQTypeDef		irq_usart2 = 
{
	.nvic_priority_group = NVIC_PRIORITYGROUP_0,
	.irqn = USART2_IRQn,
};
	
UARTEX_HandleTypeDef huartex2 = 
{	
	.gpio_port = GPIOD,
	.gpio_init = &gpio_init_usart2,
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
		}
	},	
	
	.hdma_rx = &hdma_usart2_rx,
	.hdma_tx = &hdma_usart2_tx,
	.irq = &irq_usart2,
};

static GPIO_InitTypeDef		gpio_init_usart3 =
{
	.Pin = GPIO_PIN_8|GPIO_PIN_9,
	.Mode = GPIO_MODE_AF_PP,
	.Pull = GPIO_NOPULL,
	.Speed = GPIO_SPEED_LOW,
	.Alternate = GPIO_AF7_USART3,
};


static UARTEX_HandleTypeDef huartex3 = 
{	
	.gpio_port = GPIOD,
	.gpio_init = &gpio_init_usart3,
	.huart = 
	{
		.Instance = USART3,
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

static void SystemClock_Config(void)
{

  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_OscInitTypeDef RCC_OscInitStruct;

  __PWR_CLK_ENABLE();

  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 25;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  HAL_RCC_OscConfig(&RCC_OscInitStruct);

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_SYSCLK|RCC_CLOCKTYPE_PCLK1
                              |RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;
  HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5);

}

int unity_output_char(int a) 
{
	uint8_t chr = a;
	
	HAL_UART_Transmit(&huartex3.huart, &chr, 1, 10);
	return a;
}

TEST_GROUP(Null_Test);
TEST_SETUP(Null_Test){}
TEST_TEAR_DOWN(Null_Test){}
TEST(Null_Test, AssertTrue){	TEST_ASSERT_TRUE(1); }


static void RunAllTests(void)
{
	RUN_TEST_CASE(Null_Test, AssertTrue);
}

static char console_ok[] = "console is ok.\r\n";
static char hello[] = "hello";
static char world[10] = {0};

void board_init(void)
{
	
	bool loop_ok = false;
  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* Configure the system clock */
  SystemClock_Config();

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
	MX_DMA_Init();

	HAL_UART_Init(&huartex3.huart);
	HAL_UART_Transmit(&huartex3.huart, (uint8_t*)console_ok, strlen(console_ok), 10);
	
	HAL_UART_Init(&huartex2.huart);
	HAL_UART_Receive_DMA(&huartex2.huart, (uint8_t*)world, 10);
	HAL_UART_Transmit_DMA(&huartex2.huart, (uint8_t*)hello, strlen(hello));
	
	HAL_Delay(100);
	
	if (memcmp(hello, world, strlen(hello)) == 0) {
		loop_ok = true;
	}
	
	HAL_UART_DMAStop(&huartex2.huart);
	HAL_UART_DeInit(&huartex2.huart);
	
	if (loop_ok)
	{
		HAL_UART_Transmit(&huartex3.huart, (uint8_t*)"loop hello success\r\n", strlen("loop hello success\r\n"), 10);
	}
	else {
		HAL_UART_Transmit(&huartex3.huart, (uint8_t*)"loop hello fail\r\n", strlen("loop hello fail\r\n"), 10);
	}
	
	UnityMain(1, 0, RunAllTests);
}

void board_main(void)
{
	while(1){};
}

/*************************** interrupt handler *******************************/

/**
* @brief This function handles System tick timer.
*/
void SysTick_Handler(void)
{
  HAL_IncTick();
  HAL_SYSTICK_IRQHandler();
}

/**
* @brief This function handles DMA1 Stream6 global interrupt.
*/
void DMA1_Stream6_IRQHandler(void)
{
  HAL_NVIC_ClearPendingIRQ(DMA1_Stream6_IRQn);
  HAL_DMA_IRQHandler(&hdma_usart2_tx);
}

/**
* @brief This function handles DMA1 Stream5 global interrupt.
*/
void DMA1_Stream5_IRQHandler(void)
{
  HAL_NVIC_ClearPendingIRQ(DMA1_Stream5_IRQn);
  HAL_DMA_IRQHandler(&hdma_usart2_rx);
}

/**
* @brief This function handles USART2 global interrupt.
*/
void USART2_IRQHandler(void)
{
  HAL_NVIC_ClearPendingIRQ(USART2_IRQn);
  HAL_UART_IRQHandler(&huartex2.huart);
}




/************************************** end of file **************************/



