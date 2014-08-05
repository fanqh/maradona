#include <string.h>
#include <stdlib.h>
#include "errno_ex.h"
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
	IRQ_HandleRegistryTypeDef data;
	memset(&data, 0, sizeof(data));
	
	IRQ_HandlerObject_Register(&data, irqn, p);
	TEST_ASSERT_EQUAL_HEX32(p, IRQ_HandlerObject_Get(&data, irqn));
}

TEST(IRQ_HandlerObjectRegistry, IRQ_HandlerObject_Unregister)
{
	IRQn_Type irqn = USART2_IRQn;
	void* p = (void*)(&irqn);
	IRQ_HandleRegistryTypeDef registry;
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

TEST(IRQ_Handle, IRQ_HAL_Init)
{
	IRQ_HandleTypeDef 									handle;
	IRQ_HandleRegistryTypeDef						registry;
	int																	irqh_obj;
	
	memset(&handle, 0, sizeof(handle));
	handle.irqn = USART2_IRQn;
	handle.registry = &registry;
	
	HAL_NVIC_DisableIRQ(USART2_IRQn);
	
	IRQ_HAL_Init(&handle, &irqh_obj);
	
	TEST_ASSERT_TRUE(irq_enabled(USART2_IRQn));
	TEST_ASSERT_EQUAL_HEX32(&irqh_obj, IRQ_HandlerObject_Get(handle.registry, USART2_IRQn));
	TEST_ASSERT_EQUAL(IRQ_HANDLE_STATE_SET, handle.state);
	
	HAL_NVIC_DisableIRQ(USART2_IRQn);
}

TEST(IRQ_Handle, IRQ_HAL_DeInit)
{
	IRQ_HandleTypeDef 					handle;
	IRQ_HandleRegistryTypeDef		registry;
	int													hdata;
	
	memset(&handle, 0, sizeof(handle));
	handle.irqn = USART2_IRQn;
	handle.registry = &registry;
	
	HAL_NVIC_DisableIRQ(USART2_IRQn);
	
	IRQ_HAL_Init(&handle, &hdata);
	IRQ_HAL_DeInit(&handle);
	
	TEST_ASSERT_FALSE(irq_enabled(USART2_IRQn));
	TEST_ASSERT_NULL(IRQ_HandlerObject_Get(handle.registry, USART2_IRQn));
	TEST_ASSERT_EQUAL(IRQ_HANDLE_STATE_RESET, handle.state);
	
	HAL_NVIC_DisableIRQ(USART2_IRQn);
}

//TEST(IRQ_Handle, Ctor)
//{
//	IRQ_HandleRegistryTypeDef fake;
//	IRQ_HandleTypeDef* h = NULL;
//	
//	h = IRQ_Handle_Ctor(USART2_IRQn, 2, 3, &fake);
//	
//	TEST_ASSERT_NOT_NULL(h);
//	TEST_ASSERT_EQUAL(USART2_IRQn, h->irqn);
//	TEST_ASSERT_EQUAL(2, h->preempt_priority);
//	TEST_ASSERT_EQUAL(3, h->sub_priority);
//	TEST_ASSERT_EQUAL_HEX32(&fake, h->registry);
//	TEST_ASSERT_NULL(h->irqh_obj);
//	TEST_ASSERT_EQUAL(IRQ_HANDLE_STATE_RESET, h->state);
//	
//	if (h) free(h);
//}

TEST(IRQ_Handle, HandleInitInvalidArgs)
{
	int ret;
	
	IRQ_HandleRegistryTypeDef fake;
	IRQ_HandleTypeDef h;
	
	ret = IRQ_Handle_Init(0, USART2_IRQn, 2, 3, &fake);
	TEST_ASSERT_EQUAL(-EINVAL, ret);
	
	// low bound
	ret = IRQ_Handle_Init(&h, SysTick_IRQn, 2, 3, &fake);	// SysTick_IRQn = -1
	TEST_ASSERT_EQUAL(-EINVAL, ret);
	
	// high bound
	ret = IRQ_Handle_Init(&h, (IRQn_Type)(FPU_IRQn+1), 2, 3, &fake);
	TEST_ASSERT_EQUAL(-EINVAL, ret);
	
	ret = IRQ_Handle_Init(&h, USART2_IRQn, 2, 3, NULL);
	TEST_ASSERT_EQUAL(-EINVAL, ret);
}

TEST(IRQ_Handle, HandleInitSuccess)
{
	// int	IRQ_Handle_Init(IRQ_HandleTypeDef* h, IRQn_Type irqn, uint32_t preempt, uint32_t sub, IRQ_HandleRegistryTypeDef* registry);
	int ret;
	IRQ_HandleRegistryTypeDef fake;
	IRQ_HandleTypeDef h;
	
	ret = IRQ_Handle_Init(&h, USART2_IRQn, 2, 3, &fake);
	
	TEST_ASSERT_EQUAL(0, ret);
	
	TEST_ASSERT_EQUAL(USART2_IRQn, h.irqn);
	TEST_ASSERT_EQUAL(2, h.preempt_priority);
	TEST_ASSERT_EQUAL(3, h.sub_priority);
	TEST_ASSERT_EQUAL_HEX32(&fake, h.registry);
	TEST_ASSERT_NULL(h.irqh_obj);
	TEST_ASSERT_EQUAL(IRQ_HANDLE_STATE_RESET, h.state);
}

const extern IRQ_HandleTypeDef IRQ_Handle_Uart2_Default;

//TEST(IRQ_Handle, CtorByTemplate)
//{
//	IRQ_HandleRegistryTypeDef fake;
//	const IRQ_HandleTypeDef* dfl = &IRQ_Handle_Uart2_Default;
//	
//	IRQ_HandleTypeDef* h = IRQ_Handle_Ctor_By_Template(dfl, &fake);
//	TEST_ASSERT_NOT_NULL(h);
//	TEST_ASSERT_EQUAL(dfl->irqn, h->irqn);
//	TEST_ASSERT_EQUAL(dfl->preempt_priority , h->preempt_priority);
//	TEST_ASSERT_EQUAL(dfl->sub_priority , h->sub_priority);
//	TEST_ASSERT_EQUAL_HEX32(&fake, h->registry);
//	TEST_ASSERT_NULL(h->irqh_obj);
//	TEST_ASSERT_EQUAL(IRQ_HANDLE_STATE_RESET, h->state);
//	
//	if (h) free(h);
//}

//TEST(IRQ_Handle, CtorByConfig)
//{
//	IRQ_HandleRegistryTypeDef fake;
//	
//	IRQ_HandleTypeDef* h = IRQ_Handle_CtorByConfig(&IRQ_Uart2_DefaultConfig, &fake);
//	TEST_ASSERT_NOT_NULL(h);
//	TEST_ASSERT_EQUAL(IRQ_Uart2_DefaultConfig.irqn, h->irqn);
//	TEST_ASSERT_EQUAL(IRQ_Uart2_DefaultConfig.preempt_priority , h->preempt_priority);
//	TEST_ASSERT_EQUAL(IRQ_Uart2_DefaultConfig.sub_priority , h->sub_priority);
//	TEST_ASSERT_EQUAL_HEX32(&fake, h->registry);
//	TEST_ASSERT_NULL(h->irqh_obj);
//	TEST_ASSERT_EQUAL(IRQ_HANDLE_STATE_RESET, h->state);
//	
//	if (h) free(h);	
//}

TEST(IRQ_Handle, HandleInitByConfigInvalidArgs)
{
	int ret;
	IRQ_HandleRegistryTypeDef fake;
	IRQ_HandleTypeDef h;
	const IRQ_ConfigTypeDef* cfg = 0;
	
	/** only invalid config checked. other validation should be done by basic init/ctor **/
	ret = IRQ_Handle_InitByConfig(&h, cfg, &fake);	
	TEST_ASSERT_EQUAL(-EINVAL, ret);
}

TEST(IRQ_Handle, HandleInitByConfigSuccess)
{	
	int ret;
	IRQ_HandleRegistryTypeDef fake;
	IRQ_HandleTypeDef h;
	const IRQ_ConfigTypeDef* cfg = &IRQ_Uart2_DefaultConfig;
	
	ret = IRQ_Handle_InitByConfig(&h, cfg, &fake);
	
	TEST_ASSERT_EQUAL(0, ret);	
	
	TEST_ASSERT_EQUAL(cfg->irqn, h.irqn);
	TEST_ASSERT_EQUAL(cfg->preempt_priority , h.preempt_priority);
	TEST_ASSERT_EQUAL(cfg->sub_priority , h.sub_priority);
	TEST_ASSERT_EQUAL_HEX32(&fake, h.registry);
	TEST_ASSERT_NULL(h.irqh_obj);
	TEST_ASSERT_EQUAL(IRQ_HANDLE_STATE_RESET, h.state);
}

TEST_GROUP_RUNNER(IRQ_Handle)
{
	RUN_TEST_CASE(IRQ_Handle, IRQ_HAL_Init);
	RUN_TEST_CASE(IRQ_Handle, IRQ_HAL_DeInit);
	RUN_TEST_CASE(IRQ_Handle, HandleInitInvalidArgs);
	RUN_TEST_CASE(IRQ_Handle, HandleInitSuccess);
	RUN_TEST_CASE(IRQ_Handle, HandleInitByConfigInvalidArgs);
	RUN_TEST_CASE(IRQ_Handle, HandleInitByConfigSuccess);
	
	// RUN_TEST_CASE(IRQ_Handle, Ctor);
	// RUN_TEST_CASE(IRQ_Handle, CtorByTemplate);
	// RUN_TEST_CASE(IRQ_Handle, CtorByConfig);
}

TEST_GROUP_RUNNER(IRQ_All)
{
	RUN_TEST_GROUP(IRQ_HandlerObjectRegistry);
	RUN_TEST_GROUP(IRQ_Handle)
}



