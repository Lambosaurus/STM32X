
#include "GPIO.h"

/*
 * PRIVATE DEFINITIONS
 */

/*
 * PRIVATE TYPES
 */

/*
 * PRIVATE PROTOTYPES
 */

#ifdef USE_GPIO_IRQS
static inline void EXTIx_IRQHandler(int n);
static void EXTIx_EnableIRQn(int n);
static void EXTIx_DefaultHandler(void);
#endif //USE_GPIO_IRQS

/*
 * PRIVATE VARIABLES
 */

#ifdef USE_GPIO_IRQS
VoidFunction_t gCallback[16] = {
	EXTIx_DefaultHandler,
	EXTIx_DefaultHandler,
	EXTIx_DefaultHandler,
	EXTIx_DefaultHandler,
	EXTIx_DefaultHandler,
	EXTIx_DefaultHandler,
	EXTIx_DefaultHandler,
	EXTIx_DefaultHandler,
	EXTIx_DefaultHandler,
	EXTIx_DefaultHandler,
	EXTIx_DefaultHandler,
	EXTIx_DefaultHandler,
	EXTIx_DefaultHandler,
	EXTIx_DefaultHandler,
	EXTIx_DefaultHandler,
	EXTIx_DefaultHandler,
};
#endif //USE_GPIO_IRQS

/*
 * PUBLIC FUNCTIONS
 */

void GPIO_Write(GPIO_t * gpio, uint32_t pin, GPIO_PinState state)
{
	if (state == GPIO_PIN_SET)
	{
		GPIO_Set(gpio, pin);
	}
	else
	{
		GPIO_Reset(gpio, pin);
	}
}

void GPIO_EnableOutput(GPIO_t * gpio, uint32_t pin, GPIO_PinState state)
{
	GPIO_InitTypeDef init = {
	  .Mode = GPIO_MODE_OUTPUT_PP,
	  .Pin = pin,
	  .Pull = GPIO_NOPULL,
	  .Speed = GPIO_SPEED_LOW,
	};
	HAL_GPIO_Init(gpio, &init);
	GPIO_Write(gpio, pin, state);
}

void GPIO_EnableInput(GPIO_t * gpio, uint32_t pin, uint32_t pullup)
{
	GPIO_InitTypeDef init = {
	  .Mode = GPIO_MODE_INPUT,
	  .Pin = pin,
	  .Pull = pullup,
	  .Speed = GPIO_SPEED_LOW,
	};
	HAL_GPIO_Init(gpio, &init);
}

#ifdef USE_GPIO_IRQS
void GPIO_EnableIRQ(GPIO_t * gpio, uint32_t pin, uint32_t pullup, GPIO_IT_Dir_t dir, VoidFunction_t callback)
{
	int n = 0;
	while ((pin & (1 << n)) == 0) { n++; }

	gCallback[n] = callback;

	GPIO_InitTypeDef init = {
	  .Mode = dir,
	  .Pin = pin,
	  .Pull = pullup,
	  .Speed = GPIO_SPEED_HIGH,
	};
	HAL_GPIO_Init(gpio, &init);
	EXTIx_EnableIRQn(n);
}
#endif //USE_GPIO_IRQS

void GPIO_Disable(GPIO_t * gpio, uint32_t pin)
{
	GPIO_InitTypeDef init = {
	  .Mode = GPIO_MODE_ANALOG,
	  .Pin = pin,
	  .Pull = GPIO_NOPULL,
	  .Speed = GPIO_SPEED_LOW,
	};
	HAL_GPIO_Init(gpio, &init);
}

/*
 * PRIVATE FUNCTIONS
 */

#ifdef USE_GPIO_IRQS
static inline void EXTIx_IRQHandler(int n)
{
	if (__HAL_GPIO_EXTI_GET_IT(1 << n) != RESET)
	{
		__HAL_GPIO_EXTI_CLEAR_IT(1 << n);
		gCallback[n]();
	}
}

static void EXTIx_EnableIRQn(int n)
{
	if (n <= 1)
	{
		HAL_NVIC_EnableIRQ(EXTI0_1_IRQn);
	}
	else if (n <= 3)
	{
		HAL_NVIC_EnableIRQ(EXTI2_3_IRQn);
	}
	else
	{
		HAL_NVIC_EnableIRQ(EXTI4_15_IRQn);
	}
}

static void EXTIx_DefaultHandler(void)
{
#ifdef DEBUG
	__BKPT();
#endif
}
#endif //USE_GPIO_IRQS

/*
 * INTERRUPT ROUTINES
 */

#ifdef USE_GPIO_IRQS
#if defined(USE_EXTI_0) || defined(USE_EXTI_1)
void EXTI0_1_IRQHandler(void)
{
#ifdef USE_EXTI_0
	EXTIx_IRQHandler(0);
#endif
#ifdef USE_EXTI_1
	EXTIx_IRQHandler(1);
#endif
}
#endif

#if defined(USE_EXTI_2) || defined(USE_EXTI_3)
void EXTI2_3_IRQHandler(void)
{
#ifdef USE_EXTI_2
	EXTIx_IRQHandler(2);
#endif
#ifdef USE_EXTI_3
	EXTIx_IRQHandler(3);
#endif
}
#endif

#if    defined(USE_EXTI_4) || defined(USE_EXTI_5) || defined(USE_EXTI_6)   \
    || defined(USE_EXTI_7) || defined(USE_EXTI_8) || defined(USE_EXTI_9)   \
	|| defined(USE_EXTI_10) || defined(USE_EXTI_11) || defined(USE_EXTI_12)\
	|| defined(USE_EXTI_13) || defined(USE_EXTI_14) || defined(USE_EXTI_15)
void EXTI4_15_IRQHandler(void)
{
#ifdef USE_EXTI_4
	EXTIx_IRQHandler(4);
#endif
#ifdef USE_EXTI_5
	EXTIx_IRQHandler(5);
#endif
#ifdef USE_EXTI_6
	EXTIx_IRQHandler(6);
#endif
#ifdef USE_EXTI_7
	EXTIx_IRQHandler(7);
#endif
#ifdef USE_EXTI_8
	EXTIx_IRQHandler(8);
#endif
#ifdef USE_EXTI_9
	EXTIx_IRQHandler(9);
#endif
#ifdef USE_EXTI_10
	EXTIx_IRQHandler(10);
#endif
#ifdef USE_EXTI_11
	EXTIx_IRQHandler(11);
#endif
#ifdef USE_EXTI_12
	EXTIx_IRQHandler(12);
#endif
#ifdef USE_EXTI_13
	EXTIx_IRQHandler(13);
#endif
#ifdef USE_EXTI_14
	EXTIx_IRQHandler(14);
#endif
#ifdef USE_EXTI_15
	EXTIx_IRQHandler(15);
#endif
}
#endif
#endif //USE_GPIO_IRQS
