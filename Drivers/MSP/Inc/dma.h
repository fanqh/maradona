#ifndef __DMA_H_
#define __DMA_H_

#include <stdbool.h>
#include "stm32f4xx_hal.h"
#include "irq.h"

typedef struct {
	
	uint8_t	dma1;
	uint8_t	dma2;
	
} DMA_ClockTypeDef;

typedef struct {
	
	DMA_HandleTypeDef* handle[8];
	
} DMA_IRQ_HandlerTypeDef;

typedef enum {
	DMAEX_HANDLE_STATE_RESET = 0,
	DMAEX_HANDLE_STATE_SET,
} DMAEX_HandleStateTypeDef;

typedef struct {

	DMA_ClockTypeDef					*clk;			// reference to dma clock resource manager
	
	DMA_HandleTypeDef					hdma;			// dma handle
	
	bool											useIRQ;		// if IRQ is used.
	IRQ_HandleTypeDef					hirq;			// IRQ handle
	
	DMAEX_HandleStateTypeDef	state;		// the state of this struct, SET or RESET
	
} DMAEX_HandleTypeDef;

/** please use these singletons/globals in DI pattern **/
extern DMA_ClockTypeDef 					DMA_Clock_Singleton;

/*********** exported function ******************/
#define DMA_CHANNEL_TO_INDEX(channel)						((uint32_t)channel >> 25)

/** the copy constructor **/
// void 	DMAEX_Clone(DMAEX_HandleTypeDef* dst, const DMAEX_HandleTypeDef* src);

void 	DMAEX_Init(DMAEX_HandleTypeDef* dmaex);
void	DMAEX_DeInit(DMAEX_HandleTypeDef* dmaex);

int 	DMA_Stream_Instance_To_Index(DMA_Stream_TypeDef* stream);
void 	DMA_Clock_Get(DMA_ClockTypeDef* dma_clock, DMA_Stream_TypeDef* stream);
void 	DMA_Clock_Put(DMA_ClockTypeDef* dma_clock, DMA_Stream_TypeDef* stream);
bool 	DMA_Clock_Status(DMA_ClockTypeDef* dma_clock, DMA_Stream_TypeDef* stream);

/*********** template handle ********************/
const extern DMAEX_HandleTypeDef DMAEX_Handle_Uart2Rx_Default;
const extern DMAEX_HandleTypeDef DMAEX_Handle_Uart2Tx_Default;

#endif

