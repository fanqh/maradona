#include <string.h>
#include "stm32f4xx_hal.h"
#include "dma.h"
#include "gpio.h"
#include "usart.h"

static GPIOEX_TypeDef rxpin_usart3 =
{
	.instance = GPIOD,
	.init =
	{
		.Pin = GPIO_PIN_9,
		.Mode = GPIO_MODE_AF_PP,
		.Pull = GPIO_NOPULL,
		.Speed = GPIO_SPEED_LOW,
		.Alternate = GPIO_AF7_USART2,			
	},
	.clk = &GPIO_ClockProvider,
};

static GPIOEX_TypeDef txpin_usart3 = 
{
	.instance = GPIOD,
	.init =
	{
		.Pin = GPIO_PIN_8,
		.Mode = GPIO_MODE_AF_PP,
		.Pull = GPIO_NOPULL,
		.Speed = GPIO_SPEED_LOW,
		.Alternate = GPIO_AF7_USART2,			
	},
	.clk = &GPIO_ClockProvider,
};

static UARTEX_HandleTypeDef huartex3 = 
{	
	.rxpin = &rxpin_usart3,
	.txpin = &txpin_usart3,
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

static void RunAllTests(void)
{
	RUN_TEST_GROUP(GPIO_All);
	RUN_TEST_GROUP(IRQ_All);
	RUN_TEST_GROUP(DMA_All);
	RUN_TEST_GROUP(UART_All_HI);
	RUN_TEST_GROUP(UART_All_HD);

	RUN_TEST_GROUP(UsartIO_DMA);
	
	RUN_TEST_GROUP(MSP);
}

void board_init(void)
{
  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* Configure the system clock */
  SystemClock_Config();
	
  /* Initialize all configured peripherals */
  HAL_UART_Init(&huartex3.huart);	
	UnityMain(1, 0, RunAllTests);
}

void board_main(void)
{	
	while(1){};
}

void SysTick_Handler(void)
{
  HAL_IncTick();
  HAL_SYSTICK_IRQHandler();
}

/**
* @brief This function handles DMA1 Stream5 global interrupt.
*/




