
#include "LPTIM.h"
#include "CLK.h"

/*
 * PRIVATE DEFINITIONS
 */


#ifdef STM32G0
#define LPTIM1_IRQn				TIM6_DAC_LPTIM1_IRQn
#define LPTIM2_IRQn 			TIM7_LPTIM2_IRQn

#define LPTIM1_IRQHandler		TIM6_DAC_LPTIM1_IRQHandler
#define LPTIM2_IRQHandler		TIM7_LPTIM2_IRQHandler
#endif

#ifdef CLK_USE_LSE
#define LPTIM1_CLK_SRC	RCC_LPTIM1CLKSOURCE_LSE
#define LPTIM2_CLK_SRC 	RCC_LPTIM2CLKSOURCE_LSE
#else
#define LPTIM1_CLK_SRC	RCC_LPTIM1CLKSOURCE_LSI
#define LPTIM2_CLK_SRC 	RCC_LPTIM2CLKSOURCE_LSI
#endif

/*
 * PRIVATE TYPES
 */

/*
 * PRIVATE PROTOTYPES
 */

static void LPTIMx_Init(LPTIM_t * tim);
static void LPTIMx_Deinit(LPTIM_t * tim);

/*
 * PRIVATE VARIABLES
 */

#ifdef LPTIM1_ENABLE
static LPTIM_t gLPTIM_1 = {
	.Instance = LPTIM1
};
LPTIM_t * const LPTIM_1 = &gLPTIM_1;
#endif
#ifdef LPTIM2_ENABLE
static LPTIM_t gLPTIM_2 = {
	.Instance = LPTIM2
};
LPTIM_t * const LPTIM_2 = &gLPTIM_2;
#endif

/*
 * PUBLIC FUNCTIONS
 */

void LPTIM_Init(LPTIM_t * tim, uint32_t freq, uint32_t reload)
{
	LPTIMx_Init(tim);

	uint32_t src = CLK_GetLSOFreq();
	uint32_t div = CLK_SelectPrescalar(src, 1, 128, &freq);

	uint32_t cfgr = (div * LPTIM_CFGR_PRESC_0);
	tim->Instance->CFGR = cfgr;

	tim->Instance->CR = LPTIM_CR_ENABLE;
	LPTIM_SetReload(tim, reload);
}

void LPTIM_Deinit(LPTIM_t * tim)
{
	tim->Instance->CR = 0;
	LPTIMx_Deinit(tim);
}

void LPTIM_SetReload(LPTIM_t * tim, uint32_t reload)
{
	tim->Instance->ICR = LPTIM_ICR_ARROKCF;
	tim->Instance->ARR = reload;
	while (!(tim->Instance->ISR & LPTIM_ISR_ARROK));
}

void LPTIM_Start(LPTIM_t * tim)
{
	tim->Instance->CR |= LPTIM_CR_CNTSTRT;
}

uint32_t LPTIM_Read(LPTIM_t * tim)
{
	return tim->Instance->CNT;
}

#ifdef LPTIM_USE_IRQS

void LPTIM_OnReload(LPTIM_t * tim, VoidFunction_t callback)
{
	if (callback)
	{
		tim->ReloadCallback = callback;
		tim->Instance->IER |= LPTIM_IER_ARRMIE;
	}
	else
	{
		tim->Instance->IER &= ~LPTIM_IER_ARRMIE;
	}
}

void LPTIM_OnPulse(LPTIM_t * tim, uint32_t value, VoidFunction_t callback)
{
	tim->Instance->IER &= ~LPTIM_IER_CMPMIE;
	tim->PulseCallback = callback;

	// Because of the different clock domains, we need to wait for the write to go through.
	tim->Instance->ICR = LPTIM_ICR_CMPOKCF;
	tim->Instance->CMP = value;
	while (!(tim->Instance->ISR & LPTIM_ISR_CMPOK));

	tim->Instance->IER |= LPTIM_IER_CMPMIE;
}

void LPTIM_StopPulse(LPTIM_t * tim)
{
	tim->Instance->IER &= ~LPTIM_IER_CMPMIE;
}

#endif //LPTIM_USE_IRQS

/*
 * PRIVATE FUNCTIONS
 */

static void LPTIMx_Init(LPTIM_t * tim)
{
#ifdef LPTIM1_ENABLE
	if (tim == LPTIM_1)
	{
		__HAL_RCC_LPTIM1_CONFIG(LPTIM1_CLK_SRC);
		__HAL_RCC_LPTIM1_CLK_ENABLE();
		HAL_NVIC_EnableIRQ(LPTIM1_IRQn);
	}

#endif
#ifdef LPTIM2_ENABLE
	if (tim == LPTIM_2)
	{
		__HAL_RCC_LPTIM2_CONFIG(LPTIM2_CLK_SRC);
		__HAL_RCC_LPTIM2_CLK_ENABLE();
		HAL_NVIC_EnableIRQ(LPTIM2_IRQn);
	}
#endif
}


static void LPTIMx_Deinit(LPTIM_t * tim)
{
#ifdef LPTIM1_ENABLE
	if (tim == LPTIM_1)
	{
		__HAL_RCC_LPTIM1_CLK_DISABLE();
	}
#endif
#ifdef LPTIM2_ENABLE
	if (tim == LPTIM_2)
	{
		__HAL_RCC_LPTIM2_CLK_DISABLE();
	}
#endif
}


/*
 * INTERRUPT ROUTINES
 */

#ifdef LPTIM_USE_IRQS

static void LPTIM_IRQHandler(LPTIM_t * tim)
{
	uint32_t isr = tim->Instance->ISR & tim->Instance->IER;
	if (isr & LPTIM_ISR_ARRM)
	{
		tim->Instance->ICR = LPTIM_ICR_ARRMCF;
		tim->ReloadCallback();
	}
	if (isr & LPTIM_ISR_CMPM)
	{
		tim->Instance->ICR = LPTIM_ICR_CMPMCF;
		tim->PulseCallback();
	}
}

#ifdef LPTIM1_ENABLE
void LPTIM1_IRQHandler(void)
{
	LPTIM_IRQHandler(LPTIM_1);
}
#endif //LPTIM1_ENABLE

#ifdef LPTIM2_ENABLE
void LPTIM2_IRQHandler(void)
{
	LPTIM_IRQHandler(LPTIM_2);
}
#endif //LPTIM2_ENABLE

#endif //LPTIM_USE_IRQS


