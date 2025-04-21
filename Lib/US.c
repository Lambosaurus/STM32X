
#include "US.h"

#ifdef US_ENABLE
#include "TIM.h"
#else
#include "CLK.h"
#endif

/*
 * PRIVATE DEFINITIONS
 */

#if defined(STM32G0)
#define TUNED_FACTOR	14032
#else
#define TUNED_FACTOR	11225
#endif

#ifdef US_ENABLE
#if ((US_RES) & (US_RES - 1))
#error "US resolution must be a power of 2"
#endif
#endif //US_ENABLE

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

#ifdef US_ENABLE

void US_Init(void)
{
	TIM_Init(US_TIM, 1000000 / US_RES, US_TIM_MAX);
	TIM_Start(US_TIM);
}

uint32_t US_Read(void)
{
	uint32_t ticks = TIM_Read(US_TIM);
	return ticks * US_RES;
}

void US_Delay(uint32_t us)
{
	us += US_RES;
	uint32_t start = US_Read();
	while (US_Subtract(US_Read(), start) < us);
}

#else //US_ENABLE

void __attribute__((optimize("-Os"))) US_Delay(uint32_t us)
{
	// -Os will generate a straight forward output.
	// 11225 is our tuned factor.
	volatile uint32_t i = (us * (CLK_GetHCLKFreq() >> 10)) / TUNED_FACTOR;
	while(i--);
}

#endif //US_ENABLE

/*
 * PRIVATE FUNCTIONS
 */

/*
 * INTERRUPT ROUTINES
 */

