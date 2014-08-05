#include <stdlib.h>
#include <string.h>
#include "errno_ex.h"
#include "stm32f4xx_hal.h"
#include "irq.h"


IRQ_HandleRegistryTypeDef IRQ_HandlerObjectRegistry = {0};

void IRQ_HandlerObject_Register(IRQ_HandleRegistryTypeDef* registry, IRQn_Type irqn, void* p)
{
	if (irqn > FPU_IRQn || p == NULL)
		return;
	
	registry->irqh_obj[irqn] = p;
}

void IRQ_HandlerObject_Unregister(IRQ_HandleRegistryTypeDef* registry, IRQn_Type irqn)
{
	if (irqn > FPU_IRQn)
		return;
	
	registry->irqh_obj[irqn] = NULL;
}

void* IRQ_HandlerObject_Get(IRQ_HandleRegistryTypeDef* registry, IRQn_Type irqn)
{
	if (irqn > FPU_IRQn)
		return NULL;
	
	return registry->irqh_obj[irqn];
}



void IRQ_HAL_Init(IRQ_HandleTypeDef* hirq, void* irqh_obj)
{
	assert_param(hirq);
	assert_param(hirq->irqn <= FPU_IRQn);
	
	/** the following assertion forcefully requires hdata & hdata_store be set **/
	/** without this assumption, it may require the user/caller to do this themselves, which is not reasonable **/
	assert_param(hirq->registry);
	
	if (irqh_obj)
	{
		hirq->irqh_obj = irqh_obj;
		IRQ_HandlerObject_Register(hirq->registry, hirq->irqn, hirq->irqh_obj);
	}
	
	hirq->state = IRQ_HANDLE_STATE_SET;

	HAL_NVIC_SetPriority(hirq->irqn, hirq->preempt_priority, hirq->sub_priority);
	HAL_NVIC_EnableIRQ(hirq->irqn);
}

void IRQ_HAL_DeInit(IRQ_HandleTypeDef* hirq)
{
	assert_param(hirq);
	assert_param(hirq->irqn <= FPU_IRQn);
	
	HAL_NVIC_DisableIRQ(hirq->irqn);
	
	hirq->state = IRQ_HANDLE_STATE_RESET;
	
	if (hirq->irqh_obj)
	{
		assert_param(hirq->registry);
		IRQ_HandlerObject_Unregister(hirq->registry, hirq->irqn);
		hirq->irqh_obj = 0;	// remove link
	}
}

IRQ_HandleTypeDef *IRQ_Handle_Ctor(IRQn_Type irqn, uint32_t preempt, uint32_t sub, IRQ_HandleRegistryTypeDef* registry)
{
	IRQ_HandleTypeDef* h = (IRQ_HandleTypeDef*)malloc(sizeof(IRQ_HandleTypeDef));
	if (h == NULL) return NULL;
	
	h->irqh_obj = 0;
	h->registry = registry;
	h->irqn = irqn;
	h->preempt_priority = preempt;
	h->state = IRQ_HANDLE_STATE_RESET;
	h->sub_priority = sub;
	
	return h;
}

int	IRQ_Handle_Init(IRQ_HandleTypeDef* h, IRQn_Type irqn, uint32_t preempt, uint32_t sub, IRQ_HandleRegistryTypeDef* registry)
{	
	if (h == NULL || irqn < 0 || irqn > FPU_IRQn || registry == NULL)
		return -EINVAL;
	
	h->irqh_obj = 0;
	h->registry = registry;
	h->irqn = irqn;
	h->preempt_priority = preempt;
	h->state = IRQ_HANDLE_STATE_RESET;
	h->sub_priority = sub;
	
	return 0;
}

IRQ_HandleTypeDef	* IRQ_Handle_CtorByConfig(const IRQ_ConfigTypeDef* config, IRQ_HandleRegistryTypeDef* registry)
{
	return IRQ_Handle_Ctor(config->irqn, config->preempt_priority, config->sub_priority, registry);
}

int	IRQ_Handle_InitByConfig(IRQ_HandleTypeDef* h, const IRQ_ConfigTypeDef* config, IRQ_HandleRegistryTypeDef* registry)
{
	if (config == NULL)
		return -EINVAL;
	
	return IRQ_Handle_Init(h, config->irqn, config->preempt_priority, config->sub_priority, registry);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

const IRQ_ConfigTypeDef	IRQ_Uart2_DefaultConfig =
{
	.irqn = USART2_IRQn,
};

const IRQ_HandleTypeDef	IRQ_Handle_Uart2_Default =
{
	.irqn = USART2_IRQn,
	.registry = &IRQ_HandlerObjectRegistry,
	.irqh_obj = 0, // should be override		
};

const IRQ_ConfigTypeDef	IRQ_Uart2RxDMA_DefaultConfig =
{
	.irqn = DMA1_Stream5_IRQn,
};

const IRQ_HandleTypeDef IRQ_Handle_Uart2RxDMA_Default =
{
	.irqn = DMA1_Stream5_IRQn,
	.registry = &IRQ_HandlerObjectRegistry,
	.irqh_obj = 0, // THIS ONE should be link to the copied instance.hdma
};

const IRQ_ConfigTypeDef IRQ_Uart2TxDMA_DefaultConfig = 
{
	.irqn = DMA1_Stream6_IRQn,
};

const IRQ_HandleTypeDef IRQ_Handle_Uart2TxDMA_Default = 
{
	.irqn = DMA1_Stream6_IRQn,
	.registry = &IRQ_HandlerObjectRegistry,
	.irqh_obj = 0,	// should be override
};


