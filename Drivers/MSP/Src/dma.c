#include "stm32f4xx_hal.h"
#include "dma.h"

/** DMA Clock (Resource Manager) Singleton **/
DMA_ClockTypeDef 						DMA_Clock_Singleton = {0};


/******************************************************************************
#define DMA1                ((DMA_TypeDef *) DMA1_BASE)
#define DMA1_Stream0        ((DMA_Stream_TypeDef *) DMA1_Stream0_BASE)
#define DMA1_Stream1        ((DMA_Stream_TypeDef *) DMA1_Stream1_BASE)
#define DMA1_Stream2        ((DMA_Stream_TypeDef *) DMA1_Stream2_BASE)
#define DMA1_Stream3        ((DMA_Stream_TypeDef *) DMA1_Stream3_BASE)
#define DMA1_Stream4        ((DMA_Stream_TypeDef *) DMA1_Stream4_BASE)
#define DMA1_Stream5        ((DMA_Stream_TypeDef *) DMA1_Stream5_BASE)
#define DMA1_Stream6        ((DMA_Stream_TypeDef *) DMA1_Stream6_BASE)
#define DMA1_Stream7        ((DMA_Stream_TypeDef *) DMA1_Stream7_BASE)
#define DMA2                ((DMA_TypeDef *) DMA2_BASE)
#define DMA2_Stream0        ((DMA_Stream_TypeDef *) DMA2_Stream0_BASE)
#define DMA2_Stream1        ((DMA_Stream_TypeDef *) DMA2_Stream1_BASE)
#define DMA2_Stream2        ((DMA_Stream_TypeDef *) DMA2_Stream2_BASE)
#define DMA2_Stream3        ((DMA_Stream_TypeDef *) DMA2_Stream3_BASE)
#define DMA2_Stream4        ((DMA_Stream_TypeDef *) DMA2_Stream4_BASE)
#define DMA2_Stream5        ((DMA_Stream_TypeDef *) DMA2_Stream5_BASE)
#define DMA2_Stream6        ((DMA_Stream_TypeDef *) DMA2_Stream6_BASE)
#define DMA2_Stream7        ((DMA_Stream_TypeDef *) DMA2_Stream7_BASE)
******************************************************************************/

int DMA_Stream_Instance_To_Index(DMA_Stream_TypeDef* stream)
{
	if (stream == DMA1_Stream0 || stream == DMA2_Stream0)
		return 0;
	if (stream == DMA1_Stream1 || stream == DMA2_Stream1)
		return 1;
	if (stream == DMA1_Stream2 || stream == DMA2_Stream2)
		return 2;
	if (stream == DMA1_Stream3 || stream == DMA2_Stream3)
		return 3;
	if (stream == DMA1_Stream4 || stream == DMA2_Stream4)
		return 4;
	if (stream == DMA1_Stream5 || stream == DMA2_Stream5)
		return 5;
	if (stream == DMA1_Stream6 || stream == DMA2_Stream6)
		return 6;
	if (stream == DMA1_Stream7 || stream == DMA2_Stream7)
		return 7;
	
	return -1;
}

void 	DMAEX_Init(DMAEX_HandleTypeDef* dmaex)
{
	DMA_Clock_Get(dmaex->clk, dmaex->hdma.Instance);
	HAL_DMA_Init(&dmaex->hdma);
	IRQ_Init(&dmaex->hirq, &dmaex->hdma);				/** make the link **/
	dmaex->state = DMAEX_HANDLE_STATE_SET;
}

void	DMAEX_DeInit(DMAEX_HandleTypeDef* dmaex)
{
	IRQ_DeInit(&dmaex->hirq);
	HAL_DMA_DeInit(&dmaex->hdma);
	DMA_Clock_Put(dmaex->clk, dmaex->hdma.Instance);
	dmaex->state = DMAEX_HANDLE_STATE_RESET;
}

//void 	DMAEX_Clone(DMAEX_HandleTypeDef* dst, const DMAEX_HandleTypeDef* src)
//{
//	(*dst) = (*src);
//	
//	if (dst->useIRQ)
//	{
//		dst->hirq.hdata = &dst->hdma;
//	}
//}

void DMA_Clock_Get(DMA_ClockTypeDef* dma_clock, DMA_Stream_TypeDef* stream)
{
	int pos = 0;
	
	if (DMA1_Stream0 == stream ||
			DMA1_Stream1 == stream ||
			DMA1_Stream2 == stream ||
			DMA1_Stream3 == stream ||
			DMA1_Stream4 == stream ||
			DMA1_Stream5 == stream ||
			DMA1_Stream6 == stream ||
			DMA1_Stream7 == stream)
	{
		pos = (stream - DMA1_Stream0);
		
		dma_clock->dma1 |= (1 << pos);
		if (dma_clock->dma1)
		{
			__DMA1_CLK_ENABLE();
		}
		
		return;
	}
	
	if (DMA2_Stream0 == stream ||
			DMA2_Stream1 == stream ||
			DMA2_Stream2 == stream ||
			DMA2_Stream3 == stream ||
			DMA2_Stream4 == stream ||
			DMA2_Stream5 == stream ||
			DMA2_Stream6 == stream ||
			DMA2_Stream7 == stream)
	{
		pos = (stream - DMA2_Stream0);
		
		(dma_clock->dma2) |= (1 << pos);
		if (dma_clock->dma2)
		{
			__DMA2_CLK_ENABLE();
		}
		
		return;		
	}
}

void DMA_Clock_Put(DMA_ClockTypeDef* dma_clock, DMA_Stream_TypeDef* stream)
{
	int pos = 0;
	
	if (DMA1_Stream0 == stream ||
			DMA1_Stream1 == stream ||
			DMA1_Stream2 == stream ||
			DMA1_Stream3 == stream ||
			DMA1_Stream4 == stream ||
			DMA1_Stream5 == stream ||
			DMA1_Stream6 == stream ||
			DMA1_Stream7 == stream)
	{
		pos = (stream - DMA1_Stream0);
		
		(dma_clock->dma1) &= ~(1 << pos);
		if (dma_clock->dma1 == 0)
		{
			__DMA1_CLK_DISABLE();
		}
		
		return;
	}
	
	if (DMA2_Stream0 == stream ||
			DMA2_Stream1 == stream ||
			DMA2_Stream2 == stream ||
			DMA2_Stream3 == stream ||
			DMA2_Stream4 == stream ||
			DMA2_Stream5 == stream ||
			DMA2_Stream6 == stream ||
			DMA2_Stream7 == stream)
	{
		pos = (stream - DMA2_Stream0);
		
		(dma_clock->dma2) &= ~(1 << pos);
		if (dma_clock->dma2 == 0)
		{
			__DMA2_CLK_DISABLE();
		}
		
		return;		
	}	
}

bool DMA_Clock_Status(DMA_ClockTypeDef* dma_clock, DMA_Stream_TypeDef* stream)
{
	int pos = 0;
	
	if (DMA1_Stream0 == stream ||
			DMA1_Stream1 == stream ||
			DMA1_Stream2 == stream ||
			DMA1_Stream3 == stream ||
			DMA1_Stream4 == stream ||
			DMA1_Stream5 == stream ||
			DMA1_Stream6 == stream ||
			DMA1_Stream7 == stream)
	{
		pos = (stream - DMA1_Stream0);
		return (dma_clock->dma1 & (1 << pos)) ? true : false;
	}
	
	if (DMA2_Stream0 == stream ||
			DMA2_Stream1 == stream ||
			DMA2_Stream2 == stream ||
			DMA2_Stream3 == stream ||
			DMA2_Stream4 == stream ||
			DMA2_Stream5 == stream ||
			DMA2_Stream6 == stream ||
			DMA2_Stream7 == stream)
	{
		pos = (stream - DMA2_Stream0);
		return (dma_clock->dma2 & (1 << pos)) ? true : false;;		
	}	
	
	assert_param(false);
	return false;
}




void DMA1_Stream5_IRQHandler(void)
{
  HAL_NVIC_ClearPendingIRQ(DMA1_Stream5_IRQn);

	DMA_HandleTypeDef* handle = IRQ_HandlerData_Get(&IRQ_HandlerDataStore_Singleton, DMA1_Stream5_IRQn);
	if (handle)
	{
		HAL_DMA_IRQHandler(handle);
	}
}

/**
* @brief This function handles DMA1 Stream6 global interrupt.
*/
void DMA1_Stream6_IRQHandler(void)
{
  HAL_NVIC_ClearPendingIRQ(DMA1_Stream6_IRQn);
	
	DMA_HandleTypeDef* handle = IRQ_HandlerData_Get(&IRQ_HandlerDataStore_Singleton, DMA1_Stream6_IRQn);
	if (handle)
	{
		HAL_DMA_IRQHandler(handle);
	}	
}


///////////////////////////////////////////////////////////////////////////////
//
// Defaults to be cloned.
//
///////////////////////////////////////////////////////////////////////////////



