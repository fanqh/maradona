#include "stm32f4xx_hal.h"
#include "irq.h"

IRQ_HandlerDataStore IRQ_HandlerDataStore_Singleton = {0};

void IRQ_HandlerData_Register(IRQ_HandlerDataStore* data, IRQn_Type irqn, void* p)
{
	if (irqn > FPU_IRQn || p == NULL)
		return;
	
	data->data[irqn] = p;
}

void IRQ_HandlerData_Unregister(IRQ_HandlerDataStore* data, IRQn_Type irqn)
{
	if (irqn > FPU_IRQn)
		return;
	
	data->data[irqn] = NULL;
}

void* IRQ_HandlerData_Get(IRQ_HandlerDataStore* data, IRQn_Type irqn)
{
	if (irqn > FPU_IRQn)
		return NULL;
	
	return data->data[irqn];
}

void IRQ_Init(IRQ_HandleTypeDef* hirq, void* hdata)
{
	assert_param(hirq);
	assert_param(hirq->irqn <= FPU_IRQn);
	
	/** the following assertion forcefully requires hdata & hdata_store be set **/
	/** without this assumption, it may require the user/caller to do this themselves, which is not reasonable **/
	assert_param(hirq->hdata_store);
	
	if (hdata)
	{
		hirq->hdata = hdata;
		IRQ_HandlerData_Register(hirq->hdata_store, hirq->irqn, hirq->hdata);
	}
	
	hirq->state = IRQ_HANDLE_STATE_SET;

	HAL_NVIC_SetPriority(hirq->irqn, hirq->preempt_priority, hirq->sub_priority);
	HAL_NVIC_EnableIRQ(hirq->irqn);
}

void IRQ_DeInit(IRQ_HandleTypeDef* hirq)
{
	assert_param(hirq);
	assert_param(hirq->irqn <= FPU_IRQn);
	
	HAL_NVIC_DisableIRQ(hirq->irqn);
	
	hirq->state = IRQ_HANDLE_STATE_RESET;
	
	if (hirq->hdata)
	{
		assert_param(hirq->hdata_store);
		IRQ_HandlerData_Unregister(hirq->hdata_store, hirq->irqn);
		hirq->hdata = 0;	// remove link
	}
}

