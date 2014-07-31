#ifndef UART_IO_PRIVATE_H
#define UART_IO_PRIVATE_H

#include "configuration.h"
#include "stm32f4xx_hal.h"
#include "usart.h"
#include "uart_io.h"

/*** intercept hal api ***/
typedef struct {
	
	HAL_StatusTypeDef (*HAL_UART_Init)(UART_HandleTypeDef *huart);
	HAL_StatusTypeDef (*HAL_UART_Transmit_DMA)(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t Size);
	HAL_StatusTypeDef (*HAL_UART_Receive_DMA)(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t Size);
	HAL_StatusTypeDef (*UART_IO_RxFlipBuffer)(UART_HandleTypeDef* h, uint8_t* buf, size_t size, uint32_t* m0ar, int* ndtr);
	
} usart_api_t;

extern usart_api_t usart_apis;

/*** end of hal api interception ***/





//uart_device* UART_IO_GetHandle(int port);
//int UART_IO_SetHandle(int port, uart_device* handle);

/** this function should be considered as an extension to HAL, rather than a part of flip buffer uart io **/
HAL_StatusTypeDef UART_IO_RxFlipBuffer(UART_HandleTypeDef* h, uint8_t* buf, size_t size, uint32_t* m0ar, int* ndtr);

#endif // UART_IO_PRIVATE_H


