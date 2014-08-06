/**
  ******************************************************************************
  * File Name          : gpio.h
  * Date               : 11/06/2014 12:17:27
  * Description        : This file contains all the functions prototypes for 
  *                      the gpio  
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __gpio_H
#define __gpio_H
#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdbool.h>
#include "stm32f4xx_hal.h"
	 
typedef struct 
{
	uint16_t 				bits[8];	/** A to H **/
	
} GPIO_ClockProviderTypeDef;

typedef enum
{
	GPIOEX_STATE_RESET = 0,
	GPIOEX_STATE_SET,
} GPIOEX_StateTypeDef;

typedef struct
{
	GPIO_TypeDef										*instance;
	GPIO_InitTypeDef								init;
	
} GPIO_ConfigTypeDef;
	 
typedef struct
{
	GPIO_TypeDef  									*instance;
	GPIO_InitTypeDef								init;
	GPIO_ClockProviderTypeDef				*clk;
	
	GPIOEX_StateTypeDef							state;
	
} GPIOEX_TypeDef;	

extern GPIO_ClockProviderTypeDef GPIO_ClockProvider;

int GPIOEX_Init(GPIOEX_TypeDef* gpioex, GPIO_TypeDef* gpiox, const GPIO_InitTypeDef* init, GPIO_ClockProviderTypeDef* clk);
int GPIOEX_InitByConfig(GPIOEX_TypeDef* gpioex, const GPIO_ConfigTypeDef* config, GPIO_ClockProviderTypeDef* clk);

void	GPIOEX_HAL_Init(GPIOEX_TypeDef* gpioex);
void 	GPIOEX_HAL_DeInit(GPIOEX_TypeDef* gpioex);

/** in the following function, use only separate Pin defines, don't OR them **/
void 	GPIO_Clock_Get(GPIO_ClockProviderTypeDef* clk, GPIO_TypeDef* gpiox, uint32_t Pin);
void 	GPIO_Clock_Put(GPIO_ClockProviderTypeDef* clk, GPIO_TypeDef* gpiox, uint32_t Pin);
bool	GPIO_Clock_Status(GPIO_ClockProviderTypeDef* clk, GPIO_TypeDef* gpiox, uint32_t Pin);

const extern GPIO_ConfigTypeDef	PC6_As_Uart6Tx_DefaultConfig;
const extern GPIO_ConfigTypeDef	PD6_As_Uart2Rx_DefaultConfig;
const extern GPIO_ConfigTypeDef	PD5_As_Uart2Tx_DefaultConfig;

const extern GPIOEX_TypeDef	PC6_As_Uart6Tx_Default;
const extern GPIOEX_TypeDef	PD6_As_Uart2Rx_Default;
const extern GPIOEX_TypeDef	PD5_As_Uart2Tx_Default;


// void MX_GPIO_Init(void);
#ifdef __cplusplus
}
#endif
#endif /*__ pinoutConfig_H */

/**
  * @}
  */

/**
  * @}
  */



/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
