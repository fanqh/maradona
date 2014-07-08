/**
  ******************************************************************************
  * File Name          : gpio.c
  * Date               : 11/06/2014 12:17:27
  * Description        : This file provides code for the configuration
  *                      of all used GPIO pins.
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
#include "gpio.h"


GPIO_ClockTypeDef	GPIOA_Clock_Singleton = {.instance = GPIOA, };
GPIO_ClockTypeDef	GPIOB_Clock_Singleton = {.instance = GPIOB, };
GPIO_ClockTypeDef	GPIOC_Clock_Singleton = {.instance = GPIOC, };
GPIO_ClockTypeDef	GPIOD_Clock_Singleton = {.instance = GPIOD, };
GPIO_ClockTypeDef	GPIOE_Clock_Singleton = {.instance = GPIOE, };
GPIO_ClockTypeDef	GPIOF_Clock_Singleton = {.instance = GPIOF, };
GPIO_ClockTypeDef	GPIOG_Clock_Singleton = {.instance = GPIOG, };
GPIO_ClockTypeDef	GPIOH_Clock_Singleton = {.instance = GPIOH, };

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/*----------------------------------------------------------------------------*/
/* Configure GPIO                                                             */
/*----------------------------------------------------------------------------*/
/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/** Configure pins as 
        * Analog 
        * Input 
        * Output
        * EVENT_OUT
        * EXTI
*/
//void MX_GPIO_Init(void)
//{

//  /* GPIO Ports Clock Enable */
//  __GPIOH_CLK_ENABLE();
//  __GPIOD_CLK_ENABLE();

//}

void	GPIOEX_Init(GPIOEX_TypeDef* gpioex)
{
	GPIO_Clock_Get(gpioex->clk, gpioex->init.Pin);
	HAL_GPIO_Init(gpioex->instance, &gpioex->init);
	gpioex->state = GPIOEX_STATE_SET;
}

void 	GPIOEX_DeInit(GPIOEX_TypeDef* gpioex)
{
	HAL_GPIO_DeInit(gpioex->instance, gpioex->init.Pin);
	GPIO_Clock_Put(gpioex->clk, gpioex->init.Pin);
	gpioex->state = GPIOEX_STATE_RESET;
}

void GPIO_Clock_Get(GPIO_ClockTypeDef* clk, uint32_t Pin) 
{
	clk->bits |= Pin;
	clk->bits &= 0x0000FFFF;
	
	if (clk->instance == GPIOA)
	{
		__GPIOA_CLK_ENABLE();
	}
	else if (clk->instance == GPIOB)
	{
		__GPIOB_CLK_ENABLE();
	}
	else if (clk->instance == GPIOC)
	{
		__GPIOC_CLK_ENABLE();
	}
	else if (clk->instance == GPIOD)
	{
		__GPIOD_CLK_ENABLE();
	}
	else if (clk->instance == GPIOE)
	{
		__GPIOE_CLK_ENABLE();
	}
	else if (clk->instance == GPIOF)
	{
		__GPIOF_CLK_ENABLE();
	}
	else if (clk->instance == GPIOG)
	{
		__GPIOG_CLK_ENABLE();
	}
	else if (clk->instance == GPIOH)
	{
		__GPIOH_CLK_ENABLE();
	}
	else {
		assert_param(false);
	}
}


void GPIO_Clock_Put(GPIO_ClockTypeDef* clk, uint32_t Pin)
{
	clk->bits &= ~Pin;
	clk->bits &= 0x0000FFFF;
	
	if (clk->instance == GPIOA)
	{
		__GPIOA_CLK_DISABLE();
	}
	else if (clk->instance == GPIOB)
	{
		__GPIOB_CLK_DISABLE();
	}
	else if (clk->instance == GPIOC)
	{
		__GPIOC_CLK_DISABLE();
	}
	else if (clk->instance == GPIOD)
	{
		__GPIOD_CLK_DISABLE();
	}
	else if (clk->instance == GPIOE)
	{
		__GPIOE_CLK_DISABLE();
	}
	else if (clk->instance == GPIOF)
	{
		__GPIOF_CLK_DISABLE();
	}
	else if (clk->instance == GPIOG)
	{
		__GPIOG_CLK_DISABLE();
	}
	else if (clk->instance == GPIOH)
	{
		__GPIOH_CLK_DISABLE();
	}
	else {
		assert_param(false);
	}	
}

bool GPIO_Clock_Status(GPIO_ClockTypeDef* clk, uint32_t Pin)
{
	return (clk->bits & Pin) ? true : false;
}

/**
  * @}
  */

/**
  * @}
  */

/************************ Defaults ********************************************/

const GPIOEX_TypeDef	PC6_As_Uart6Tx_Default =
{
	.instance = GPIOC,
	.init =
	{
		.Pin = GPIO_PIN_6,
		.Mode = GPIO_MODE_AF_PP,
		.Pull = GPIO_NOPULL,
		.Speed = GPIO_SPEED_LOW,
		.Alternate = GPIO_AF8_USART6,			
	},
	.clk = &GPIOC_Clock_Singleton,			// don't forget this! bug!
};

const GPIOEX_TypeDef	PD6_As_Uart2Rx_Default = 
{
	.instance = GPIOD,
	.init =
	{
		.Pin = GPIO_PIN_6,
		.Mode = GPIO_MODE_AF_PP,
		.Pull = GPIO_NOPULL,
		.Speed = GPIO_SPEED_LOW,
		.Alternate = GPIO_AF7_USART2,			
	},
	.clk = &GPIOD_Clock_Singleton,
};

const GPIOEX_TypeDef	PD5_As_Uart2Tx_Default =
{
	.instance = GPIOD,
	.init =
	{
		.Pin = GPIO_PIN_5,
		.Mode = GPIO_MODE_AF_PP,
		.Pull = GPIO_NOPULL,
		.Speed = GPIO_SPEED_LOW,
		.Alternate = GPIO_AF7_USART2,			
	},
	.clk = &GPIOD_Clock_Singleton,		
};

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
