#include <string.h>
#include <stdlib.h>
#include "stm32f4xx_hal.h"
#include "irq.h"

TEST_GROUP(IRQ_HandlerObjectRegistry);
TEST_SETUP(IRQ_HandlerObjectRegistry)
{
}

TEST_TEAR_DOWN(IRQ_HandlerObjectRegistry)
{
}

TEST(IRQ_HandlerObjectRegistry, IRQ_HandlerObject_Register)
{
	IRQn_Type irqn = USART2_IRQn;	// 38
	void* p = (void*)(&irqn);
	IRQ_HandlerObjectRegistryTypeDef data;
	memset(&data, 0, sizeof(data));
	
	IRQ_HandlerObject_Register(&data, irqn, p);
	TEST_ASSERT_EQUAL_HEX32(p, IRQ_HandlerObject_Get(&data, irqn));
}

TEST(IRQ_HandlerObjectRegistry, IRQ_HandlerObject_Unregister)
{
	IRQn_Type irqn = USART2_IRQn;
	void* p = (void*)(&irqn);
	IRQ_HandlerObjectRegistryTypeDef registry;
	memset(&registry, 0, sizeof(registry));
	IRQ_HandlerObject_Register(&registry, irqn, p);
	
	IRQ_HandlerObject_Unregister(&registry, irqn);
	TEST_ASSERT_NULL(IRQ_HandlerObject_Get(&registry, irqn));
}

TEST_GROUP_RUNNER(IRQ_HandlerObjectRegistry)
{
	RUN_TEST_CASE(IRQ_HandlerObjectRegistry, IRQ_HandlerObject_Register);
	RUN_TEST_CASE(IRQ_HandlerObjectRegistry, IRQ_HandlerObject_Unregister);
}

/******** TEST GROUP FOR IRQ_INIT ********************************************/

static bool irq_enabled(IRQn_Type IRQn) 
{
	uint32_t reg;
	
	/** or NVIC->ISER[(uint32_t)((int32_t)IRQn) >> 5]; **/
	reg = NVIC->ISER[(uint32_t)((int32_t)IRQn) >> 5];
	return (reg & (1 << ((uint32_t)(IRQn) & 0x1F))) ? true : false;
}

TEST_GROUP(IRQ_Handle);

TEST_SETUP(IRQ_Handle)
{
}

TEST_TEAR_DOWN(IRQ_Handle)
{
}

TEST(IRQ_Handle, IRQ_Init)
{
	IRQ_HandleTypeDef 									handle;
	IRQ_HandlerObjectRegistryTypeDef		registry;
	int																	irqh_obj;
	
	memset(&handle, 0, sizeof(handle));
	handle.irqn = USART2_IRQn;
	handle.registry = &registry;
	
	HAL_NVIC_DisableIRQ(USART2_IRQn);
	
	IRQ_Init(&handle, &irqh_obj);
	
	TEST_ASSERT_TRUE(irq_enabled(USART2_IRQn));
	TEST_ASSERT_EQUAL_HEX32(&irqh_obj, IRQ_HandlerObject_Get(handle.registry, USART2_IRQn));
	TEST_ASSERT_EQUAL(IRQ_HANDLE_STATE_SET, handle.state);
	
	HAL_NVIC_DisableIRQ(USART2_IRQn);
}

TEST(IRQ_Handle, IRQ_DeInit)
{
	IRQ_HandleTypeDef 									handle;
	IRQ_HandlerObjectRegistryTypeDef		registry;
	int																	hdata;
	
	memset(&handle, 0, sizeof(handle));
	handle.irqn = USART2_IRQn;
	handle.registry = &registry;
	
	HAL_NVIC_DisableIRQ(USART2_IRQn);
	
	IRQ_Init(&handle, &hdata);
	IRQ_DeInit(&handle);
	
	TEST_ASSERT_FALSE(irq_enabled(USART2_IRQn));
	TEST_ASSERT_NULL(IRQ_HandlerObject_Get(handle.registry, USART2_IRQn));
	TEST_ASSERT_EQUAL(IRQ_HANDLE_STATE_RESET, handle.state);
	
	HAL_NVIC_DisableIRQ(USART2_IRQn);
}

TEST(IRQ_Handle, Ctor)
{
	IRQ_HandlerObjectRegistryTypeDef fake;
	IRQ_HandleTypeDef* h = IRQ_Handle_Ctor(USART2_IRQn, 2, 3, &fake);
	
	TEST_ASSERT_NOT_NULL(h);
	TEST_ASSERT_EQUAL(USART2_IRQn, h->irqn);
	TEST_ASSERT_EQUAL(2, h->preempt_priority);
	TEST_ASSERT_EQUAL(3, h->sub_priority);
	TEST_ASSERT_EQUAL_HEX32(&fake, h->registry);
	TEST_ASSERT_NULL(h->irqh_obj);
	TEST_ASSERT_EQUAL(IRQ_HANDLE_STATE_RESET, h->state);
	
	if (h) free(h);
}

const extern IRQ_HandleTypeDef IRQ_Handle_Uart2_Default;

TEST(IRQ_Handle, CtorByTemplate)
{
	IRQ_HandlerObjectRegistryTypeDef fake;
	const IRQ_HandleTypeDef* dfl = &IRQ_Handle_Uart2_Default;
	
	IRQ_HandleTypeDef* h = IRQ_Handle_Ctor_By_Template(dfl, &fake);
	TEST_ASSERT_NOT_NULL(h);
	TEST_ASSERT_EQUAL(dfl->irqn, h->irqn);
	TEST_ASSERT_EQUAL(dfl->preempt_priority , h->preempt_priority);
	TEST_ASSERT_EQUAL(dfl->sub_priority , h->sub_priority);
	TEST_ASSERT_EQUAL_HEX32(&fake, h->registry);
	TEST_ASSERT_NULL(h->irqh_obj);
	TEST_ASSERT_EQUAL(IRQ_HANDLE_STATE_RESET, h->state);
	
	if (h) free(h);
}

TEST(IRQ_Handle, CtorByConfig)
{
	IRQ_HandlerObjectRegistryTypeDef fake;
	
	IRQ_HandleTypeDef* h = IRQ_Handle_CtorByConfig(&IRQ_Uart2_DefaultConfig, &fake);
	TEST_ASSERT_NOT_NULL(h);
	TEST_ASSERT_EQUAL(IRQ_Uart2_DefaultConfig.irqn, h->irqn);
	TEST_ASSERT_EQUAL(IRQ_Uart2_DefaultConfig.preempt_priority , h->preempt_priority);
	TEST_ASSERT_EQUAL(IRQ_Uart2_DefaultConfig.sub_priority , h->sub_priority);
	TEST_ASSERT_EQUAL_HEX32(&fake, h->registry);
	TEST_ASSERT_NULL(h->irqh_obj);
	TEST_ASSERT_EQUAL(IRQ_HANDLE_STATE_RESET, h->state);
	
	if (h) free(h);	
}

TEST_GROUP_RUNNER(IRQ_Handle)
{
	RUN_TEST_CASE(IRQ_Handle, IRQ_Init);
	RUN_TEST_CASE(IRQ_Handle, IRQ_DeInit);
	RUN_TEST_CASE(IRQ_Handle, Ctor);
	RUN_TEST_CASE(IRQ_Handle, CtorByTemplate);
}

TEST_GROUP_RUNNER(IRQ_All)
{
	RUN_TEST_GROUP(IRQ_HandlerObjectRegistry);
	RUN_TEST_GROUP(IRQ_Handle)
}



