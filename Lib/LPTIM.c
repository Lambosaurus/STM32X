
#include "LPTIM.h"
#ifdef LPTIM_ENABLE
#include "CLK.h"

/*
 * PRIVATE DEFINITIONS
 */

#ifdef CLK_USE_LSE
#define LPTIM1_CLK_SRC	RCC_LPTIM1CLKSOURCE_LSE
#else
#define LPTIM1_CLK_SRC	RCC_LPTIM1CLKSOURCE_LSI
#endif

/*
 * PRIVATE TYPES
 */

/*
 * PRIVATE PROTOTYPES
 */

static void LPTIM_Reload(void);

/*
 * PRIVATE VARIABLES
 */

static struct {
	VoidFunction_t onReload;
	VoidFunction_t onCompare;
} gLPTIM;

/*
 * PUBLIC FUNCTIONS
 */

void LPTIM_Init(uint32_t freq, uint32_t reload)
{
	__HAL_RCC_LPTIM1_CONFIG(LPTIM1_CLK_SRC);
	__HAL_RCC_LPTIM1_CLK_ENABLE();
	HAL_NVIC_EnableIRQ(LPTIM1_IRQn);

	uint32_t src = CLK_GetLSOFreq();
	uint32_t div = CLK_SelectPrescalar(src, 1, 128, &freq);

	uint32_t cfgr = LPTIM1->CFGR;
	cfgr &= LPTIM_CFGR_PRELOAD | LPTIM_CFGR_CKPOL | LPTIM_CFGR_CKSEL | LPTIM_CFGR_CKFLT | LPTIM_CFGR_PRESC;
	cfgr |= LPTIM_CLOCKSAMPLETIME_DIRECTTRANSITION | LPTIM_CLOCKPOLARITY_RISING | LPTIM_UPDATE_IMMEDIATE
			| (div * LPTIM_CFGR_PRESC_0);
	LPTIM1->CFGR = cfgr;
	LPTIM1->ICR = 0;

	LPTIM_SetReload(reload);
	LPTIM1->CR |= LPTIM_CR_ENABLE;
}

void LPTIM_Deinit(void)
{
	LPTIM1->CR &= ~LPTIM_CR_ENABLE;
	__HAL_RCC_LPTIM1_CLK_DISABLE();
}

void LPTIM_SetReload(uint32_t reload)
{
	LPTIM1->ARR = reload;
}

void LPTIM_Start()
{
	LPTIM_Reload();
	LPTIM1->CR |= LPTIM_CR_CNTSTRT;
}

uint32_t LPTIM_Read(void)
{
	return LPTIM1->CNT;
}

void LPTIM_OnReload(VoidFunction_t callback)
{
	if (callback)
	{
		gLPTIM.onReload = callback;
		LPTIM1->IER |= LPTIM_IER_ARRMIE;
	}
	else
	{
		LPTIM1->IER &= ~LPTIM_IER_ARRMIE;
	}
}

void LPTIM_OnCompare(uint32_t value, VoidFunction_t callback)
{
	gLPTIM.onCompare = callback;
	LPTIM1->CMP = value;
	LPTIM1->IER |= LPTIM_IER_CMPMIE;
}

void LPTIM_StopCompare(void)
{
	LPTIM1->IER &= ~LPTIM_IER_CMPMIE;
}

/*
 * PRIVATE FUNCTIONS
 */

static void LPTIM_Reload(void)
{
	LPTIM1->ICR = LPTIM_FLAG_ARROK;
	LPTIM1->ARR = LPTIM1->ARR;
	while (!(LPTIM1->ISR & LPTIM_FLAG_ARROK));
}

/*
 * INTERRUPT ROUTINES
 */

void LPTIM1_IRQHandler(void)
{
	uint32_t isr = LPTIM1->ISR & LPTIM1->IER;
	if (isr & LPTIM_FLAG_ARRM)
	{
		LPTIM1->ICR = LPTIM_FLAG_ARRM;
		gLPTIM.onReload();
	}
	if (isr & LPTIM_FLAG_CMPM)
	{
		LPTIM1->ICR = LPTIM_FLAG_CMPM;
		gLPTIM.onCompare();
	}
}

#endif //LPTIM_ENABLE
