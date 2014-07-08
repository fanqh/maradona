/**
  ******************************************************************************
  * File Name          : USART.c
  * Date               : 11/06/2014 12:17:27
  * Description        : This file provides code for the configuration
  *                      of the USART instances.
  ******************************************************************************
  *
  * COPYRIGHT(c) 2014 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include <stdbool.h>
#include <stdlib.h>
#include "usart.h"
#include "gpio.h"

UART_IRQHandlerTypeDef	Uart_IRQ_Handler_Singleton = {0};


static void enable_usart_clock(USART_TypeDef* instance)
{
	if(instance == USART1)
	{
		__USART1_CLK_ENABLE();
	}
  else if(instance == USART2)
  {
    __USART2_CLK_ENABLE();
  }
  else if(instance == USART3)
  {
    __USART3_CLK_ENABLE();
  }
  else if(instance == UART4)
  {
    __UART4_CLK_ENABLE();
  }
  else if(instance == UART5)
  {
    __UART5_CLK_ENABLE();
  }
  else if(instance ==USART6)
  {
    __USART6_CLK_ENABLE();
  }
}

static void disable_usart_clock(USART_TypeDef* instance)
{
	if(instance == USART1)
	{
		__USART1_CLK_DISABLE();
	}
  else if(instance == USART2)
  {
    __USART2_CLK_DISABLE();
  }
  else if(instance == USART3)
  {
    __USART3_CLK_DISABLE();
  }
  else if(instance == UART4)
  {
    __UART4_CLK_DISABLE();
  }
  else if(instance == UART5)
  {
    __UART5_CLK_DISABLE();
  }
  else if(instance ==USART6)
  {
    __USART6_CLK_DISABLE();
  }	
}


int UART_Instance_To_Index(USART_TypeDef* instance)
{
	if (instance == USART1)
		return 1;
	if (instance == USART2)
		return 2;
	if (instance == USART3)
		return 3;
	if (instance == UART4)
		return 4;
	if (instance == UART5)
		return 5;
	if (instance == USART6)
		return 6;
	
	return 0;
}

void HAL_UART_MspInit(UART_HandleTypeDef* huart)
{
	UARTEX_HandleTypeDef* hue = container_of(huart, UARTEX_HandleTypeDef, huart);
	
	enable_usart_clock(huart->Instance);

	GPIOEX_Init(&hue->rxpin);
	if (hue->hdmaex_rx)
	{
		assert_param(hue->hdmaex_rx->clk);
		// assert_param(false == DMA_Clock_Status(hue->hdmaex_rx->clk, hue->hdmaex_rx->hdma.Instance));
		__HAL_LINKDMA(&hue->huart,hdmarx,hue->hdmaex_rx->hdma);
		DMAEX_Init(hue->hdmaex_rx);
	}

	GPIOEX_Init(&hue->txpin);
	if (hue->hdmaex_tx)
	{
		assert_param(hue->hdmaex_tx->clk);
		// assert_param(false == DMA_Clock_Status(hue->hdmaex_tx->clk, hue->hdmaex_tx->hdma.Instance));
		__HAL_LINKDMA(&hue->huart,hdmatx,hue->hdmaex_tx->hdma);
		DMAEX_Init(hue->hdmaex_tx);		
	}
		
	if (hue->useIRQ)
	{
		IRQ_Init(&hue->hirq, &hue->huart);
	}
}

void HAL_UART_MspDeInit(UART_HandleTypeDef* huart)
{
	
//	/*##-1- Reset peripherals ##################################################*/
//  USARTx_FORCE_RESET();
//  USARTx_RELEASE_RESET();

//  /*##-2- Disable peripherals and GPIO Clocks #################################*/
//  /* Configure UART Tx as alternate function  */
//  HAL_GPIO_DeInit(USARTx_TX_GPIO_PORT, USARTx_TX_PIN);
//  /* Configure UART Rx as alternate function  */
//  HAL_GPIO_DeInit(USARTx_RX_GPIO_PORT, USARTx_RX_PIN);
//   
//  /*##-3- Disable the DMA Streams ############################################*/
//  /* De-Initialize the DMA Stream associate to transmission process */
//  HAL_DMA_DeInit(&hdma_tx); 
//  /* De-Initialize the DMA Stream associate to reception process */
//  HAL_DMA_DeInit(&hdma_rx);
//  
//  /*##-4- Disable the NVIC for DMA ###########################################*/
//  HAL_NVIC_DisableIRQ(USARTx_DMA_TX_IRQn);
//  HAL_NVIC_DisableIRQ(USARTx_DMA_RX_IRQn);
	
	UARTEX_HandleTypeDef* hue = container_of(huart, UARTEX_HandleTypeDef, huart);
	
	if (hue->useIRQ)
	{
    IRQ_DeInit(&hue->hirq);
	}	
	
	if (hue->hdmaex_tx)
	{
		DMAEX_DeInit(hue->hdmaex_tx);
		hue->hdmaex_tx->hdma.Parent = 0;	// unlink
		hue->huart.hdmatx = 0;
	}	

	if (hue->hdmaex_rx)
	{
		DMAEX_DeInit(hue->hdmaex_rx);
		hue->hdmaex_rx->hdma.Parent = 0;	// unlink
		hue->huart.hdmarx = 0;
	}

	GPIOEX_DeInit(&hue->rxpin);
	GPIOEX_DeInit(&hue->txpin);
	
	disable_usart_clock(huart->Instance);
} 


void HAL_UART_ClockEnable(USART_TypeDef* uart)
{
/******************************************************************************
in rcc
#define __USART1_CLK_ENABLE()  (RCC->APB2ENR |= (RCC_APB2ENR_USART1EN))
#define __USART6_CLK_ENABLE()  (RCC->APB2ENR |= (RCC_APB2ENR_USART6EN))
#define __USART2_CLK_ENABLE()  (RCC->APB1ENR |= (RCC_APB1ENR_USART2EN))

in rcc_ex
#define __USART3_CLK_ENABLE()  (RCC->APB1ENR |= (RCC_APB1ENR_USART3EN))
#define __UART4_CLK_ENABLE()   (RCC->APB1ENR |= (RCC_APB1ENR_UART4EN))
#define __UART5_CLK_ENABLE()   (RCC->APB1ENR |= (RCC_APB1ENR_UART5EN))
******************************************************************************/
	if (uart == USART1)
	{
		__USART1_CLK_ENABLE();
	}
	else if (uart == USART2)
	{
		__USART2_CLK_ENABLE();
	}
	else if (uart == USART3)
	{
		__USART3_CLK_ENABLE();
	}
	else if (uart == UART4)
	{
		__UART4_CLK_ENABLE();
	}
	else if (uart == UART5)
	{
		__UART5_CLK_ENABLE();
	}
	else if (uart == USART6)
	{
		__USART6_CLK_ENABLE();
	}
	else
	{
		assert_param(false);
	}
}

void HAL_UART_ClockDisable(USART_TypeDef* uart)
{
	if (uart == USART1)
	{
		__USART1_CLK_DISABLE();
	}
	else if (uart == USART2)
	{
		__USART2_CLK_DISABLE();
	}
	else if (uart == USART3)
	{
		__USART3_CLK_DISABLE();
	}
	else if (uart == UART4)
	{
		__UART4_CLK_DISABLE();
	}
	else if (uart == UART5)
	{
		__UART5_CLK_DISABLE();
	}
	else if (uart == USART6)
	{
		__USART6_CLK_DISABLE();
	}
	else
	{
		assert_param(false);
	}
}

bool HAL_UART_ClockIsEnabled(USART_TypeDef* uart)
{
	if (uart == USART1)
	{
		return (RCC->APB2ENR & RCC_APB2ENR_USART1EN) ? true : false;
	}
	else if (uart == USART2)
	{
		return (RCC->APB1ENR & RCC_APB1ENR_USART2EN) ? true : false;
	}
	else if (uart == USART3)
	{
		return (RCC->APB1ENR & RCC_APB1ENR_USART3EN) ? true : false;
	}
	else if (uart == UART4)
	{
		return (RCC->APB1ENR & RCC_APB1ENR_UART4EN) ? true : false;
	}
	else if (uart == UART5)
	{
		return (RCC->APB1ENR & RCC_APB1ENR_UART5EN) ? true : false;
	}
	else if (uart == USART6)
	{
		return (RCC->APB2ENR & RCC_APB2ENR_USART6EN) ? true : false;
	}
	else 
	{
		assert_param(false);
	}
	
	return false;
}

/**
* @brief This function handles USART2 global interrupt.
*/
void USART2_IRQHandler(void)
{
  HAL_NVIC_ClearPendingIRQ(USART2_IRQn);
	
	if (Uart_IRQ_Handler_Singleton.handle[2])
	{
		HAL_UART_IRQHandler(Uart_IRQ_Handler_Singleton.handle[2]);
	}
}


void UARTEX_Clone(UARTEX_HandleTypeDef* dst, 							
									const UARTEX_HandleTypeDef* defaults,		
									const GPIOEX_TypeDef* rxpin,
									const GPIOEX_TypeDef* txpin,
									const IRQ_HandleTypeDef* irq,
									DMAEX_HandleTypeDef* rxdma,
									DMAEX_HandleTypeDef* txdma)
{
	(*dst) = (*defaults);
	
	if (rxpin) dst->rxpin = (*rxpin);
	if (txpin) dst->txpin = (*txpin);
	
	if (irq) {
		dst->useIRQ = true;
		dst->hirq = (*irq);
	}

	if (rxdma)
	{
		dst->hdmaex_rx = rxdma;
	}
	
	if (txdma)
	{
		dst->hdmaex_tx = txdma;
	}
}

/************************ Defaults ********************************************/

const IRQ_HandleTypeDef	IRQ_Handle_Uart2_Default =
{
	.irqn = USART2_IRQn,
	.hdata_store = &IRQ_HandlerDataStore_Singleton,
	.hdata = 0, // should be override		
};

const DMAEX_HandleTypeDef DMAEX_Handle_Uart2Rx_Default =
{
	.clk = &DMA_Clock_Singleton,
	.hdma =
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
	},
	
	.useIRQ = true,
	.hirq = 
	{
		.irqn = DMA1_Stream5_IRQn,
		.hdata_store = &IRQ_HandlerDataStore_Singleton,
		.hdata = 0, // THIS ONE should be link to the copied instance.hdma
	},
};

const DMAEX_HandleTypeDef DMAEX_Handle_Uart2Tx_Default = 
{
	.hdma =
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
	},

	.useIRQ = true,
	.hirq = 
	{
		.irqn = DMA1_Stream6_IRQn,
		.hdata_store = &IRQ_HandlerDataStore_Singleton,
		.hdata = 0,	// should be override
	},	
	
	.clk = &DMA_Clock_Singleton,
};



const UARTEX_HandleTypeDef UARTEX_Handle_Uart2_Default =
{	
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




/************************ (C) COPYRIGHT Actnova *****END OF FILE****/
