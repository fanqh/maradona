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
#include <string.h>
#include "errno_ex.h"
#include "gpio.h"

GPIO_ClockProviderTypeDef GPIO_ClockProvider = {0};

void	GPIOEX_HAL_Init(GPIOEX_TypeDef* gpioex)
{
	GPIO_Clock_Get(gpioex->clk, gpioex->instance, gpioex->init.Pin);
	HAL_GPIO_Init(gpioex->instance, &gpioex->init);
	gpioex->state = GPIOEX_STATE_SET;
}

void 	GPIOEX_HAL_DeInit(GPIOEX_TypeDef* gpioex)
{
	HAL_GPIO_DeInit(gpioex->instance, gpioex->init.Pin);
	GPIO_Clock_Put(gpioex->clk, gpioex->instance, gpioex->init.Pin);
	gpioex->state = GPIOEX_STATE_RESET;
}

void GPIO_Clock_Get(GPIO_ClockProviderTypeDef* clk, GPIO_TypeDef* gpiox, uint32_t Pin) 
{
	int index;
	uint16_t prev, curr;
	
	if (gpiox == GPIOA)
	{
		index = 0;
	}
	else if (gpiox == GPIOB)
	{
		index = 1;
	}
	else if (gpiox == GPIOC)
	{
		index = 2;
	}
	else if (gpiox == GPIOD)
	{
		index = 3;
	}
	else if (gpiox == GPIOE)
	{	
		index = 4;
	}
	else if (gpiox == GPIOF)
	{
		index = 5;
	}
	else if (gpiox == GPIOG)
	{
		index = 6;
	}
	else if (gpiox == GPIOH)
	{
		index = 7;
	}
	else 
	{
		return;
	}
	
	prev = clk->bits[index];
	curr = (clk->bits[index] |= Pin);		/**** need new case, get twice, put once, clock on ****/

	if (prev == 0 && curr != 0)
	{
		switch (index)
		{
			case 0:
				__GPIOA_CLK_ENABLE();
				break;
			case 1:
				__GPIOB_CLK_ENABLE();
				break;
			case 2:
				__GPIOC_CLK_ENABLE();
				break;
			case 3:
				__GPIOD_CLK_ENABLE();
				break;
			case 4:
				__GPIOE_CLK_ENABLE();
				break;
			case 5:
				__GPIOF_CLK_ENABLE();
				break;
			case 6:
				__GPIOG_CLK_ENABLE();
				break;
			case 7:
				__GPIOH_CLK_ENABLE();
				break;
			default:
				break;
		}
	}
}


void GPIO_Clock_Put(GPIO_ClockProviderTypeDef* clk, GPIO_TypeDef* gpiox, uint32_t Pin)
{
	int index;
	uint16_t prev, curr;	
	
	if (gpiox == GPIOA)
	{
		index = 0;
	}
	else if (gpiox == GPIOB)
	{
		index = 1;
	}
	else if (gpiox == GPIOC)
	{
		index = 2;
	}
	else if (gpiox == GPIOD)
	{
		index = 3;
	}
	else if (gpiox == GPIOE)
	{	
		index = 4;
	}
	else if (gpiox == GPIOF)
	{
		index = 5;
	}
	else if (gpiox == GPIOG)
	{
		index = 6;
	}
	else if (gpiox == GPIOH)
	{
		index = 7;
	}
	else 
	{
		return;
	}
	
	prev = clk->bits[index];
	curr = (clk->bits[index] &= ~Pin);

	if (prev != 0 && curr == 0)
	{
		switch (index)
		{
			case 0:
				__GPIOA_CLK_DISABLE();
				break;
			case 1:
				__GPIOB_CLK_DISABLE();
				break;
			case 2:
				__GPIOC_CLK_DISABLE();
				break;
			case 3:
				__GPIOD_CLK_DISABLE();
				break;
			case 4:
				__GPIOE_CLK_DISABLE();
				break;
			case 5:
				__GPIOF_CLK_DISABLE();
				break;
			case 6:
				__GPIOG_CLK_DISABLE();
				break;
			case 7:
				__GPIOH_CLK_DISABLE();
				break;
			default:
				break;
		}
	}
}

bool GPIO_Clock_Status(GPIO_ClockProviderTypeDef* clk, GPIO_TypeDef* gpiox, uint32_t Pin)
{
	int index;
	
	if (gpiox == GPIOA)
	{
		index = 0;
	}
	else if (gpiox == GPIOB)
	{
		index = 1;
	}
	else if (gpiox == GPIOC)
	{
		index = 2;
	}
	else if (gpiox == GPIOD)
	{
		index = 3;
	}
	else if (gpiox == GPIOE)
	{	
		index = 4;
	}
	else if (gpiox == GPIOF)
	{
		index = 5;
	}
	else if (gpiox == GPIOG)
	{
		index = 6;
	}
	else if (gpiox == GPIOH)
	{
		index = 7;
	}
	else 
	{
		return false;
	}	
	
	return (clk->bits[index] & Pin) ? true : false;
}

int GPIOEX_Init(GPIOEX_TypeDef* ge, GPIO_TypeDef* gpiox, const GPIO_InitTypeDef* init, GPIO_ClockProviderTypeDef* clk)
{
	if (ge == NULL || gpiox == NULL || init == NULL || clk == NULL)
		return -EINVAL;
		
	memset(ge, 0, sizeof(GPIOEX_TypeDef));
	
	ge->instance = gpiox;
	memmove(&ge->init, init, sizeof(GPIO_InitTypeDef));
	ge->clk = clk;
	ge->state = GPIOEX_STATE_RESET;
	
	return 0;
}

int GPIOEX_InitByConfig(GPIOEX_TypeDef* ge, const GPIO_ConfigTypeDef* config, GPIO_ClockProviderTypeDef* clk)
{
	return GPIOEX_Init(ge, config->instance, &config->init, clk);
}

/************************ Defaults ********************************************/

const GPIO_ConfigTypeDef	PC6_As_Uart6Tx_DefaultConfig =
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
};


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
	.clk = &GPIO_ClockProvider,			// don't forget this! bug!
};

const GPIO_ConfigTypeDef	PD6_As_Uart2Rx_DefaultConfig = 
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
	.clk = &GPIO_ClockProvider,
};

const GPIO_ConfigTypeDef	PD5_As_Uart2Tx_DefaultConfig =
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
	.clk = &GPIO_ClockProvider,		
};

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
