
#include "GPIO.h"

/*
 * PRIVATE DEFINITIONS
 */

#ifdef STM32F0
#define GPIO_SPEED_LOW 		GPIO_SPEED_FREQ_LOW
#define GPIO_SPEED_MEDIUM 	GPIO_SPEED_FREQ_MEDIUM
#define GPIO_SPEED_HIGH 	GPIO_SPEED_FREQ_HIGH
#endif

/*
 * PRIVATE TYPES
 */

/*
 * PRIVATE PROTOTYPES
 */

#ifdef GPIO_USE_IRQS
static inline void EXTIx_IRQHandler(int n);
static void EXTIx_EnableIRQn(int n);
#endif //GPIO_USE_IRQS

/*
 * PRIVATE VARIABLES
 */

#ifdef GPIO_USE_IRQS
VoidFunction_t gCallback[16] = { 0 };
#endif //GPIO_USE_IRQS

/*
 * PUBLIC FUNCTIONS
 */

void GPIO_Write(GPIO_t * gpio, uint32_t pin, GPIO_PinState state)
{
	if (state != GPIO_PIN_RESET)
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

#ifdef GPIO_USE_IRQS
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
#endif //GPIO_USE_IRQS

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

#ifdef GPIO_USE_IRQS
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
#endif //GPIO_USE_IRQS

/*
 * INTERRUPT ROUTINES
 */

#ifdef GPIO_USE_IRQS
#if defined(GPIO_IRQ0_ENABLE) || defined(GPIO_IRQ1_ENABLE)
void EXTI0_1_IRQHandler(void)
{
#ifdef GPIO_IRQ0_ENABLE
	EXTIx_IRQHandler(0);
#endif
#ifdef GPIO_IRQ1_ENABLE
	EXTIx_IRQHandler(1);
#endif
}
#endif

#if defined(GPIO_IRQ2_ENABLE) || defined(GPIO_IRQ3_ENABLE)
void EXTI2_3_IRQHandler(void)
{
#ifdef GPIO_IRQ2_ENABLE
	EXTIx_IRQHandler(2);
#endif
#ifdef GPIO_IRQ3_ENABLE
	EXTIx_IRQHandler(3);
#endif
}
#endif

#if    defined(GPIO_IRQ4_ENABLE) || defined(GPIO_IRQ5_ENABLE) || defined(GPIO_IRQ6_ENABLE)   \
    || defined(GPIO_IRQ7_ENABLE) || defined(GPIO_IRQ8_ENABLE) || defined(GPIO_IRQ9_ENABLE)   \
	|| defined(GPIO_IRQ10_ENABLE) || defined(GPIO_IRQ11_ENABLE) || defined(GPIO_IRQ12_ENABLE)\
	|| defined(GPIO_IRQ13_ENABLE) || defined(GPIO_IRQ14_ENABLE) || defined(GPIO_IRQ15_ENABLE)
void EXTI4_15_IRQHandler(void)
{
#ifdef GPIO_IRQ4_ENABLE
	EXTIx_IRQHandler(4);
#endif
#ifdef GPIO_IRQ5_ENABLE
	EXTIx_IRQHandler(5);
#endif
#ifdef GPIO_IRQ6_ENABLE
	EXTIx_IRQHandler(6);
#endif
#ifdef GPIO_IRQ7_ENABLE
	EXTIx_IRQHandler(7);
#endif
#ifdef GPIO_IRQ8_ENABLE
	EXTIx_IRQHandler(8);
#endif
#ifdef GPIO_IRQ9_ENABLE
	EXTIx_IRQHandler(9);
#endif
#ifdef GPIO_IRQ10_ENABLE
	EXTIx_IRQHandler(10);
#endif
#ifdef GPIO_IRQ11_ENABLE
	EXTIx_IRQHandler(11);
#endif
#ifdef GPIO_IRQ12_ENABLE
	EXTIx_IRQHandler(12);
#endif
#ifdef GPIO_IRQ13_ENABLE
	EXTIx_IRQHandler(13);
#endif
#ifdef GPIO_IRQ14_ENABLE
	EXTIx_IRQHandler(14);
#endif
#ifdef GPIO_IRQ15_ENABLE
	EXTIx_IRQHandler(15);
#endif
}
#endif
#endif //GPIO_USE_IRQS
