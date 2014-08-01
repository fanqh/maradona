#include <string.h>
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

TEST_GROUP(RTC_READ);

TEST_SETUP(RTC_READ){}
TEST_TEAR_DOWN(RTC_READ){}
	
TEST(RTC_READ, InvalidArgs)
{
	// int RTC_read(RTCEX_HandleTypeDef* hRTChandle, void *buff, unsigned int len)
	int ret;
	char buf[16];
	unsigned int len = 16;
	RTCEX_HandleTypeDef hrtc;
	
	ret = RTC_read(NULL, buf, len);
	TEST_ASSERT_EQUAL(-1, ret);
	// TEST_AASERT_EQUAL(16, errno);
	ret = RTC_read(&hrtc, NULL, len);
	TEST_ASSERT_EQUAL(-1, ret);
	
	ret = RTC_read(&hrtc, buf, 0);
	TEST_ASSERT_EQUAL(0, ret);
}
/**
typedef struct
{
		RTC_DateTypeDef initsdatestructure;
		RTC_TimeTypeDef initstimestructure;
	
}RTC_CalendarTypeDef; 
HAL_StatusTypeDef HAL_RTC_GetTime(RTC_HandleTypeDef *hrtc, RTC_TimeTypeDef *sTime, uint32_t Format)
HAL_StatusTypeDef HAL_RTC_GetDate(RTC_HandleTypeDef *hrtc, RTC_DateTypeDef *sDate, uint32_t Format)
**/
HAL_StatusTypeDef mock_get_time(RTC_HandleTypeDef *hrtc, RTC_TimeTypeDef *sTime, uint32_t Format)
{
	RTCEX_HandleTypeDef* hrtcex = container_of(hrtc, RTCEX_HandleTypeDef, rtc);
	
	TEST_ASSERT_EQUAL(hrtcex->format, Format);
	memset(sTime, 0xA5, sizeof(RTC_TimeTypeDef));
}

HAL_StatusTypeDef mock_get_date(RTC_HandleTypeDef *hrtc, RTC_DateTypeDef *sDate, uint32_t Format)
{
	RTCEX_HandleTypeDef* hrtcex = container_of(hrtc, RTCEX_HandleTypeDef, rtc);
	
	TEST_ASSERT_EQUAL(hrtcex->format, Format);
	memset(sDate, 0xB5, sizeof(RTC_DateTypeDef));
}

TEST(RTC_READ, read)
{
	int ret;
	RTCEX_HandleTypeDef hrtc;

	RTC_CalendarTypeDef bytes;
	RTC_DateTypeDef expected_date;
	RTC_TimeTypeDef expected_time;
	
	hrtc.get_date = &mock_get_date;
	hrtc.get_time = &mock_get_time;
	memset(&expected_date, 0xB5, sizeof(expected_date));
	memset(&expected_time, 0xA5, sizeof(expected_time));
	
	ret = RTC_read(&hrtc, &bytes, sizeof(bytes));
	
	TEST_ASSERT_EQUAL_MEMORY(&expected_date, &bytes.initsdatestructure, sizeof(RTC_DateTypeDef));
	TEST_ASSERT_EQUAL_MEMORY(&expected_time, &bytes.initstimestructure, sizeof(RTC_TimeTypeDef));
}

TEST_GROUP_RUNNER(RTC_All)
{
	RUN_TEST_GROUP(OPEN);
//	RUN_TEST_GROUP(READ);
//	RUN_TEST_GROUP(WRITE);
//	RUN_TEST_GROUP(CLOSE);

}



