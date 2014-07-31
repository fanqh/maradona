#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "errno_ex.h"
#include "configuration.h"
#include "stm32f4xx_hal.h"
#include "uart_io_private.h"

//#ifdef UNIT_TEST
//#define __CALL	usart_apis.
//#else
//#define __CALL
//#endif

//static UART_HandleTypeDef huart2_default = {
//  .Instance = USART2,
//  .Init.BaudRate = 115200,
//  .Init.WordLength = UART_WORDLENGTH_8B,
//  .Init.StopBits = UART_STOPBITS_1,
//  .Init.Parity = UART_PARITY_NONE,
//  .Init.Mode = UART_MODE_TX_RX,
//  .Init.HwFlowCtl = UART_HWCONTROL_NONE,
//  .Init.OverSampling = UART_OVERSAMPLING_16,
//};

/*
 *	declare external uart handles
 */
//#if (UART_IO_USE_USART1)
//extern UART_HandleTypeDef	huart1;
//#endif
//#if (UART_IO_USE_USART2)
//extern UART_HandleTypeDef huart2;
//static char uart2_rbuf[2][UART_IO_BUFFER_SIZE];
//static char uart2_tbuf[2][UART_IO_BUFFER_SIZE];
//static uart_device huio2 =
//{
//	.handle = &huart2,
//	.rbuf[0] = uart2_rbuf[0],
//	.rbuf[1] = uart2_rbuf[1],
//	.rx_upper = uart2_rbuf[1],
//	.rx_head = &uart2_rbuf[1][1],
//	.rx_tail = &uart2_rbuf[1][1],
//	
//	.tbuf[0] = uart2_tbuf[0],
//	.tbuf[1] = uart2_tbuf[1],

//	.tx_head = &uart2_tbuf[1][0],
//	.tx_tail = &uart2_tbuf[1][0],
//	
//	.port = 2
//};	
//#endif
//#if (UART_IO_USE_USART3)
//extern UART_HandleTypeDef huart3;
//static char uart3_rbuf[2][UART_IO_BUFFER_SIZE];
//static uart_device huio3 =
//{
//	.handle = &huart3,
//	.rbuf[0] = uart3_rbuf[0],
//	.rbuf[1] = uart3_rbuf[1],
//	.rx_upper = uart3_rbuf[1],
//	.rx_head = &uart3_rbuf[1][1],
//	.rx_tail = &uart3_rbuf[1][1],
//	.port = 3
//};	
//#endif
//#if (UART_IO_USE_USART4)
//extern UART_HandleTypeDef huart4;
//#endif
//#if (UART_IO_USE_USART5)
//extern UART_HandleTypeDef huart5;
//#endif
//#if (UART_IO_USE_USART6)
//extern UART_HandleTypeDef	huart6;
//#endif



/*
 *	This globals are required for UART_IO_Task, which starts Tx transmission
 *	if hardware ready.
 */
//static uart_device* uart_io_handles[7] = 
//{
//#if (UART_IO_USE_USART1)
//&huio1,
//#else	
//0,
//#endif
//	
//#if (UART_IO_USE_USART2)
//&huio2,
//#else
//0,
//#endif

//#if (UART_IO_USE_USART3)
//&huio3,
//#else
//0,
//#endif

//#if (UART_IO_USE_USART4)
//&huio4,
//#else
//0,
//#endif

//#if (UART_IO_USE_USART5)
//&huio5,
//#else
//0,
//#endif

//#if (UART_IO_USE_USART6)
//&huio6,
//#else
//0,
//#endif
//};

//uart_device* UART_IO_GetHandle(int port) 
//{
//	
//	if (port < 1 || port > 6)
//		return 0;
//	
//	return uart_io_handles[port];
//}

//int UART_IO_SetHandle(int port, uart_device* handle)
//{
//	if (port < 1 || port > 6)
//		return -1;
//	
//	uart_io_handles[port] = handle;
//	
//	return 0;
//}


/** do it anyway, considering we cannot know what is the previously set buffer size, dont make assumptios, leave the POLICY to the caller **/
HAL_StatusTypeDef UART_IO_RxFlipBuffer(UART_HandleTypeDef* h, uint8_t* buf, size_t size, uint32_t* m0ar, int* ndtr) 
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



/**
uart_device* UART_IO_Open(int port)
{
//	HAL_StatusTypeDef status;
	uart_device* huio;
	
	if (port < 1 || port > 6)
	{
		errno = EINVAL;
		return 0;
	}
	
	huio = UART_IO_GetHandle(port);
	if (huio == 0)
	{
		errno = ENODEV;
		return 0;
	}
	
	if (huio->handle->State != HAL_UART_STATE_RESET)
	{
		errno = EBUSY;
		return 0;
	}
	
	if (huio->handle == 0 || huio->rbuf[0] == 0 || huio->rbuf[1] == 0 || 
			huio->tbuf[0] == 0 || huio->tbuf[1] == 0)
	{
		errno = EINVAL;
		return 0;
	}
	
	__CALL HAL_UART_Init(huio->handle);
	__CALL HAL_UART_Receive_DMA(huio->handle, (uint8_t*)huio->rbuf[0], UART_IO_BUFFER_SIZE);
	
	huio->rx_upper = huio->rbuf[1];
	huio->rx_tail = &huio->rbuf[1][UART_IO_BUFFER_SIZE];
	huio->rx_head = huio->rx_tail;
	
	huio->tx_head = &huio->tbuf[1][0];
	huio->tx_tail = huio->tx_head;
	
	return huio;
}
**/



/*
 * This function read out of upper buffer first, then flip (fetch DMA buffer) and read again.
 * Flip at most once. The assumption is if there are data in either buffers, 
 * this function should not return 0
 *
 * When i/o error occurs, read won't return error immediately if the buffer 
 * is not empty. Instead it returns data in buffer first. 
 * When the function is called again, that is, the buffer is empty, 
 * it return -1 and set errno correspondingly..
 */
int UART_IO_Read(struct uart_device* h, char* buffer, size_t buffer_size)
{

	HAL_StatusTypeDef status;
	char* p = buffer;
	uint32_t m0ar;
	int ndtr, count = 0;

	/** validate parameters **/
	if (h == 0 || buffer == 0) {
		errno = EINVAL;
		return -1;
	}
	
	if (buffer_size == 0) {
		return 0;
	} 
	
	/** read data in upper buffer **/
	while (h->rx_head < h->rx_tail) {
		
		*p++ = *h->rx_head++;
		count++;

		if (count == buffer_size) {
			return count;
		}
	}
	
	status = (h->handle->ops.swap)(h->handle, (uint8_t*)h->rx_upper, UART_IO_BUFFER_SIZE, &m0ar, &ndtr);
	
	if (status == HAL_ERROR) {
		
		if (count) return count;
		
		errno = EINVAL;
		return -1;
	}
	
	if (status == HAL_BUSY) {
		
		if (count) return count;
		errno = EBUSY;
		return -1;
	}
	
	if (status == HAL_TIMEOUT) {
		
		if (count) return count;
		errno = EIO;
		return -1;
	}
	
	h->rx_upper = (char*)m0ar;
	h->rx_head = h->rx_upper;
	h->rx_tail = h->rx_head + UART_IO_BUFFER_SIZE - ndtr;

	/** read data in upper buffer, again **/
	while (h->rx_head < h->rx_tail) {
		
		*p++ = *h->rx_head++;
		count++;

		if (count == buffer_size) {
			return count;
		}
	}

	return count;
}



/*
 * This function write to upper buffer first, then flip if upper buffer full.
 * the assumption for this function is it must send data if there is room in 
 * upper buffers. ALL-OR-NONE is not acceptable for user must slice the long
 * string by themselves.
 */
int UART_IO_Write(struct uart_device* h, char* buffer, size_t size) {
	
	HAL_StatusTypeDef status;
	char* tx_end;
	int count = 0;
	
	if (h == 0 || buffer == 0) {
		
		errno = EINVAL;
		return -1;
	}
	
	if (size == 0)
		return 0;
	
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
	
	/** These error states cannot proceed **/
	if (h->handle->huart.State == HAL_UART_STATE_TIMEOUT ||
			h->handle->huart.State == HAL_UART_STATE_ERROR ||
			h->handle->huart.State == HAL_UART_STATE_RESET)
	{
		errno = EIO;
		return -1;
	}

	tx_end = &h->tx_head[UART_IO_BUFFER_SIZE];
	while (h->tx_tail < tx_end) {
		*h->tx_tail++ = *buffer++;
		count++;
		
		if (count == size) {
			
			// status = __CALL HAL_UART_Transmit_DMA(&h->handle->huart, (uint8_t*)h->tx_head, h->tx_tail - h->tx_head);
			status = h->handle->ops.send(h->handle, (uint8_t*)h->tx_head, h->tx_tail - h->tx_head);
			if (status == HAL_OK) {
				
				h->tx_head = (h->tx_head == h->tbuf[0]) ? h->tbuf[1] : h->tbuf[0];
				h->tx_tail = h->tx_head;
				return count;
			}
			
			return count;
		}
	}
	
	// status = __CALL HAL_UART_Transmit_DMA(&h->handle->huart, (uint8_t*)h->tx_head, h->tx_tail - h->tx_head);
	status = h->handle->ops.send(h->handle, (uint8_t*)h->tx_head, h->tx_tail - h->tx_head);
	if (status == HAL_OK)
	{
		h->tx_tail = h->tx_head = (h->tx_head == h->tbuf[0]) ? h->tbuf[1] : h->tbuf[0];
		
		tx_end = &h->tx_head[UART_IO_BUFFER_SIZE];
		while (h->tx_tail < tx_end) {
			*h->tx_tail++ = *buffer++;
			count++;
			
			if (count == size) {
				return count;
			}
		}
		
		return count;
	}
	
	return count;
}

int	UART_IO_Open(struct device * dev, struct file * filp)
{
	return 0;
}


#if 0

/*
 * Close
 */
int UART_IO_Close(uart_device* h) {
	
	if (0 == h) {
		return -1;
	}
	
	HAL_UART_DeInit(h->handle);
	uart_io_handles[h->port] = 0;
	free(h);
	
	return 0;
}

void UART_IO_Task(void) {
	
	int i;
	
	for (i = 1; i < 7; i++) {
		if (uart_io_handles[i]) {
			UART_IO_TxFlipBuffer(uart_io_handles[i]);
		}
	}
}

/*
 * Call this function right after (auto-gened) system init code
 * to get a "clean" open afterwards.
 */
void UART_IO_DeInitAll(void) {
	
	int i;
	
	for (i = 1; i < 7; i++) {
		if (uart_handles[i]) {
			HAL_UART_DeInit(uart_handles[i]);
		}
	}
}

UART_HandleTypeDef*			UART_IO_GetUartHandle(uart_device* h) {
	
	if (h == 0) return 0;
	
	return h->handle;
}

UART_HandleTypeDef*			UART_IO_GetUartHandleByPort(int num) {
	
	if (num < 1 || num > 6)
		return 0;
	
	return uart_handles[num];
}

#endif















