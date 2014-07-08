#include <string.h>
#include "stm32f4xx_hal.h"
#include "irq.h"
#include "unity_fixture.h"



TEST_GROUP(IRQ_Handler);

TEST_SETUP(IRQ_Handler)
{
	
}

TEST_TEAR_DOWN(IRQ_Handler)
{
	
}

TEST(IRQ_Handler, IRQ_HandlerData_Register)
{
	IRQn_Type irqn = USART2_IRQn;	// 38
	void* p = (void*)(&irqn);
	IRQ_HandlerDataStore data;
	memset(&data, 0, sizeof(data));
	
	IRQ_HandlerData_Register(&data, irqn, p);
	TEST_ASSERT_EQUAL_HEX32(p, IRQ_HandlerData_Get(&data, irqn));
}

TEST(IRQ_Handler, IRQ_HandlerData_Unregister)
{
	IRQn_Type irqn = USART2_IRQn;
	void* p = (void*)(&irqn);
	IRQ_HandlerDataStore data;
	memset(&data, 0, sizeof(data));
	IRQ_HandlerData_Register(&data, irqn, p);
	
	IRQ_HandlerData_Unregister(&data, irqn);
	TEST_ASSERT_NULL(IRQ_HandlerData_Get(&data, irqn));
}

TEST_GROUP_RUNNER(IRQ_Handler)
{
	RUN_TEST_CASE(IRQ_Handler, IRQ_HandlerData_Register);
	RUN_TEST_CASE(IRQ_Handler, IRQ_HandlerData_Unregister);
}

/******** TEST GROUP FOR IRQ_INIT ********************************************/

static bool irq_enabled(IRQn_Type IRQn) 
{
	uint32_t reg;
	
	/** or NVIC->ISER[(uint32_t)((int32_t)IRQn) >> 5]; **/
	reg = NVIC->ISER[(uint32_t)((int32_t)IRQn) >> 5];
	return (reg & (1 << ((uint32_t)(IRQn) & 0x1F))) ? true : false;
}

TEST_GROUP(IRQ_Init);

TEST_SETUP(IRQ_Init)
{
}

TEST_TEAR_DOWN(IRQ_Init)
{
}

TEST(IRQ_Init, IRQ_Init)
{
	IRQ_HandleTypeDef 			handle;
	IRQ_HandlerDataStore		hdata_store;
	int											hdata;
	
	memset(&handle, 0, sizeof(handle));
	handle.irqn = USART2_IRQn;
	handle.hdata_store = &hdata_store;
	
	HAL_NVIC_DisableIRQ(USART2_IRQn);
	
	IRQ_Init(&handle, &hdata);
	
	TEST_ASSERT_TRUE(irq_enabled(USART2_IRQn));
	TEST_ASSERT_EQUAL_HEX32(&hdata, IRQ_HandlerData_Get(handle.hdata_store, USART2_IRQn));
	TEST_ASSERT_EQUAL(IRQ_HANDLE_STATE_SET, handle.state);
	
	HAL_NVIC_DisableIRQ(USART2_IRQn);
}

TEST(IRQ_Init, IRQ_DeInit)
{
	IRQ_HandleTypeDef 			handle;
	IRQ_HandlerDataStore		hdata_store;
	int											hdata;
	
	memset(&handle, 0, sizeof(handle));
	handle.irqn = USART2_IRQn;
	handle.hdata_store = &hdata_store;
	
	HAL_NVIC_DisableIRQ(USART2_IRQn);
	
	IRQ_Init(&handle, &hdata);
	IRQ_DeInit(&handle);
	
	TEST_ASSERT_FALSE(irq_enabled(USART2_IRQn));
	TEST_ASSERT_NULL(IRQ_HandlerData_Get(handle.hdata_store, USART2_IRQn));
	TEST_ASSERT_EQUAL(IRQ_HANDLE_STATE_RESET, handle.state);
	
	HAL_NVIC_DisableIRQ(USART2_IRQn);
}

TEST_GROUP_RUNNER(IRQ_Init)
{
	RUN_TEST_CASE(IRQ_Init, IRQ_Init);
	RUN_TEST_CASE(IRQ_Init, IRQ_DeInit);
}



