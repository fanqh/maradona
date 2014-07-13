#ifndef UART_IO_H
#define UART_IO_H

#include <stddef.h>
#include "configuration.h"
#include "stm32f4xx_hal.h"


/** forward declaration **/
typedef struct UART_IO_HandleStruct UART_IO_HandleTypeDef;


void										UART_IO_DeInitAll(void);
void 										UART_IO_Task(void);

UART_IO_HandleTypeDef* 	UART_IO_Open(int port);
int 										UART_IO_Read(UART_IO_HandleTypeDef* handle, char* buffer, size_t size);
int											UART_IO_FlushRx(UART_IO_HandleTypeDef* uio);
int 										UART_IO_Write(UART_IO_HandleTypeDef* handle, char* buffer, size_t size);
int											UART_IO_FlushTx(UART_IO_HandleTypeDef* uio);
int 										UART_IO_Close(UART_IO_HandleTypeDef* uio);

//UART_HandleTypeDef*			UART_IO_GetUartHandle(UART_IO_HandleTypeDef* h);
//UART_HandleTypeDef*			UART_IO_GetUartHandleByPort(int num);



#endif // UART_IO_H

