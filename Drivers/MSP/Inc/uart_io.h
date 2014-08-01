#ifndef UART_IO_H
#define UART_IO_H

#include <stddef.h>
#include "configuration.h"
#include "stm32f4xx_hal.h"
#include "usart.h"
#include "msp.h"
#include "devicefs.h"

/** 

This is a logical device

Of course it should USE the (hal) uart handle to communicate with hardware, 
but it is NOT neccessary to OWN this handle. By OWN we mean to take the 
responsibility for the configuration and construction. We separate this 
responsibility to uart handle factory (or should we call it the uart hal?), which will be fattier, and have
the configurability but the internal mechanism is opaque to this logical device.

Consider the uart handle factory as an email server or an tcp/ip stack. As a user, 
you have no need to know what happened inside, what is the limit, does hardware exist,
what is the configuration? is it configured by external file or hard coded?

Think about yourself as a USER, not an OWNER. Do one thing, Do it well!

**/
struct uart_device {
	
	struct device						dev;
	
	struct msp_factory			*msp;
	UARTEX_HandleTypeDef		*handle;
	
	char										*rbuf[2];		/** two pointers **/
	const int								rbuf_size;
	
	char										*rx_upper;
	char 										*rx_head;
	char										*rx_tail;	
	
	char										*tbuf[2];		/** two pointers **/
	const int 							tbuf_size;
	
	char										*tx_head;
	char										*tx_tail;
	
	int											open_count;
	
	/////////////////////////////////////////////////////////////////////////////
	// test data
	void										*testdata;								
};

/** tested **/
int 									UART_IO_Read(struct uart_device * udev, char* buffer, size_t size);
int 									UART_IO_Write(struct uart_device * udev, char* buffer, size_t size);

/** working **/
int 									UART_IO_Open(struct device * udev, struct file * file);

/** not tested yet **/
void									UART_IO_DeInitAll(void);
void 									UART_IO_Task(void);

int										UART_IO_FlushRx(struct uart_device* udev);

int										UART_IO_FlushTx(struct uart_device* uio);
int 									UART_IO_Close(struct uart_device* uio);

//UART_HandleTypeDef*			UART_IO_GetUartHandle(uart_device* h);
//UART_HandleTypeDef*			UART_IO_GetUartHandleByPort(int num);



#endif // UART_IO_H

