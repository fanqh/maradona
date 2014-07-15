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
	 
struct UARTEX_HandleStruct;
typedef struct UARTEX_HandleStruct UARTEX_HandleTypeDef;	 
	 
typedef struct 
{
	HAL_StatusTypeDef (*init)(UARTEX_HandleTypeDef *hue);
	HAL_StatusTypeDef (*deinit)(UARTEX_HandleTypeDef *hue);
	HAL_StatusTypeDef (*send)(UARTEX_HandleTypeDef *hue, uint8_t *pData, uint16_t Size);
	HAL_StatusTypeDef (*recv)(UARTEX_HandleTypeDef *hue, uint8_t *pData, uint16_t Size);
	HAL_StatusTypeDef (*swap)(UARTEX_HandleTypeDef* hue, uint8_t* buf, size_t size, uint32_t* m0ar, int* ndtr);

} UARTEX_Operations;

extern UARTEX_Operations UARTEX_Ops_Default;

struct UARTEX_HandleStruct 
{	
	GPIOEX_TypeDef					*rxpin;
	GPIOEX_TypeDef					*txpin;
	
	UART_HandleTypeDef 			huart;
	
	DMAEX_HandleTypeDef			*hdmaex_rx;
	DMAEX_HandleTypeDef			*hdmaex_tx;
	
	IRQ_HandleTypeDef				*hirq;
	
	UARTEX_Operations				*ops;
};

typedef struct UARTEX_HandleStruct UARTEX_HandleTypeDef;

UARTEX_HandleTypeDef*	 UARTEX_Handle_Ctor(USART_TypeDef						*uart,
																					const UART_InitTypeDef	*init,
																					GPIOEX_TypeDef					*rxpin, 		// DI
																					GPIOEX_TypeDef					*txpin, 		// DI		
																					DMAEX_HandleTypeDef			*hdmaex_rx,	// DI
																					DMAEX_HandleTypeDef			*hdmaex_tx,	// DI
																					IRQ_HandleTypeDef				*hirq,			// DI
																					UARTEX_Operations				*ops);

void UARTEX_Handle_Dtor(UARTEX_HandleTypeDef* handle);
																					

typedef struct 
{
	IRQ_HandlerObjectRegistryTypeDef* registry;
	DMA_ClockProviderTypeDef*					dma_clk;
	GPIO_ClockProviderTypeDef*				gpio_clk;
	
	UARTEX_Operations*								uart_ops;
	
} UARTEX_Handle_FactoryTypeDef;	

UARTEX_HandleTypeDef* UARTEX_Handle_FactoryCreate(	const UARTEX_Handle_FactoryTypeDef* factory,
																										const UART_HandleTypeDef* huart,						// instance + init
																										const GPIOEX_TypeDef*	rxpin,								// instance + init
																										const GPIOEX_TypeDef*	txpin,								// instance + init
																										const DMA_HandleTypeDef* hdmarx, 						// instance + init, optional
																										const IRQ_HandleTypeDef* hirq_dmarx,				// instance + init, optional
																										const DMA_HandleTypeDef* hdmatx,						// instance + init, optional
																										const IRQ_HandleTypeDef* hirq_dmatx,				// instance + init, optional
																										const IRQ_HandleTypeDef* hirq_uart);				// instance + init, optional

void UARTEX_Handle_FactoryDestroy(const UARTEX_Handle_FactoryTypeDef* factory, UARTEX_HandleTypeDef* h);																										
																										
/** utility **/
void HAL_UART_ClockEnable(USART_TypeDef* uart);
void HAL_UART_ClockDisable(USART_TypeDef* uart);
bool HAL_UART_ClockIsEnabled(USART_TypeDef* uart);

/** this function should be considered as and HAL extension, it has a lot of dirty works on hardware registers **/
HAL_StatusTypeDef HAL_UART_SwapRxDMABuffer(UART_HandleTypeDef* h, uint8_t* buf, size_t size, uint32_t* m0ar, int* ndtr);									
									
int UART_Instance_To_Index(USART_TypeDef* uart);
									
/*********************** Defaults *********************************************/


const extern UART_HandleTypeDef	UART_Handle_Uart2_Default;
const extern UARTEX_HandleTypeDef UARTEX_Handle_Uart2_Default;			

#ifdef __cplusplus
}
#endif
#endif /*__ usart_H */



/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
