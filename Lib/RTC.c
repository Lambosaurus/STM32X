
#include "RTC.h"

/*
 * PRIVATE DEFINITIONS
 */

/*
 * PRIVATE TYPES
 */

/*
 * PRIVATE PROTOTYPES
 */

/*
 * PRIVATE VARIABLES
 */

/*
 * PUBLIC FUNCTIONS
 */

bool RTC_Init(void)
{
	__HAL_RCC_RTC_ENABLE();
}

void RTC_Deinit(void)
{
	__HAL_RCC_RTC_DISABLE();
}

void RTC_Write(DateTime_t * time)
{

}

void RTC_Read(DateTime_t * time)
{

}

#ifdef RTC_USE_IRQS
void RTC_OnAlarm(RTC_Alarm_t alarm, DateTime_t * time, RTC_Mask_t mask, VoidFunction_t callback);
void RTC_StopAlarm(RTC_Alarm_t alarm);
void RTC_OnPeriod(uint32_t ms, VoidFunction_t callback);
void RTC_StopPeriod(void);
#endif


/*
 * PRIVATE FUNCTIONS
 */

/*
 * INTERRUPT ROUTINES
 */

