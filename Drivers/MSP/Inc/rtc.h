
#ifndef __RTC_H
#define __RTC_H
#include "stm32f4xx_hal.h"
#include "rtc.h"


typedef struct
{
		RTC_DateTypeDef initsdatestructure;
		RTC_TimeTypeDef initstimestructure;
	
}RTC_CalendarTypeDef;

//typedef enum
//{
//		RTC_HOURFORMAT_24,
//		RTC_HOURFORMAT_12
//	
//}RTC_FORMAT_TypeDef;

typedef enum
{
	 RTCEX_STATE_RESET,
	 RTCEX_STATE_SET,
	 RTCEX_STATE_CONFIG
}RTC_State_TypeDef;




typedef struct
{
	RTC_HandleTypeDef rtc;
	RTC_CalendarTypeDef calendar;
	RTC_State_TypeDef state;
	uint32_t format;

	HAL_StatusTypeDef (*get_time)(RTC_HandleTypeDef *hrtc, RTC_TimeTypeDef *sTime, uint32_t Format);
	HAL_StatusTypeDef (*get_date)(RTC_HandleTypeDef *hrtc, RTC_DateTypeDef *sDate, uint32_t Format);	
	
}RTCEX_HandleTypeDef; 

typedef struct
{
//	HAL_StatusTypeDef	(*init)(RTC_HandleTypeDef *hrtc);
//	HAL_StatusTypeDef	(*deinit)(RTC_HandleTypeDef *hrtc);
//	HAL_StatusTypeDef (*settime)(RTC_HandleTypeDef *hrtc, RTC_TimeTypeDef *sTime, uint32_t Format);
//	HAL_StatusTypeDef (*setdata)(RTC_HandleTypeDef *hrtc, RTC_DateTypeDef *sDate, uint32_t Format);
//	HAL_StatusTypeDef (*gettime)(RTC_HandleTypeDef *hrtc, RTC_TimeTypeDef *sTime, uint32_t Format);
//	HAL_StatusTypeDef (*getdata)(RTC_HandleTypeDef *hrtc, RTC_TimeTypeDef *sTime, uint32_t Format);
	
	RTCEX_HandleTypeDef (*open) (unsigned char *name ,int flage);
	int (*read)(RTCEX_HandleTypeDef* hRTChandle, void *buff, unsigned int len);
	int (*write)(RTCEX_HandleTypeDef* hRTChandle, void *buff, unsigned int len);
	int (*close)(RTCEX_HandleTypeDef* hRTChandle);
	
}RTCEX_Operations;

RTCEX_HandleTypeDef* RTC_open(unsigned char *name ,int flage);
int RTC_write(RTCEX_HandleTypeDef* hRTChandle, void *buff, unsigned int len);
int RTC_read(RTCEX_HandleTypeDef* hRTChandle, void *buff, unsigned int len);
int RTC_close(RTCEX_HandleTypeDef* hRTChandle);


#endif




