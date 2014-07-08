/**
  ******************************************************************************
  * File Name          : USART.h
  * Date               : 11/06/2014 12:17:28
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
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __usart_H
#define __usart_H
#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdbool.h>	 
#include "stm32f4xx_hal.h"
#include "gpio.h"
#include "dma.h"
	
struct UART_IRQHandlerStruct
{
	UART_HandleTypeDef*		handle[7];	/** the [0] element not used. **/
};

typedef struct UART_IRQHandlerStruct UART_IRQHandlerTypeDef;

struct UARTEX_HandleStruct 
{	
	GPIOEX_TypeDef					rxpin;
	GPIOEX_TypeDef					txpin;
	
	UART_HandleTypeDef 			huart;
	
	DMAEX_HandleTypeDef			*hdmaex_rx;
	DMAEX_HandleTypeDef			*hdmaex_tx;
	
	bool										useIRQ;
	IRQ_HandleTypeDef				hirq;
};

typedef struct UARTEX_HandleStruct UARTEX_HandleTypeDef;

extern UART_IRQHandlerTypeDef	Uart_IRQ_Handler_Singleton;


void UARTEX_Clone(UARTEX_HandleTypeDef* dst, 
									const UARTEX_HandleTypeDef* defaults,
									const GPIOEX_TypeDef* rxpin,
									const GPIOEX_TypeDef* txpin,
									const IRQ_HandleTypeDef* irq,
									DMAEX_HandleTypeDef* rxdma,
									DMAEX_HandleTypeDef* txdma);

/** utility **/
void HAL_UART_ClockEnable(USART_TypeDef* uart);
void HAL_UART_ClockDisable(USART_TypeDef* uart);
bool HAL_UART_ClockIsEnabled(USART_TypeDef* uart);

int UART_Instance_To_Index(USART_TypeDef* uart);
									
/*********************** Defaults *********************************************/

const extern IRQ_HandleTypeDef	IRQ_Handle_Uart2_Default;
									
const extern UARTEX_HandleTypeDef UARTEX_Handle_Uart2_Default;			

const extern DMAEX_HandleTypeDef DMAEX_Handle_Uart2Rx_Default;
const extern DMAEX_HandleTypeDef DMAEX_Handle_Uart2Tx_Default;									
									

#ifdef __cplusplus
}
#endif
#endif /*__ usart_H */



/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
