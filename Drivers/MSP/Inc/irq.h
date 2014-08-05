#ifndef __IRQ_H_
#define __IRQ_H_

#include <stdbool.h>
#include "stm32f4xx_hal.h"

typedef struct 
{
	void* irqh_obj[FPU_IRQn + 1];
	
} IRQ_HandleRegistryTypeDef;

extern IRQ_HandleRegistryTypeDef IRQ_HandlerObjectRegistry;

void 	IRQ_HandlerObject_Register(IRQ_HandleRegistryTypeDef* obj, IRQn_Type irqn, void* p);
void 	IRQ_HandlerObject_Unregister(IRQ_HandleRegistryTypeDef* obj, IRQn_Type irqn);
void*	IRQ_HandlerObject_Get(IRQ_HandleRegistryTypeDef* obj, IRQn_Type irqn);

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

typedef enum
{
	IRQ_HANDLE_STATE_RESET = 0,
	IRQ_HANDLE_STATE_SET = 1,
} IRQ_HandleStateTypeDef;

typedef struct
{
	IRQn_Type														irqn;
	uint32_t														preempt_priority;
	uint32_t														sub_priority;
	
} IRQ_ConfigTypeDef;

typedef struct
{
	IRQn_Type 													irqn;
	uint32_t 														preempt_priority;
	uint32_t 														sub_priority;
	
	IRQ_HandleRegistryTypeDef*					registry;
	
	/** these are runtime states		**/
	void*																irqh_obj;
	IRQ_HandleStateTypeDef							state;
	
} IRQ_HandleTypeDef;	

int	IRQ_Handle_Init(IRQ_HandleTypeDef* h, IRQn_Type irqn, uint32_t preempt, uint32_t sub, IRQ_HandleRegistryTypeDef* registry);
int	IRQ_Handle_InitByConfig(IRQ_HandleTypeDef* h, const IRQ_ConfigTypeDef* config, IRQ_HandleRegistryTypeDef* registry);

// void	IRQ_Handle_Dtor(IRQ_HandleTypeDef *handle);

/** exported functions **/
void 	IRQ_HAL_Init(IRQ_HandleTypeDef* hirq, void* irqh_obj);
void 	IRQ_HAL_DeInit(IRQ_HandleTypeDef* hirq);

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

const extern IRQ_ConfigTypeDef	IRQ_Uart2_DefaultConfig;
const extern IRQ_ConfigTypeDef	IRQ_Uart2RxDMA_DefaultConfig;
const extern IRQ_ConfigTypeDef	IRQ_Uart2TxDMA_DefaultConfig;

const extern IRQ_HandleTypeDef	IRQ_Handle_Uart2_Default;
const extern IRQ_HandleTypeDef 	IRQ_Handle_Uart2TxDMA_Default;
const extern IRQ_HandleTypeDef 	IRQ_Handle_Uart2RxDMA_Default;		







#endif


