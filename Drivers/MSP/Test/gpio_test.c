#include <stdbool.h>
#include <string.h>
#include "errno_ex.h"
#include "stm32f4xx_hal.h"
#include "gpio.h"


static GPIO_ClockProviderTypeDef	gclk;

/**
#define __GPIOA_CLK_ENABLE()         (RCC->AHB1ENR |= (RCC_AHB1ENR_GPIOAEN))
#define __GPIOB_CLK_ENABLE()         (RCC->AHB1ENR |= (RCC_AHB1ENR_GPIOBEN))
#define __GPIOC_CLK_ENABLE()         (RCC->AHB1ENR |= (RCC_AHB1ENR_GPIOCEN))
#define __GPIOD_CLK_ENABLE()         (RCC->AHB1ENR |= (RCC_AHB1ENR_GPIODEN))
#define __GPIOE_CLK_ENABLE()         (RCC->AHB1ENR |= (RCC_AHB1ENR_GPIOEEN))
#define __GPIOH_CLK_ENABLE()         (RCC->AHB1ENR |= (RCC_AHB1ENR_GPIOHEN))
**/

bool gpioc_clock_is_enabled(void)
{
	return (RCC->AHB1ENR & (RCC_AHB1ENR_GPIOCEN)) ? true : false;
}

bool gpio_mode_set(GPIO_TypeDef* GPIOx, GPIO_InitTypeDef* init)
{
	uint32_t position = 0, moder, afr;
	
	while (!(init->Pin & ((uint32_t)1 << position)))
	{
		position++;
	}
	
	moder = GPIOx->MODER;
	moder = moder >> (position * 2);
	moder = moder & (uint32_t)3;
	
	if (moder != (init->Mode & (uint32_t)3))
		return false;
	
	// af = ((uint32_t)(init->Alternate) << (((uint32_t)position & (uint32_t)0x07) * 4));
	afr = GPIOx->AFR[position >> 3];
	afr = afr >> (((uint32_t)position & (uint32_t)0x07) * 4);
	afr = afr & ((uint32_t)0xF);
	
	if (afr != init->Alternate)
		return false;
	
	return true;
	
// GPIOx->MODER &= ~(GPIO_MODER_MODER0 << (position * 2));
// GPIOx->MODER |= ((GPIO_Init->Mode & GPIO_MODE) << (position * 2));
	
///* Configure Alternate function mapped with the current IO */
//temp = ((uint32_t)(GPIO_Init->Alternate) << (((uint32_t)position & (uint32_t)0x07) * 4)) ;
//GPIOx->AFR[position >> 3] &= ~((uint32_t)0xF << ((uint32_t)(position & (uint32_t)0x07) * 4)) ;
//GPIOx->AFR[position >> 3] |= temp;
}

bool gpio_mode_reset(GPIO_TypeDef* GPIOx, GPIO_InitTypeDef* init)
{
	uint32_t position = 0, moder, afr;
	
	while (!(init->Pin & ((uint32_t)1 << position)))
	{
		position++;
	}
	
	moder = GPIOx->MODER;
	moder = moder >> (position * 2);
	moder = moder & (uint32_t)3;
	
	if (moder != 0)
		return false;
	
	afr = GPIOx->AFR[position >> 3];
	afr = afr >> (((uint32_t)position & (uint32_t)0x07) * 4);
	afr = afr & ((uint32_t)0xF);
	
	if (afr != 0)
		return false;
	
	return true;
	
///*------------------------- GPIO Mode Configuration --------------------*/
///* Configure IO Direction in Input Floting Mode */
//GPIOx->MODER &= ~(GPIO_MODER_MODER0 << (position * 2));

///* Configure the default Alternate Function in current IO */
//GPIOx->AFR[position >> 3] &= ~((uint32_t)0xF << ((uint32_t)(position & (uint32_t)0x07) * 4)) ;
}




TEST_GROUP(GPIO_Clock);

TEST_SETUP(GPIO_Clock)
{
	int i;
	
	__GPIOC_CLK_DISABLE();
	HAL_GPIO_DeInit(GPIOC, GPIO_PIN_6);

	for (i = 0; i < 8; i++)
	{
		gclk.bits[i] = 0;
	}
}

TEST_TEAR_DOWN(GPIO_Clock)
{
	int i;
	
	__GPIOC_CLK_DISABLE();
	HAL_GPIO_DeInit(GPIOC, GPIO_PIN_6);
	
	for (i = 0; i < 8; i++)
	{
		gclk.bits[i] = 0;
	}
}

TEST(GPIO_Clock, ClockGet)   //enable clock 
{
	GPIO_Clock_Get(&gclk, GPIOC, GPIO_PIN_6);
	TEST_ASSERT_TRUE(gpioc_clock_is_enabled());
	TEST_ASSERT_TRUE(GPIO_Clock_Status(&gclk, GPIOC, GPIO_PIN_6));
}

TEST(GPIO_Clock, ClockPut)//disable clock
{
	GPIO_Clock_Get(&gclk, GPIOC, GPIO_PIN_6);
	GPIO_Clock_Put(&gclk, GPIOC, GPIO_PIN_6);
	TEST_ASSERT_FALSE(gpioc_clock_is_enabled());
	TEST_ASSERT_FALSE(GPIO_Clock_Status(&gclk, GPIOC, GPIO_PIN_6));
}

TEST(GPIO_Clock, ClockGet2Put1)
{
	GPIO_Clock_Get(&gclk, GPIOC, GPIO_PIN_6);
	GPIO_Clock_Get(&gclk, GPIOC, GPIO_PIN_7);
	GPIO_Clock_Put(&gclk, GPIOC, GPIO_PIN_7);
	TEST_ASSERT_TRUE(gpioc_clock_is_enabled());
	TEST_ASSERT_TRUE(GPIO_Clock_Status(&gclk, GPIOC, GPIO_PIN_6));
	TEST_ASSERT_FALSE(GPIO_Clock_Status(&gclk, GPIOC, GPIO_PIN_7));
}

TEST(GPIO_Clock, GPIOEX_HAL_Init)
{

	GPIOEX_TypeDef gpioex = PC6_As_Uart6Tx_Default;
	gpioex.clk = &gclk;	// mock
	GPIOEX_HAL_Init(&gpioex);
	
	TEST_ASSERT_TRUE(GPIO_Clock_Status(gpioex.clk, GPIOC, gpioex.init.Pin));
	TEST_ASSERT_TRUE(gpio_mode_set(gpioex.instance, &gpioex.init));
	TEST_ASSERT_EQUAL(GPIOEX_STATE_SET, gpioex.state);
}

TEST(GPIO_Clock, GPIOEX_HAL_DeInit)
{
	GPIOEX_TypeDef gpioex = PC6_As_Uart6Tx_Default;
	gpioex.clk = &gclk;	// mock
	GPIOEX_HAL_Init(&gpioex);
	GPIOEX_HAL_DeInit(&gpioex);
	
	TEST_ASSERT_FALSE(GPIO_Clock_Status(gpioex.clk, GPIOC, gpioex.init.Pin));
	TEST_ASSERT_TRUE(gpio_mode_reset(gpioex.instance, &gpioex.init));
	TEST_ASSERT_EQUAL(GPIOEX_STATE_RESET, gpioex.state);	
}


TEST_GROUP_RUNNER(GPIO_Clock)
{
	RUN_TEST_CASE(GPIO_Clock, ClockGet);
	RUN_TEST_CASE(GPIO_Clock, ClockPut);
	RUN_TEST_CASE(GPIO_Clock, ClockGet2Put1);
	
	RUN_TEST_CASE(GPIO_Clock, GPIOEX_HAL_Init);
	RUN_TEST_CASE(GPIO_Clock, GPIOEX_HAL_DeInit);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

TEST_GROUP(GPIOEX_Type);

TEST_SETUP(GPIOEX_Type){}
TEST_TEAR_DOWN(GPIOEX_Type){}

TEST(GPIOEX_Type, InitInvalidArgs)
{
	int ret;
	GPIO_ClockProviderTypeDef	clk;

	const GPIO_ConfigTypeDef* config = &PC6_As_Uart6Tx_DefaultConfig;
	GPIOEX_TypeDef 						ge;
	TEST_ASSERT_NOT_NULL(&ge);
	memset(&ge, 0xA5, sizeof(GPIOEX_TypeDef));
	
	ret = GPIOEX_Init(0, config->instance, &config->init, &clk);
	TEST_ASSERT_EQUAL(-EINVAL, ret);

	ret = GPIOEX_Init(&ge, 0, &config->init, &clk);
	TEST_ASSERT_EQUAL(-EINVAL, ret);
	
	ret = GPIOEX_Init(&ge, config->instance, 0, &clk);
	TEST_ASSERT_EQUAL(-EINVAL, ret);
	
	ret = GPIOEX_Init(&ge, config->instance, &config->init, 0);
	TEST_ASSERT_EQUAL(-EINVAL, ret);
}
	
	
TEST(GPIOEX_Type, Init)
{
	int ret;
	GPIO_ClockProviderTypeDef	clk;

	const GPIO_ConfigTypeDef* config = &PC6_As_Uart6Tx_DefaultConfig;
	GPIOEX_TypeDef ge;
	
	memset(&ge, 0xA5, sizeof(GPIOEX_TypeDef));
	
	ret = GPIOEX_Init(&ge, config->instance, &config->init, &clk);
	
	TEST_ASSERT_EQUAL(0, ret);
	TEST_ASSERT_EQUAL_HEX32(config->instance, ge.instance);
	TEST_ASSERT_EQUAL_MEMORY(&config->init, &ge.init, sizeof(ge.init));
	TEST_ASSERT_EQUAL_HEX32(&clk, ge.clk);
	TEST_ASSERT_EQUAL(GPIOEX_STATE_RESET, ge.state);
}

extern const GPIO_ConfigTypeDef PC6_As_Uart6Tx_DefaultConfig;

//TEST(GPIOEX_Type, Dtor)
//{
//	GPIOEX_TypeDef* ge;
//	GPIO_ClockProviderTypeDef	clk;
//	
//	ge = GPIOEX_Init(PC6_As_Uart6Tx_Default.instance, &PC6_As_Uart6Tx_Default.init, &clk);

//	if (ge) GPIOEX_Dtor(ge);
//}
	
TEST_GROUP_RUNNER(GPIOEX_Type)
{
	RUN_TEST_CASE(GPIOEX_Type, InitInvalidArgs);
	RUN_TEST_CASE(GPIOEX_Type, Init);
	// RUN_TEST_CASE(GPIOEX_Type, InitByConfig); no test for wrapper function
	// RUN_TEST_CASE(GPIOEX_Type, Dtor);
}

TEST_GROUP_RUNNER(GPIO_All)
{
	RUN_TEST_GROUP(GPIO_Clock);
	RUN_TEST_GROUP(GPIOEX_Type);
}


	

