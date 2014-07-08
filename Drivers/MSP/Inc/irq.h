#ifndef __IRQ_H_
#define __IRQ_H_

#include <stdbool.h>
#include "stm32f4xx_hal.h"

typedef struct 
{
	void* data[FPU_IRQn + 1];
} IRQ_HandlerDataStore;

typedef enum
{
	IRQ_HANDLE_STATE_RESET = 0,
	IRQ_HANDLE_STATE_SET = 1,
} IRQ_HandleStateTypeDef;

typedef struct
{
	IRQn_Type 							irqn;
	uint32_t 								preempt_priority;
	uint32_t 								sub_priority;
	IRQ_HandlerDataStore*		hdata_store;
	
	/** these are states		**/
	void*										hdata;
	
	IRQ_HandleStateTypeDef	state;
	
} IRQ_HandleTypeDef;	



/** singletons **/
extern IRQ_HandlerDataStore IRQ_HandlerDataStore_Singleton;

/** exported functions **/
void 	IRQ_Init(IRQ_HandleTypeDef* hirq, void* hdata);
void 	IRQ_DeInit(IRQ_HandleTypeDef* hirq);

void 	IRQ_HandlerData_Register(IRQ_HandlerDataStore* data, IRQn_Type irqn, void* p);
void 	IRQ_HandlerData_Unregister(IRQ_HandlerDataStore* data, IRQn_Type irqn);
void*	IRQ_HandlerData_Get(IRQ_HandlerDataStore* data, IRQn_Type irqn);


#endif


