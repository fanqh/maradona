#include "stm32f4xx_hal.h"
#include "rtc.h"
#include <string.h>




#define BKUFLAGE  0X32F2


const RTCEX_HandleTypeDef	RTC_Haldle_Default=
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
	
};

const RTC_CalendarTypeDef InitCalendar =
{
			.initsdatestructure =
			{
					.Year = 0x14,
					.Month = RTC_MONTH_JULY,
					.Date = 0x7,
					.WeekDay = RTC_WEEKDAY_MONDAY,
			},
			.initstimestructure = 
			{
					.Hours = 0x00,
					.Minutes = 0x00,
					.Seconds =0x00,		
			}	
};



HAL_StatusTypeDef RTC_CalendarConfig(RTCEX_HandleTypeDef* hRTChandle,  RTC_CalendarTypeDef calendar)
{
	HAL_StatusTypeDef state;
	RTC_HandleTypeDef *phadle = &hRTChandle->rtc;
	
	if(phadle==NULL)
				return  HAL_ERROR;
	state = HAL_RTC_SetDate(phadle,&calendar.initsdatestructure,FORMAT_BCD);
	if(state!=HAL_OK)
			return state;
	state = HAL_RTC_SetTime(phadle,&calendar.initstimestructure,FORMAT_BCD);
	return state;
}

int RTC_open(unsigned char *name ,int flage)
{
		if(strcmp((const char*) name, "RTC1")!=0)
				return -1;
		if(HAL_RTC_Init((RTC_HandleTypeDef*) &RTC_Haldle_Default.rtc)!=HAL_OK)
					return -1;
		
}	
int RTC_write(RTCEX_HandleTypeDef rtc, void *buff, unsigned int len)
{
		
}


int RTC_read(RTCEX_HandleTypeDef rtc, void *buff, unsigned int len)
{
}


int RTC_close(RTCEX_HandleTypeDef rtc)
{
	
}




