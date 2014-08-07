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
#include <string.h>
#include "errno_ex.h"
#include "usart.h"
#include "gpio.h"

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

	GPIOEX_HAL_Init(hue->rxpin);
	if (hue->hdmaex_rx)
	{
		assert_param(hue->hdmaex_rx->clk);
		// assert_param(false == DMA_Clock_Status(hue->hdmaex_rx->clk, hue->hdmaex_rx->hdma.Instance));
		__HAL_LINKDMA(&hue->huart,hdmarx,hue->hdmaex_rx->hdma);
		DMAEX_HAL_Init(hue->hdmaex_rx);
	}

	GPIOEX_HAL_Init(hue->txpin);
	if (hue->hdmaex_tx)
	{
		assert_param(hue->hdmaex_tx->clk);
		// assert_param(false == DMA_Clock_Status(hue->hdmaex_tx->clk, hue->hdmaex_tx->hdma.Instance));
		__HAL_LINKDMA(&hue->huart,hdmatx,hue->hdmaex_tx->hdma);
		DMAEX_HAL_Init(hue->hdmaex_tx);		
	}
		
	if (hue->hirq)
	{
		IRQ_HAL_Init(hue->hirq, &hue->huart);
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
	
	if (hue->hirq)
	{
    IRQ_HAL_DeInit(hue->hirq);
	}	
	
	if (hue->hdmaex_tx)
	{
		DMAEX_HAL_DeInit(hue->hdmaex_tx);
		hue->hdmaex_tx->hdma.Parent = 0;	// unlink
		hue->huart.hdmatx = 0;
	}	

	if (hue->hdmaex_rx)
	{
		DMAEX_HAL_DeInit(hue->hdmaex_rx);
		hue->hdmaex_rx->hdma.Parent = 0;	// unlink
		hue->huart.hdmarx = 0;
	}

	GPIOEX_HAL_DeInit(hue->rxpin);
	GPIOEX_HAL_DeInit(hue->txpin);
	
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

///////////////////////////////////////////////////////////////////////////////
// This function handles USART2 global interrupt.

void USART2_IRQHandler(void)
{
  HAL_NVIC_ClearPendingIRQ(USART2_IRQn);
	
	UART_HandleTypeDef* handle = IRQ_HandlerObject_Get(&IRQ_HandlerObjectRegistry, USART2_IRQn);
	
	if (handle)
	{
		HAL_UART_IRQHandler(handle);
	}
}

UARTEX_HandleTypeDef*	 UARTEX_Handle_Ctor(USART_TypeDef										*uart,
																					const UART_InitTypeDef					*init,
																					GPIOEX_TypeDef									*rxpin, 		// DI
																					GPIOEX_TypeDef									*txpin, 		// DI		
																					DMAEX_HandleTypeDef							*hdmaex_rx,	// DI
																					DMAEX_HandleTypeDef							*hdmaex_tx,	// DI
																					IRQ_HandleTypeDef								*hirq,			// DI
																					const struct UARTEX_Operations	*ops)
{
	UARTEX_HandleTypeDef* h;
	
	if (uart == NULL || init == NULL || rxpin == NULL || txpin == NULL || ops == NULL)
		return NULL;
	
	h = (UARTEX_HandleTypeDef*)malloc(sizeof(UARTEX_HandleTypeDef));
	if (!h) return NULL;
	
	memset(h, 0, sizeof(UARTEX_HandleTypeDef));
	
	h->rxpin = rxpin;
	h->txpin = txpin;
	h->huart.Instance = uart;
	memmove(&h->huart.Init, init, sizeof(UART_InitTypeDef));
	h->hdmaex_rx = hdmaex_rx;
	h->hdmaex_tx = hdmaex_tx;
	h->hirq = hirq;
	h->ops = *ops;
	
	return h;
}

int	UARTEX_Handle_Init( UARTEX_HandleTypeDef						*h,
												USART_TypeDef										*uart,
												const UART_InitTypeDef					*init,
												GPIOEX_TypeDef									*rxpin, 		// DI
												GPIOEX_TypeDef									*txpin, 		// DI		
												DMAEX_HandleTypeDef							*hdmaex_rx,	// DI
												DMAEX_HandleTypeDef							*hdmaex_tx,	// DI
												IRQ_HandleTypeDef								*hirq,			// DI
												const struct UARTEX_Operations	*ops)
{	
	if (h == NULL || (!IS_USART_INSTANCE(uart)) || init == NULL || rxpin == NULL || txpin == NULL || ops == NULL)
		return -EINVAL;
	
	/** all or none **/
	if (!((hdmaex_rx && hdmaex_tx && hirq) || (!hdmaex_rx && !hdmaex_tx && !hirq))) 
		return -EINVAL;
	
	memset(h, 0, sizeof(UARTEX_HandleTypeDef));
	
	h->rxpin = rxpin;
	h->txpin = txpin;
	h->huart.Instance = uart;
	memmove(&h->huart.Init, init, sizeof(UART_InitTypeDef));
	h->hdmaex_rx = hdmaex_rx;
	h->hdmaex_tx = hdmaex_tx;
	h->hirq = hirq;
	h->ops = *ops;

	return 0;
}

UARTEX_HandleTypeDef*	UARTEX_Handle_CtorByConfig(	const UART_ConfigTypeDef					*config,	
																									GPIOEX_TypeDef										*rxpin, 				// DI
																									GPIOEX_TypeDef										*txpin, 				// DI		
																									DMAEX_HandleTypeDef								*hdmaex_rx,			// DI
																									DMAEX_HandleTypeDef								*hdmaex_tx,			// DI
																									IRQ_HandleTypeDef									*hirq,					// DI
																									const struct UARTEX_Operations		*ops)
{
	return UARTEX_Handle_Ctor(config->Instance, &config->Init, rxpin, txpin, hdmaex_rx, hdmaex_tx, hirq, ops);
}

int	UARTEX_Handle_InitByConfig(	UARTEX_HandleTypeDef* 						h,
																const UART_ConfigTypeDef					*config,	
																GPIOEX_TypeDef										*rxpin, 				// DI
																GPIOEX_TypeDef										*txpin, 				// DI		
																DMAEX_HandleTypeDef								*hdmaex_rx,			// DI
																DMAEX_HandleTypeDef								*hdmaex_tx,			// DI
																IRQ_HandleTypeDef									*hirq,					// DI
																const struct UARTEX_Operations		*ops)
{
	if (config == NULL)
		return -EINVAL;
	
	return UARTEX_Handle_Init(h, config->Instance, &config->Init, rxpin, txpin, hdmaex_rx, hdmaex_tx, hirq, ops);
}

void UARTEX_Handle_Dtor(UARTEX_HandleTypeDef* handle)
{
	if (handle) free(handle);
}



/** do it anyway, considering we cannot know what is the previously set buffer size, dont make assumptios, leave the POLICY to the caller **/
HAL_StatusTypeDef HAL_UART_SwapRxDMABuffer(UART_HandleTypeDef* h, uint8_t* buf, size_t size, uint32_t* m0ar, int* ndtr) 
{
	
	// HAL_StatusTypeDef status;
	if (h == 0 || buf == 0 || size == 0 || size >= 65535) /** the NDTR accept 65535 at most **/
		return HAL_ERROR;

#if 0
			HAL_UART_STATE_RESET             = 0x00,    /*!< Peripheral is not yet Initialized                  */
			HAL_UART_STATE_READY             = 0x01,    /*!< Peripheral Initialized and ready for use           */
			HAL_UART_STATE_BUSY              = 0x02,    /*!< an internal process is ongoing                     */   
			HAL_UART_STATE_BUSY_TX           = 0x12,    /*!< Data Transmission process is ongoing               */ 
			HAL_UART_STATE_BUSY_RX           = 0x22,    /*!< Data Reception process is ongoing                  */
			HAL_UART_STATE_BUSY_TX_RX        = 0x32,    /*!< Data Transmission and Reception process is ongoing */  
fatal	HAL_UART_STATE_TIMEOUT           = 0x03,    /*!< Timeout state                                      */
n/a		HAL_UART_STATE_ERROR             = 0x04     /*!< Error                                              */  
#endif
	
	if (h->State == HAL_UART_STATE_BUSY)
		return HAL_BUSY;
	
	if (h->State == HAL_UART_STATE_TIMEOUT || h->State == HAL_UART_STATE_RESET || h->State == HAL_UART_STATE_ERROR)
		return HAL_TIMEOUT;
	
	/** for non-rx/tx state, this func takes no effect.*/
	HAL_UART_DMAPause(h);
	
	if (h->State == HAL_UART_STATE_READY || h->State == HAL_UART_STATE_BUSY_TX) {
		
		/** resume asap **/
		HAL_UART_DMAResume(h);
		
		if (m0ar)
			*m0ar = h->hdmarx->Instance->M0AR;
	
		if (ndtr) 
			*ndtr = __HAL_DMA_GET_COUNTER(h->hdmarx);
		
		return HAL_UART_Receive_DMA(h, buf, size);
	}
	
	__HAL_DMA_DISABLE_IT(h->hdmarx, DMA_IT_TC);
	__HAL_DMA_DISABLE(h->hdmarx);				/** this clear DMA_SxCR_EN bit**/	
	
	if (m0ar) 
		*m0ar = h->hdmarx->Instance->M0AR;

	if (ndtr) 
		*ndtr = __HAL_DMA_GET_COUNTER(h->hdmarx);

	h->hdmarx->Instance->M0AR = (uint32_t)buf;
	__HAL_DMA_SET_COUNTER(h->hdmarx, size);

	__HAL_DMA_CLEAR_FLAG(h->hdmarx, __HAL_DMA_GET_TC_FLAG_INDEX(h->hdmarx));
	__HAL_DMA_ENABLE(h->hdmarx);
	__HAL_DMA_ENABLE_IT(h->hdmarx, DMA_IT_TC);	
	HAL_UART_DMAResume(h);
		
	return HAL_OK;
}

///////////////////////////////////////////////////////////////////////////////
// thin wrappers, no test cases 
static HAL_StatusTypeDef UARTEX_Init(UARTEX_HandleTypeDef *hue)
{
	return HAL_UART_Init(&hue->huart);
}

static HAL_StatusTypeDef UARTEX_DeInit(UARTEX_HandleTypeDef *hue) 
{
	HAL_UART_DMAStop(&hue->huart);
	return HAL_UART_DeInit(&hue->huart);
}

static HAL_StatusTypeDef UARTEX_Send(UARTEX_HandleTypeDef *hue, uint8_t *pData, uint16_t Size)
{
	return HAL_UART_Transmit_DMA(&hue->huart, pData, Size);
}

static HAL_StatusTypeDef UARTEX_Recv(UARTEX_HandleTypeDef *hue, uint8_t *pData, uint16_t Size)
{
	return HAL_UART_Receive_DMA(&hue->huart, pData, Size);
}

static HAL_StatusTypeDef UARTEX_Swap(UARTEX_HandleTypeDef* hue, uint8_t* buf, size_t size, uint32_t* m0ar, int* ndtr)
{
	return HAL_UART_SwapRxDMABuffer(&hue->huart, buf, size, m0ar, ndtr);
}

const struct UARTEX_Operations UARTEX_Ops_DefaultConfig = 
{
	.init = UARTEX_Init,
	.deinit = UARTEX_DeInit,
	.send = UARTEX_Send,
	.recv = UARTEX_Recv,
	.swap = UARTEX_Swap,
};

///////////////////////////////////////////////////////////////////////////////
// Defauts
const UART_ConfigTypeDef	UART2_DefaultConfig = 
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
};	

const UART_HandleTypeDef	UART_Handle_Uart2_Default =
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



