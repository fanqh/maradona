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

void HAL_UART_MspInit(UART_HandleTypeDef* huart)
{
	UARTEX_HandleTypeDef* hue = container_of(huart, UARTEX_HandleTypeDef, huart);
	
	enable_usart_clock(huart->Instance);
	HAL_GPIO_Init(hue->gpio_port, hue->gpio_init);
	
	if (hue->hdma_rx)
	{
		HAL_DMA_Init(hue->hdma_rx); 
		__HAL_LINKDMA(&hue->huart,hdmarx,*hue->hdma_rx);
	}
	
	if (hue->hdma_tx)
	{
		HAL_DMA_Init(hue->hdma_tx);
		__HAL_LINKDMA(&hue->huart,hdmatx,*hue->hdma_tx);
	}
	
	if (hue->irq)
	{
		HAL_NVIC_SetPriorityGrouping(hue->irq->nvic_priority_group);
		HAL_NVIC_SetPriority(hue->irq->irqn, hue->irq->preempt_priority, hue->irq->sub_priority);
		HAL_NVIC_EnableIRQ(hue->irq->irqn);
	}
}

void HAL_UART_MspDeInit(UART_HandleTypeDef* huart)
{
	UARTEX_HandleTypeDef* hue = container_of(huart, UARTEX_HandleTypeDef, huart);
	
	disable_usart_clock(huart->Instance);
	
  HAL_GPIO_DeInit(hue->gpio_port, hue->gpio_init->Pin);	
	
	if (hue->hdma_rx)
	{
		HAL_DMA_DeInit(huart->hdmarx);
	}
	
	if (hue->hdma_tx)
	{
		HAL_DMA_DeInit(huart->hdmatx);
	}

	if (hue->irq)
	{
    HAL_NVIC_DisableIRQ(hue->irq->irqn);	
	}
} 

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
