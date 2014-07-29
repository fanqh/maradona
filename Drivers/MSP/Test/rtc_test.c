#include "stm32f4xx_hal.h"
#include "gpio.h"
#include "rtc.h"



RTCEX_HandleTypeDef	RTC_Haldle_Test=
{
	.rtc =
	{
		.Instance = RTC,
		.Init =
		{
				.HourFormat = RTC_HOURFORMAT_24,
				.AsynchPrediv = 0x7F,		/* LSE as RTC clock */
				.SynchPrediv = 0x00FF,  /* LSE as RTC clock */
				.OutPut = RTC_OUTPUT_DISABLE,
				.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH,
				.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN,
		},
	},
	.format = RTC_HOURFORMAT_24,
	
};


TEST_GROUP(OPEN);

TEST_SETUP(OPEN)
{
}
TEST_TEAR_DOWN(OPEN)
{
}


TEST(OPEN, MspInit)   
{
		HAL_RTC_Init(&RTC_Haldle_Test.rtc);
		TEST_ASSERT_EQUAL(HAL_RTC_STATE_READY, RTC_Haldle_Test.rtc.State);
}

TEST(OPEN, Parameter)   //enable clock 
{
		TEST_ASSERT_NOT_NULL(RTC_open((unsigned char*)"RTC1" ,1));
		TEST_ASSERT_NULL(RTC_open((unsigned char*)"RTC" ,1));
}


TEST_GROUP_RUNNER(OPEN)
{
	RUN_TEST_CASE(OPEN, MspInit);
	RUN_TEST_CASE(OPEN, Parameter);
}

TEST_GROUP_RUNNER(RTC_All)
{
	RUN_TEST_GROUP(OPEN);
//	RUN_TEST_GROUP(READ);
//	RUN_TEST_GROUP(WRITE);
//	RUN_TEST_GROUP(CLOSE);

}



