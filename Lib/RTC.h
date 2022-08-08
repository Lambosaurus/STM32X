#ifndef RTC_H
#define RTC_H

#include "STM32X.h"

/*
 * FUNCTIONAL TESTING
 * STM32L0: Y
 * STM32F0: N
 * STM32G0: N
 * STM32WL: N
 */

/*
 * PUBLIC DEFINITIONS
 */

#ifdef RTC_CR_ALRBE
#define RTC_ALARMB_ENABLE
#endif
#ifdef RTC_CR_WUTE
#define RTC_WAKEUPTIMER_ENABLE
#endif

#define RTC_YEAR_MIN	2000
#define RTC_YEAR_MAX	(RTC_YEAR_MIN + 99)

/*
 * PUBLIC TYPES
 */

typedef struct {
	uint16_t millis;
	uint8_t second;
	uint8_t minute;
	uint8_t hour;
	uint8_t day;
	uint8_t month;
	uint16_t year;
} DateTime_t;

typedef enum {
	RTC_Alarm_A,
#ifdef RTC_ALARMB_ENABLE
	RTC_Alarm_B
#endif
} RTC_Alarm_t;

// RTC_Mask_Second will generate an event every second
// RTC_Mask_Day will generate an event every day on the supplied hour, minute & second
typedef enum {
	RTC_Mask_Second = 0,
	RTC_Mask_Minute = RTC_ALARMMASK_SECONDS,
	RTC_Mask_Hour 	= RTC_ALARMMASK_SECONDS | RTC_ALARMMASK_MINUTES,
	RTC_Mask_Day 	= RTC_ALARMMASK_SECONDS | RTC_ALARMMASK_MINUTES | RTC_ALARMMASK_HOURS,
} RTC_Mask_t;

/*
 * PUBLIC FUNCTIONS
 */

void RTC_Init(void);
void RTC_Deinit(void);

void RTC_Write(DateTime_t * time);
void RTC_Read(DateTime_t * time);

#ifdef RTC_USE_IRQS

// time can be NULL to assume h,m,s = 0
void RTC_OnAlarm(RTC_Alarm_t alarm, DateTime_t * time, RTC_Mask_t mask, VoidFunction_t callback);
void RTC_StopAlarm(RTC_Alarm_t alarm);

#ifdef RTC_WAKEUPTIMER_ENABLE
void RTC_OnPeriod(uint32_t ms, VoidFunction_t callback);
void RTC_StopPeriod(void);
#endif //RTC_WAKEUPTIMER_ENABLE

#endif //RTC_USE_IRQS

#ifdef RTC_USE_BINARY
uint32_t RTC_ReadBinary(void);
#endif

/*
 * EXTERN DECLARATIONS
 */

#endif //RTC_H
