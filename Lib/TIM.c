
#include "TIM.h"
#include "CLK.h"

/*
 * PRIVATE DEFINITIONS
 */

#define TIM_ENABLE_CCx(tim, cc)   	(tim->Instance->CCER |=  (TIM_CCER_CC1E << (cc*4)))
#define TIM_DISABLE_CCx(tim, cc) 	(tim->Instance->CCER &= ~(TIM_CCER_CC1E << (cc*4)))

#define TIM_CCMRx_MSK	(TIM_CCMR1_OC1M | TIM_CCMR1_CC1S | TIM_CCMR1_OC1FE | TIM_CCMR1_OC1PE)

#define TIM_GET_IRQ_SOURCES(tim)	(tim->Instance->SR & tim->Instance->DIER)


/*
 * PRIVATE TYPES
 */

/*
 * PRIVATE PROTOTYPES
 */

static void TIMx_Init(TIM_t * tim);
static void TIMx_Deinit(TIM_t * tim);
static void TIM_Reload(TIM_t * tim);

#ifdef TIM_USE_IRQS
static void TIM_IRQHandler(TIM_t * tim);
#endif //TIM_USE_IRQS

static void TIM_EnableOCx(TIM_t * tim, uint32_t oc, uint32_t mode);

/*
 * PRIVATE VARIABLES
 */

#ifdef TIM1_ENABLE
static TIM_t gTIM_1 = {
	.Instance = TIM1
};
TIM_t * const TIM_1 = &gTIM_1;
#endif
#ifdef TIM2_ENABLE
static TIM_t gTIM_2 = {
	.Instance = TIM2
};
TIM_t * const TIM_2 = &gTIM_2;
#endif
#ifdef TIM3_ENABLE
static TIM_t gTIM_3 = {
	.Instance = TIM3
};
TIM_t * const TIM_3 = &gTIM_3;
#endif
#ifdef TIM5_ENABLE
static TIM_t gTIM_5 = {
	.Instance = TIM5
};
TIM_t * const TIM_5 = &gTIM_5;
#endif
#ifdef TIM6_ENABLE
static TIM_t gTIM_6 = {
	.Instance = TIM6
};
TIM_t * const TIM_6 = &gTIM_6;
#endif
#ifdef TIM14_ENABLE
static TIM_t gTIM_14 = {
	.Instance = TIM14
};
TIM_t * const TIM_14 = &gTIM_14;
#endif
#ifdef TIM16_ENABLE
static TIM_t gTIM_16 = {
	.Instance = TIM16
};
TIM_t * const TIM_16 = &gTIM_16;
#endif
#ifdef TIM17_ENABLE
static TIM_t gTIM_17 = {
	.Instance = TIM17
};
TIM_t * const TIM_17 = &gTIM_17;
#endif
#ifdef TIM21_ENABLE
static TIM_t gTIM_21 = {
	.Instance = TIM21
};
TIM_t * const TIM_21 = &gTIM_21;
#endif
#ifdef TIM22_ENABLE
static TIM_t gTIM_22 = {
	.Instance = TIM22
};
TIM_t * const TIM_22 = &gTIM_22;
#endif


/*
 * PUBLIC FUNCTIONS
 */

void TIM_Init(TIM_t * tim, uint32_t freq, uint32_t reload)
{
	TIMx_Init(tim);

	uint32_t cr1 = tim->Instance->CR1;
	cr1 &= ~(TIM_CR1_DIR | TIM_CR1_CMS | TIM_CR1_CKD | TIM_CR1_ARPE);
	cr1 |= TIM_AUTORELOAD_PRELOAD_ENABLE | TIM_CLOCKDIVISION_DIV1 | TIM_COUNTERMODE_UP;
	tim->Instance->CR1 = cr1;

	TIM_SetFreq(tim, freq);
	TIM_SetReload(tim, reload);

#ifdef __HAL_TIM_MOE_ENABLE
	// Main output enable, required for timers with a break/dead-time generator.
	__HAL_TIM_MOE_ENABLE(tim);
#endif
}

void TIM_SetFreq(TIM_t * tim, uint32_t freq)
{
	uint32_t clk = CLK_GetPCLKFreq();
	tim->Instance->PSC = (clk / freq) - 1;
}

void TIM_SetReload(TIM_t * tim, uint32_t reload)
{
	tim->Instance->ARR = reload;
}

#ifdef TIM_USE_IRQS
void TIM_OnReload(TIM_t * tim, VoidFunction_t callback)
{
	__HAL_TIM_ENABLE_IT(tim, TIM_IT_UPDATE);
	tim->ReloadCallback = callback;
}

void TIM_OnPulse(TIM_t * tim, TIM_Channel_t ch, VoidFunction_t callback)
{
	// WARN: This will fail horribly if ch is greater than 4.
	TIM_EnableOCx(tim, ch, TIM_OCMODE_ACTIVE);
	// Note that the channels IT's are 1 << 1 through 1 << 4
	__HAL_TIM_ENABLE_IT(tim, TIM_IT_CC1 << ch);
	tim->PulseCallback[ch] = callback;
}
#endif //TIM_USE_IRQS

void TIM_EnablePwm(TIM_t * tim, TIM_Channel_t ch, GPIO_Pin_t pins, uint32_t af)
{
	// TIM_CCMR1_OC1PE is the output compare preload
	TIM_EnableOCx(tim, ch, TIM_OCMODE_PWM2 | TIM_CCMR1_OC1PE | TIM_OCFAST_ENABLE);
	GPIO_EnableAlternate(pins, 0, af);
}


void TIM_SetPulse(TIM_t * tim, TIM_Channel_t ch, uint32_t pulse)
{
	switch (ch)
	{
	case TIM_CH1:
		tim->Instance->CCR1 = pulse;
		break;
	case TIM_CH2:
		tim->Instance->CCR2 = pulse;
		break;
	case TIM_CH3:
		tim->Instance->CCR3 = pulse;
		break;
	case TIM_CH4:
		tim->Instance->CCR4 = pulse;
		break;
	}

}

void TIM_Start(TIM_t * tim)
{
	TIM_Reload(tim);
	__HAL_TIM_ENABLE(tim);
}

void TIM_Stop(TIM_t * tim)
{
	__HAL_TIM_DISABLE(tim);
}

void TIM_Deinit(TIM_t * tim)
{
	__HAL_TIM_DISABLE(tim);
	__HAL_TIM_DISABLE_IT(tim, TIM_IT_UPDATE | TIM_IT_CC1 | TIM_IT_CC2 | TIM_IT_CC3 | TIM_IT_CC4);
	TIMx_Deinit(tim);
}

/*
 * PRIVATE FUNCTIONS
 */

static void TIM_EnableOCx(TIM_t * tim, uint32_t oc, uint32_t mode)
{
	// Disable the channel during the update.
	TIM_DISABLE_CCx(tim, oc);
	switch (oc)
	{
	case 0:
		MODIFY_REG(tim->Instance->CCMR1, TIM_CCMRx_MSK, mode);
		MODIFY_REG(tim->Instance->CCER, TIM_CCER_CC1P, TIM_OCPOLARITY_HIGH);
		break;
	case 1:
		MODIFY_REG(tim->Instance->CCMR1, TIM_CCMRx_MSK << 8, mode << 8);
		MODIFY_REG(tim->Instance->CCER, TIM_CCER_CC2P, TIM_OCPOLARITY_HIGH << 4);
		break;
	case 2:
		MODIFY_REG(tim->Instance->CCMR2, TIM_CCMRx_MSK, mode);
		MODIFY_REG(tim->Instance->CCER, TIM_CCER_CC3P, TIM_OCPOLARITY_HIGH << 8);
		break;
	case 3:
		MODIFY_REG(tim->Instance->CCMR2, TIM_CCMRx_MSK << 8, mode << 8);
		MODIFY_REG(tim->Instance->CCER, TIM_CCER_CC4P, TIM_OCPOLARITY_HIGH << 12);
		break;
	}
	TIM_ENABLE_CCx(tim, oc);
}

static void TIM_Reload(TIM_t * tim)
{
	// Disable all timer event sources.
	uint32_t itSources = tim->Instance->DIER;
	tim->Instance->DIER = 0;

	// Update the prescalar
	tim->Instance->EGR = TIM_EGR_UG;

	// Clear the event before it occurrs.
	__HAL_TIM_CLEAR_IT(tim, TIM_IT_UPDATE);
	tim->Instance->DIER = itSources;
}

static void TIMx_Init(TIM_t * tim)
{
#ifdef TIM1_ENABLE
	if (tim == TIM_1)
	{
		HAL_NVIC_EnableIRQ(TIM1_BRK_UP_TRG_COM_IRQn);
		HAL_NVIC_EnableIRQ(TIM1_CC_IRQn);
		__HAL_RCC_TIM1_CLK_ENABLE();
	}

#endif
#ifdef TIM2_ENABLE
	if (tim == TIM_2)
	{
		HAL_NVIC_EnableIRQ(TIM2_IRQn);
		__HAL_RCC_TIM2_CLK_ENABLE();
	}
#endif
#ifdef TIM3_ENABLE
	if (tim == TIM_3)
	{
		HAL_NVIC_EnableIRQ(TIM3_IRQn);
		__HAL_RCC_TIM3_CLK_ENABLE();
	}
#endif
#ifdef TIM5_ENABLE
	if (tim == TIM_5)
	{
		HAL_NVIC_EnableIRQ(TIM5_IRQn);
		__HAL_RCC_TIM5_CLK_ENABLE();
	}
#endif
#ifdef TIM6_ENABLE
	if (tim == TIM_6)
	{
		HAL_NVIC_EnableIRQ(TIM6_IRQn);
		__HAL_RCC_TIM6_CLK_ENABLE();
	}
#endif
#ifdef TIM14_ENABLE
	if (tim == TIM_14)
	{
		HAL_NVIC_EnableIRQ(TIM14_IRQn);
		__HAL_RCC_TIM14_CLK_ENABLE();
	}
#endif
#ifdef TIM16_ENABLE
	if (tim == TIM_16)
	{
		HAL_NVIC_EnableIRQ(TIM16_IRQn);
		__HAL_RCC_TIM16_CLK_ENABLE();
	}
#endif
#ifdef TIM17_ENABLE
	if (tim == TIM_17)
	{
		HAL_NVIC_EnableIRQ(TIM17_IRQn);
		__HAL_RCC_TIM17_CLK_ENABLE();
	}
#endif
#ifdef TIM21_ENABLE
	if (tim == TIM_21)
	{
		HAL_NVIC_EnableIRQ(TIM21_IRQn);
		__HAL_RCC_TIM21_CLK_ENABLE();
	}
#endif
#ifdef TIM22_ENABLE
	if (tim == TIM_22)
	{
		HAL_NVIC_EnableIRQ(TIM22_IRQn);
		__HAL_RCC_TIM22_CLK_ENABLE();
	}
#endif
}


static void TIMx_Deinit(TIM_t * tim)
{
#ifdef TIM1_ENABLE
	if (tim == TIM_1)
	{
		HAL_NVIC_DisableIRQ(TIM1_BRK_UP_TRG_COM_IRQn);
		HAL_NVIC_DisableIRQ(TIM1_CC_IRQn);
		__HAL_RCC_TIM1_CLK_DISABLE();
	}
#endif
#ifdef TIM2_ENABLE
	if (tim == TIM_2)
	{
		HAL_NVIC_DisableIRQ(TIM2_IRQn);
		__HAL_RCC_TIM2_CLK_DISABLE();
	}
#endif
#ifdef TIM3_ENABLE
	if (tim == TIM_3)
	{
		HAL_NVIC_DisableIRQ(TIM3_IRQn);
		__HAL_RCC_TIM3_CLK_DISABLE();
	}
#endif
#ifdef TIM5_ENABLE
	if (tim == TIM_5)
	{
		HAL_NVIC_DisableIRQ(TIM5_IRQn);
		__HAL_RCC_TIM5_CLK_DISABLE();
	}
#endif
#ifdef TIM6_ENABLE
	if (tim == TIM_6)
	{
		HAL_NVIC_DisableIRQ(TIM6_IRQn);
		__HAL_RCC_TIM6_CLK_DISABLE();
	}
#endif
#ifdef TIM14_ENABLE
	if (tim == TIM_14)
	{
		HAL_NVIC_DisableIRQ(TIM14_IRQn);
		__HAL_RCC_TIM14_CLK_DISABLE();
	}
#endif
#ifdef TIM16_ENABLE
	if (tim == TIM_16)
	{
		HAL_NVIC_DisableIRQ(TIM16_IRQn);
		__HAL_RCC_TIM16_CLK_DISABLE();
	}
#endif
#ifdef TIM17_ENABLE
	if (tim == TIM_17)
	{
		HAL_NVIC_DisableIRQ(TIM17_IRQn);
		__HAL_RCC_TIM17_CLK_DISABLE();
	}
#endif
#ifdef TIM21_ENABLE
	if (tim == TIM_21)
	{
		HAL_NVIC_DisableIRQ(TIM21_IRQn);
		__HAL_RCC_TIM21_CLK_DISABLE();
	}
#endif
#ifdef TIM22_ENABLE
	if (tim == TIM_22)
	{
		HAL_NVIC_DisableIRQ(TIM22_IRQn);
		__HAL_RCC_TIM22_CLK_DISABLE();
	}
#endif
}

/*
 * INTERRUPT ROUTINES
 */

#ifdef TIM_USE_IRQS

static void TIM_IRQHandler(TIM_t * tim)
{
	uint32_t irqs = TIM_GET_IRQ_SOURCES(tim);
	if(irqs & TIM_FLAG_CC1)
	{
		__HAL_TIM_CLEAR_IT(tim, TIM_IT_CC1);
		tim->PulseCallback[0]();
	}
	if(irqs & TIM_FLAG_CC2)
	{
		__HAL_TIM_CLEAR_IT(tim, TIM_IT_CC2);
		tim->PulseCallback[1]();
	}
	if(irqs & TIM_FLAG_CC3)
	{
		__HAL_TIM_CLEAR_IT(tim, TIM_IT_CC3);
		tim->PulseCallback[2]();
	}
	if(irqs & TIM_FLAG_CC4)
	{
		__HAL_TIM_CLEAR_IT(tim, TIM_IT_CC4);
		tim->PulseCallback[3]();
	}
	if(irqs & TIM_FLAG_UPDATE)
	{
		__HAL_TIM_CLEAR_IT(tim, TIM_IT_UPDATE);
		tim->ReloadCallback();
	}
}

#ifdef TIM1_ENABLE
void TIM1_BRK_UP_TRG_COM_IRQHandler(void)
{
	TIM_IRQHandler(TIM_1);
}
void TIM1_CC_IRQHandler(void)
{
	TIM_IRQHandler(TIM_1);
}
#endif
#ifdef TIM2_ENABLE
void TIM2_IRQHandler(void)
{
	TIM_IRQHandler(TIM_2);
}
#endif
#ifdef TIM3_ENABLE
void TIM3_IRQHandler(void)
{
	TIM_IRQHandler(TIM_3);
}
#endif
#ifdef TIM5_ENABLE
void TIM5_IRQHandler(void)
{
	TIM_IRQHandler(TIM_5);
}
#endif
#ifdef TIM6_ENABLE
void TIM6_IRQHandler(void)
{
	TIM_IRQHandler(TIM_6);
}
#endif
#ifdef TIM14_ENABLE
void TIM14_IRQHandler(void)
{
	TIM_IRQHandler(TIM_14);
}
#endif
#ifdef TIM16_ENABLE
void TIM16_IRQHandler(void)
{
	TIM_IRQHandler(TIM_16);
}
#endif
#ifdef TIM17_ENABLE
void TIM17_IRQHandler(void)
{
	TIM_IRQHandler(TIM_17);
}
#endif
#ifdef TIM21_ENABLE
void TIM21_IRQHandler(void)
{
	TIM_IRQHandler(TIM_21);
}
#endif
#ifdef TIM22_ENABLE
void TIM22_IRQHandler(void)
{
	TIM_IRQHandler(TIM_22);
}
#endif

#endif //TIM_USE_IRQS

