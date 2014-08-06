#ifndef __DMA_H_
#define __DMA_H_

#include <stdbool.h>
#include "stm32f4xx_hal.h"
#include "irq.h"

typedef struct {
	
	uint8_t	dma1;
	uint8_t	dma2;
	
} DMA_ClockProviderTypeDef;

void 	DMA_Clock_Get(DMA_ClockProviderTypeDef* dma_clk, DMA_Stream_TypeDef* stream);
void 	DMA_Clock_Put(DMA_ClockProviderTypeDef* dma_clk, DMA_Stream_TypeDef* stream);
bool 	DMA_Clock_Status(DMA_ClockProviderTypeDef* dma_clk, DMA_Stream_TypeDef* stream);

/** please use these singletons/globals in DI pattern **/
extern DMA_ClockProviderTypeDef 					DMA_ClockProvider;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
typedef enum {
	
	DMAEX_HANDLE_STATE_RESET = 0,
	DMAEX_HANDLE_STATE_SET,
} DMAEX_HandleStateTypeDef;

typedef struct {
	/** see DMA_HandleTypeDef **/
  DMA_Stream_TypeDef         *Instance;		/*!< Register base address                  */
  DMA_InitTypeDef            Init;        /*!< DMA communication parameters           */ 	
} DMA_ConfigTypeDef;

typedef struct {
	
	DMA_HandleTypeDef					hdma;			// dma handle
	
	DMA_ClockProviderTypeDef	*clk;			// reference to dma clock resource manager
	IRQ_HandleTypeDef					*hirq;		// IRQ handle
	
	DMAEX_HandleStateTypeDef	state;		// the state of this struct, SET or RESET
	
} DMAEX_HandleTypeDef;

int DMAEX_Handle_Init(DMAEX_HandleTypeDef* h, DMA_Stream_TypeDef *stream, const DMA_InitTypeDef *init,
	DMA_ClockProviderTypeDef *clk, IRQ_HandleTypeDef *hirq);

int DMAEX_Handle_InitByConfig(DMAEX_HandleTypeDef* h, const DMA_ConfigTypeDef* config, 
	DMA_ClockProviderTypeDef *clk, IRQ_HandleTypeDef *hirq);

void 	DMAEX_HAL_Init(DMAEX_HandleTypeDef* dmaex);
void	DMAEX_HAL_DeInit(DMAEX_HandleTypeDef* dmaex);																								

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
																									
extern	const DMA_ConfigTypeDef		DMA_Uart2Rx_DefaultConfig;	
extern 	const DMA_ConfigTypeDef		DMA_Uart2Tx_DefaultConfig;																									
																									
const extern DMA_HandleTypeDef		DMA_Handle_Uart2Rx_Default;
const extern DMA_HandleTypeDef		DMA_Handle_Uart2Tx_Default;
const extern DMAEX_HandleTypeDef 	DMAEX_Handle_Uart2Rx_Default;
const extern DMAEX_HandleTypeDef 	DMAEX_Handle_Uart2Tx_Default;																									
																									

#endif

