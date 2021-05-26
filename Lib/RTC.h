#ifndef RTC_H
#define RTC_H

#include "STM32X.h"

/*
 * FUNCTIONAL TESTING
 * STM32L0: N
 * STM32F0: N
 */

/*
 * PUBLIC DEFINITIONS
 */

/*
 * PUBLIC TYPES
 */

typedef struct {
	uint8_t second;
	uint8_t minute;
	uint8_t hour;
	uint8_t day;
	uint8_t month;
	uint16_t year;
} DateTime_t;

typedef enum {
	RTC_Alarm_A,
	RTC_Alarm_B
} RTC_Alarm_t;

typedef enum {
	RTC_Mask_Second = (1 << 0),
	RTC_Mask_Minute = (1 << 1),
	RTC_Mask_Hour = (1 << 2),
	RTC_Mask_All = RTC_Mask_Second | RTC_Mask_Minute | RTC_Mask_Hour,
} RTC_Mask_t;

/*
 * PUBLIC FUNCTIONS
 */

bool RTC_Init(void);
void RTC_Deinit(void);

void RTC_Write(DateTime_t * time);
void RTC_Read(DateTime_t * time);

#ifdef RTC_USE_IRQS
void RTC_OnAlarm(RTC_Alarm_t alarm, DateTime_t * time, RTC_Mask_t mask, VoidFunction_t callback);
void RTC_StopAlarm(RTC_Alarm_t alarm);
void RTC_OnPeriod(uint32_t ms, VoidFunction_t callback);
void RTC_StopPeriod(void);
#endif

/*
 * EXTERN DECLARATIONS
 */

#endif //RTC_H
