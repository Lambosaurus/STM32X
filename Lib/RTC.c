
#include "RTC.h"
#include "CLK.h"

/*
 * PRIVATE DEFINITIONS
 */


#define RTC_PREDIV	128

#define _RTC_WRITEPROTECTION_ENABLE() 		(RTC->WPR = 0xFF)
#define _RTC_WRITEPROTECTION_DISABLE() 		do { RTC->WPR = 0xCA; RTC->WPR = 0x53; } while(0)

#if defined(STM32G0) || defined(STM32WL)

#define RTC_IRQn							RTC_TAMP_IRQn
#define RTC_IRQHandler						RTC_TAMP_IRQHandler

#define ISR									ICSR
#define RTC_ISR_INIT						RTC_ICSR_INIT
#define RTC_ISR_WUTWF						RTC_ICSR_WUTWF
#define RTC_ISR_RSF							RTC_ICSR_RSF
#define RTC_ISR_INIT						RTC_ICSR_INIT

// Why it have to be this way?
#define _RTC_CLEAR_FLAG(flag)   			(RTC->SCR |= (flag))
#define _RTC_GET_FLAG(flag)    				(((((flag)) >> 8U) == 1U) ? (((RTC->ICSR & (1U << (((uint16_t)(flag)) & RTC_FLAG_MASK))) != 0U)) :\
                                              (((RTC->SR & (1U << (((uint16_t)(flag)) & RTC_FLAG_MASK))) != 0U)))

#else

#define _RTC_GET_FLAG(flag)   				(RTC->ISR & (flag))
#define _RTC_CLEAR_FLAG(flag)   			(RTC->ISR) = (~((flag) | RTC_ISR_INIT) | (RTC->ISR & RTC_ISR_INIT))

#define RTC_CLEAR_WUTF						RTC_FLAG_WUTF
#define RTC_CLEAR_ALRAF						RTC_FLAG_ALRAF
#define RTC_CLEAR_ALRBF						RTC_FLAG_ALRBF

#endif


/*
 * PRIVATE TYPES
 */

/*
 * PRIVATE PROTOTYPES
 */

static void RTC_EnterInit(void);
static void RTC_WaitForSync(void);

static uint32_t RTC_ToBCD(uint32_t bin);
static uint32_t RTC_FromBCD(uint32_t bcd);

/*
 * PRIVATE VARIABLES
 */

#ifdef RTC_USE_IRQS
static struct {
	VoidFunction_t AlarmACallback;
	VoidFunction_t AlarmBCallback;
	VoidFunction_t PeriodicCallback;
} gRtc;
#endif

/*
 * PUBLIC FUNCTIONS
 */

void RTC_Init(void)
{
#if defined(STM32G0)
	__HAL_RCC_RTCAPB_CLK_ENABLE();
#endif

	CLK_EnableLSO();

	__HAL_RCC_RTC_ENABLE();
	_RTC_WRITEPROTECTION_DISABLE();
	RTC_EnterInit();

	RTC->CR &= ~(RTC_CR_FMT | RTC_CR_OSEL | RTC_CR_POL);
	RTC->CR |= RTC_HOURFORMAT_24 | RTC_OUTPUT_DISABLE | RTC_OUTPUT_POLARITY_HIGH;

	uint32_t divisor = (CLK_GetLSOFreq() / RTC_PREDIV);
	RTC->PRER = ((RTC_PREDIV - 1) << 16U) | (divisor - 1);

	// Exit Initialization mode
	RTC->ISR &= ((uint32_t)~RTC_ISR_INIT);

	// If CR_BYPSHAD bit = 0, wait for synchro else this check is not needed
	if (!(RTC->CR & RTC_CR_BYPSHAD))
	{
		RTC_WaitForSync();
	}

#ifdef RTC_USE_IRQS
  __HAL_RTC_ALARM_EXTI_ENABLE_IT();
#if !defined(STM32G0)
  __HAL_RTC_ALARM_EXTI_ENABLE_RISING_EDGE();
#endif
	HAL_NVIC_EnableIRQ(RTC_IRQn);
#endif
	_RTC_WRITEPROTECTION_ENABLE();
}

void RTC_Deinit(void)
{
#ifdef RTC_USE_IRQS
	HAL_NVIC_DisableIRQ(RTC_IRQn);
#endif
	_RTC_WRITEPROTECTION_DISABLE();
	RTC_EnterInit();

	RTC->TR = 0x00000000U;
	RTC->DR = ((uint32_t)(RTC_DR_WDU_0 | RTC_DR_MU_0 | RTC_DR_DU_0));

#ifdef RTC_WAKEUPTIMER_ENABLE
	RTC->CR &= RTC_CR_WUCKSEL;
	while (!(RTC->ISR & RTC_ISR_WUTWF));
#endif
	RTC->CR = 0x00000000U;
#ifdef RTC_WAKEUPTIMER_ENABLE
	RTC->WUTR = RTC_WUTR_WUT;
#endif
	RTC->PRER = ((uint32_t)(RTC_PRER_PREDIV_A | 0x000000FFU));
	RTC->ALRMAR = 0x00000000U;
#ifdef RTC_ALARMB_ENABLE
	RTC->ALRMBR = 0x00000000U;
#endif
	// Reset ISR register and exit initialization mode
	RTC->ISR = 0x00000000U;

	if (!(RTC->CR & RTC_CR_BYPSHAD))
	{
		RTC_WaitForSync();
	}
	_RTC_WRITEPROTECTION_ENABLE();

	__HAL_RCC_RTC_DISABLE();
	CLK_DisableLSO();
}

void RTC_Write(DateTime_t * time)
{
	uint32_t treg = (RTC_ToBCD(time->hour)   << 16)
				  | (RTC_ToBCD(time->minute) << 8)
			      | (RTC_ToBCD(time->second));

	// Note, the weekday is being ignored.
	uint32_t dreg = (RTC_ToBCD(time->year - RTC_YEAR_MIN) << 16)
			      | (RTC_ToBCD(time->month) << 8)
        		  | (RTC_ToBCD(time->day));

	_RTC_WRITEPROTECTION_DISABLE();
	RTC_EnterInit();
	RTC->TR = treg & RTC_TR_RESERVED_MASK;
	RTC->DR = dreg & RTC_DR_RESERVED_MASK;
	RTC->CR &= ~RTC_CR_BKP;
	RTC->CR |= RTC_DAYLIGHTSAVING_NONE | RTC_STOREOPERATION_SET;

	// Exit init mode
	RTC->ISR &= ~RTC_ISR_INIT;
	if (!(RTC->CR & RTC_CR_BYPSHAD))
	{
		RTC_WaitForSync();
	}
	_RTC_WRITEPROTECTION_ENABLE();
}

void RTC_Read(DateTime_t * time)
{
	// Get subseconds structure field from the corresponding register
	(void)RTC->SSR;
	uint32_t treg = RTC->TR & RTC_TR_RESERVED_MASK;
	uint32_t dreg = RTC->DR & RTC_DR_RESERVED_MASK;

	time->hour 		= RTC_FromBCD((treg & (RTC_TR_HT | RTC_TR_HU)) >> 16);
	time->minute 	= RTC_FromBCD((treg & (RTC_TR_MNT | RTC_TR_MNU)) >> 8);
	time->second 	= RTC_FromBCD((treg & (RTC_TR_ST | RTC_TR_SU)));
	time->year 		= RTC_FromBCD((dreg & (RTC_DR_YT | RTC_DR_YU)) >> 16) + RTC_YEAR_MIN;
	time->month 	= RTC_FromBCD((dreg & (RTC_DR_MT | RTC_DR_MU)) >> 8);
	time->day 		= RTC_FromBCD((dreg & (RTC_DR_DT | RTC_DR_DU)));
}

#ifdef RTC_USE_IRQS

void RTC_OnAlarm(RTC_Alarm_t alarm, DateTime_t * time, RTC_Mask_t mask, VoidFunction_t callback)
{
	uint32_t treg = RTC_ALARMMASK_ALL & ~mask;
	if (time != NULL)
	{
		treg |=	(RTC_ByteToBcd2(time->hour)   << 16)
			 |  (RTC_ByteToBcd2(time->minute) << 8)
			 |  (RTC_ByteToBcd2(time->second));
	}
	uint32_t ssreg = 0;
	_RTC_WRITEPROTECTION_DISABLE();
	switch(alarm)
	{
	case RTC_Alarm_A:
		gRtc.AlarmACallback = callback;
		RTC->CR &= ~RTC_CR_ALRAE;
		_RTC_CLEAR_FLAG(RTC_FLAG_ALRAF);
		while (!_RTC_GET_FLAG(RTC_FLAG_ALRAWF));
		RTC->ALRMAR = treg;
		RTC->ALRMASSR = ssreg;
		RTC->CR |= RTC_CR_ALRAE | RTC_CR_ALRAIE;
	    break;
#ifdef RTC_ALARMB_ENABLE
	case RTC_Alarm_B:
		gRtc.AlarmBCallback = callback;
		RTC->CR &= ~RTC_CR_ALRBE;
	    _RTC_CLEAR_FLAG(RTC_FLAG_ALRBF);
	    while (!_RTC_GET_FLAG(RTC_FLAG_ALRBWF));
	    RTC->ALRMBR = treg;
	    RTC->ALRMBSSR = ssreg;
	    RTC->CR |= RTC_CR_ALRBE | RTC_CR_ALRBIE;
		break;
#endif //RTC_ALARMB_ENABLE
	}
  _RTC_WRITEPROTECTION_ENABLE();
}

void RTC_StopAlarm(RTC_Alarm_t alarm)
{
	_RTC_WRITEPROTECTION_DISABLE();
	switch(alarm)
	{
	case RTC_Alarm_A:
		// Disable alarm & it
		RTC->CR &= ~(RTC_CR_ALRAE | RTC_CR_ALRAIE);
		while(!_RTC_GET_FLAG(RTC_FLAG_ALRAWF));
		break;
#ifdef RTC_ALARMB_ENABLE
	case RTC_Alarm_B:
		// Disable alarm & it
		RTC->CR &= ~(RTC_CR_ALRBE | RTC_CR_ALRBIE);
		while(!_RTC_GET_FLAG(RTC_FLAG_ALRBWF));
		break;
	}
#endif //RTC_ALARMB_ENABLE
	_RTC_WRITEPROTECTION_ENABLE();
}

#ifdef RTC_WAKEUPTIMER_ENABLE
void RTC_OnPeriod(uint32_t ms, VoidFunction_t callback)
{
	gRtc.PeriodicCallback = callback;
	uint32_t clk = CLK_GetLSOFreq() / 16;
	uint32_t ticks = clk * ms / 1000;

	_RTC_WRITEPROTECTION_DISABLE();

	if (RTC->CR & RTC_CR_WUTE)
	{
		// Timer already enabled. Disable it.
		while (_RTC_GET_FLAG(RTC_FLAG_WUTWF));
		RTC->CR &= ~(RTC_CR_WUTE | RTC_CR_WUTIE);
		_RTC_CLEAR_FLAG(RTC_FLAG_WUTF);
		while (!_RTC_GET_FLAG(RTC_FLAG_WUTWF));
	}

	RTC->WUTR = ticks;
	MODIFY_REG(RTC->CR, RTC_CR_WUCKSEL, RTC_WAKEUPCLOCK_RTCCLK_DIV16);
	// Enable the timer & it
	RTC->CR |= RTC_CR_WUTE | RTC_CR_WUTIE;

	__HAL_RTC_WAKEUPTIMER_EXTI_ENABLE_IT();
	__HAL_RTC_WAKEUPTIMER_EXTI_ENABLE_RISING_EDGE();

	_RTC_WRITEPROTECTION_ENABLE();
}

void RTC_StopPeriod(void)
{
	_RTC_WRITEPROTECTION_DISABLE();
	// Disable WUT enable & Int
	RTC->CR &= ~(RTC_CR_WUTE | RTC_CR_WUTIE);
	while (!_RTC_GET_FLAG(RTC_FLAG_WUTWF));
	_RTC_WRITEPROTECTION_ENABLE();
}
#endif //RTC_WAKEUPTIMER_ENABLE

#endif //RTC_USE_IRQS


/*
 * PRIVATE FUNCTIONS
 */

static uint32_t RTC_ToBCD(uint32_t bin)
{
	uint32_t high = 0;
	while (bin >= 10)
	{
		high++;
		bin -= 10;
	}
	return (high << 4) | bin;
}

static uint32_t RTC_FromBCD(uint32_t bcd)
{
	return ((bcd >> 4) * 10) + (bcd & 0xF);
}

static void RTC_EnterInit(void)
{
	if (!_RTC_GET_FLAG(RTC_FLAG_INITF))
	{
#if defined(STM32G0)
		RTC->ISR |= RTC_ISR_INIT;
#else
		RTC->ISR = RTC_INIT_MASK;
#endif
		while (!_RTC_GET_FLAG(RTC_FLAG_INITF));
	}
}

static void RTC_WaitForSync(void)
{
	RTC->ISR &= RTC_RSF_MASK;
	while (!(RTC->ISR & RTC_ISR_RSF));
}

/*
 * INTERRUPT ROUTINES
 */

#ifdef RTC_USE_IRQS

static inline void RTC_CheckWakeupTimer(void)
{
	if (_RTC_GET_FLAG(RTC_FLAG_WUTF))
	{
		if (gRtc.PeriodicCallback)
		{
			gRtc.PeriodicCallback();
		}
		_RTC_CLEAR_FLAG(RTC_CLEAR_WUTF);
	}
}

static inline void RTC_CheckAlarms(void)
{
	if (_RTC_GET_FLAG(RTC_FLAG_ALRAF))
	{
		if (gRtc.AlarmACallback)
		{
			gRtc.AlarmACallback();
		}
		_RTC_CLEAR_FLAG(RTC_CLEAR_ALRAF);
	}

#ifdef RTC_ALARMB_ENABLE
	if (_RTC_GET_FLAG(RTC_FLAG_ALRBF))
	{
		if (gRtc.AlarmBCallback)
		{
			gRtc.AlarmBCallback();
		}
		_RTC_CLEAR_FLAG(RTC_CLEAR_ALRBF);
	}
#endif
}

void RTC_IRQHandler(void)
{
#if defined(STM32G0)

	RTC_CheckWakeupTimer();
	RTC_CheckAlarms();

#else // STM32G0

	// RTC wakuptimer & Alarms are on different EXTI lines.
	// These may get a different IRQHandler on different processors (or not be present)
	if (__HAL_RTC_WAKEUPTIMER_EXTI_GET_FLAG())
	{
		RTC_CheckWakeupTimer();
		__HAL_RTC_WAKEUPTIMER_EXTI_CLEAR_FLAG();
	}

	if (__HAL_RTC_ALARM_EXTI_GET_FLAG())
	{
		RTC_CheckAlarms();
		__HAL_RTC_ALARM_EXTI_CLEAR_FLAG();
	}

#endif // !STM32G0
}

#endif
