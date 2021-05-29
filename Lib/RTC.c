
#include "RTC.h"
#include "CLK.h"

/*
 * PRIVATE DEFINITIONS
 */

#define RTC_PREDIV	128

#if defined(STM32L0)
#define _RTC_WRITEPROTECTION_ENABLE() 		(RTC->WPR = 0xFF)
#define _RTC_WRITEPROTECTION_DISABLE() 	do { RTC->WPR = 0xCA; RTC->WPR = 0x53; } while(0)
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

static struct {
	VoidFunction_t AlarmACallback;
	VoidFunction_t AlarmBCallback;
	VoidFunction_t PeriodicCallback;
} gRtc;

/*
 * PUBLIC FUNCTIONS
 */

void RTC_Init(void)
{
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

	RTC->OR = RTC_OUTPUT_REMAP_NONE | RTC_OUTPUT_TYPE_OPENDRAIN;

	// If CR_BYPSHAD bit = 0, wait for synchro else this check is not needed
	if (!(RTC->CR & RTC_CR_BYPSHAD))
	{
		RTC_WaitForSync();
	}

	_RTC_WRITEPROTECTION_ENABLE();
}

void RTC_Deinit(void)
{
	_RTC_WRITEPROTECTION_DISABLE();
	RTC_EnterInit();

	RTC->TR = 0x00000000U;
	RTC->DR = ((uint32_t)(RTC_DR_WDU_0 | RTC_DR_MU_0 | RTC_DR_DU_0));
	RTC->CR &= RTC_CR_WUCKSEL;

	while (!(RTC->ISR & RTC_ISR_WUTWF));

	RTC->CR = 0x00000000U;
	RTC->WUTR = RTC_WUTR_WUT;
	RTC->PRER = ((uint32_t)(RTC_PRER_PREDIV_A | 0x000000FFU));
	RTC->ALRMAR = 0x00000000U;
	RTC->ALRMBR = 0x00000000U;

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
	uint32_t treg = (RTC_ByteToBcd2(time->hour)   << 16)
				  | (RTC_ByteToBcd2(time->minute) << 8)
			      | (RTC_ByteToBcd2(time->second));

	// Note, the weekday is being ignored.
	uint32_t dreg = (RTC_ByteToBcd2(time->year) << 16)
			      | (RTC_ByteToBcd2(time->month) << 8)
        		  | (RTC_ByteToBcd2(time->day));

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
	time->year 		= RTC_FromBCD((dreg & (RTC_DR_YT | RTC_DR_YU)) >> 16U);
	time->month 	= RTC_FromBCD((dreg & (RTC_DR_MT | RTC_DR_MU)) >> 8U);
	time->day 		= RTC_FromBCD((dreg & (RTC_DR_DT | RTC_DR_DU)));
}

#ifdef RTC_USE_IRQS
void RTC_OnAlarm(RTC_Alarm_t alarm, DateTime_t * time, RTC_Mask_t mask, VoidFunction_t callback);
void RTC_StopAlarm(RTC_Alarm_t alarm);

void RTC_OnPeriod(uint32_t ms, VoidFunction_t callback)
{
	gRtc.PeriodicCallback = callback;
}

void RTC_StopPeriod(void)
{

}

#endif


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
	return ((bcd >> 4) * 10) | (bcd & 0xF);
}

static void RTC_EnterInit(void)
{
	if (!(RTC->ISR & RTC_ISR_INITF))
	{
		RTC->ISR = RTC_INIT_MASK;
		while (!(RTC->ISR & RTC_ISR_INITF));
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

