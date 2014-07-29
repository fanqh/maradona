#include "stm32f4xx_hal.h"
#include "rtc.h"
#include <string.h>




#define BKUFLAGE  0X32F2


RTCEX_HandleTypeDef	RTC_Haldle_File=
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



/**
  * @brief RTC MSP Initialization 
  *        This function configures the hardware resources used in this example
  * @param hrtc: RTC handle pointer
  * 
  * @note  Care must be taken when HAL_RCCEx_PeriphCLKConfig() is used to select 
  *        the RTC clock source; in this case the Backup domain will be reset in  
  *        order to modify the RTC Clock source, as consequence RTC registers (including 
  *        the backup registers) and RCC_BDCR register are set to their reset values.
  *             
  * @retval None
  */
void HAL_RTC_MspInit(RTC_HandleTypeDef *hrtc)
{
  RCC_OscInitTypeDef        RCC_OscInitStruct;
  RCC_PeriphCLKInitTypeDef  PeriphClkInitStruct;
  
  /* To change the source clock of the RTC feature (LSE, LSI), You have to:
     - Enable the power clock using __PWR_CLK_ENABLE()
     - Enable write access using HAL_PWR_EnableBkUpAccess() function before to 
       configure the RTC clock source (to be done once after reset).
     - Reset the Back up Domain using __HAL_RCC_BACKUPRESET_FORCE() and 
       __HAL_RCC_BACKUPRESET_RELEASE().
     - Configure the needed RTc clock source */
  
  /*##-1- Configue LSE as RTC clock soucre ###################################*/ 
  RCC_OscInitStruct.OscillatorType =  RCC_OSCILLATORTYPE_LSI | RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.LSIState = RCC_LSI_OFF;
	HAL_RCC_OscConfig(&RCC_OscInitStruct) ;

  
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_RTC;
  PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
  HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct);
  
  /*##-2- Enable RTC peripheral Clocks #######################################*/ 
  /* Enable RTC Clock */ 
  __HAL_RCC_RTC_ENABLE(); 
}



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
					.Hours   = 0x00,
					.Minutes = 0x00,
					.Seconds = 0x00,		
			}	
};



HAL_StatusTypeDef RTC_CalendarConfig(RTCEX_HandleTypeDef* hRTChandle, RTC_CalendarTypeDef* calendar) //设置完之后要加写入后备寄存器标记和状态标记
{
	HAL_StatusTypeDef state;
	RTC_HandleTypeDef *phandle = &hRTChandle->rtc;
	RTC_DateTypeDef data = calendar->initsdatestructure;
	RTC_TimeTypeDef time = calendar->initstimestructure;
	
	if(phandle==NULL)
			return  HAL_ERROR;   //不属于HAL层错误,但是
	if((data.Date>31)||(data.Date==0)||(data.Month>12)||(data.Month==0)||(data.WeekDay>RTC_WEEKDAY_SUNDAY))
			return HAL_ERROR;  
	
	state = HAL_RTC_SetDate(phandle, &data, hRTChandle->format);
	if(state!=HAL_OK)
			return state;
	state = HAL_RTC_SetTime(phandle, &time, hRTChandle->format);
	return state;	
}

RTCEX_HandleTypeDef* RTC_open(unsigned char *name ,int flage)
{
	
		if(strcmp((const char*) name, "RTC1")!=0)
				return NULL;
		if(HAL_RTC_Init((RTC_HandleTypeDef*) &RTC_Haldle_File.rtc)!=HAL_OK)
					return NULL;
		else
		{	
			return &RTC_Haldle_File;		//应该返回描述符，，数据 filehandle
		}
}	
int RTC_write(RTCEX_HandleTypeDef* hRTChandle, void *buff, unsigned int len)
{
		if((hRTChandle==NULL)||(buff==NULL)||(len<=0))
				return -1;
		if(RTC_CalendarConfig(hRTChandle, (RTC_CalendarTypeDef*) buff)==HAL_OK)  ///这么写很别扭
				return sizeof(RTC_CalendarTypeDef);				//
		else 
				return -1;
}


int RTC_read(RTCEX_HandleTypeDef* hRTChandle, void *buff, unsigned int len)
{
		RTC_HandleTypeDef *phandle = &hRTChandle->rtc;
		RTC_CalendarTypeDef* pcalendar = &hRTChandle->calendar;
	
		if((hRTChandle==NULL)||(buff==NULL)||(len<=0))
			 return -1;
		if((HAL_RTC_GetTime(phandle, &pcalendar->initstimestructure, hRTChandle->format)==HAL_OK)
					||(HAL_RTC_GetDate(phandle, &pcalendar->initsdatestructure, hRTChandle->format)==HAL_OK))		
				return sizeof(RTC_CalendarTypeDef);
		else 
				return -1;
}


int RTC_close(RTCEX_HandleTypeDef* hRTChandle)
{
		HAL_StatusTypeDef state;
		state = HAL_RTC_DeInit(&hRTChandle->rtc);
	  hRTChandle->state = RTCEX_STATE_RESET;
		if(state==HAL_OK)
				return 0;
		else
				return -1;	
}






