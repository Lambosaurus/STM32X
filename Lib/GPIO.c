
#include "GPIO.h"

/*
 * PRIVATE DEFINITIONS
 */

#ifdef STM32F0
#define GPIO_SPEED_LOW 		GPIO_SPEED_FREQ_LOW
#define GPIO_SPEED_MEDIUM 	GPIO_SPEED_FREQ_MEDIUM
#define GPIO_SPEED_HIGH 	GPIO_SPEED_FREQ_HIGH

#define GPIO_OSPEEDER_OSPEED0 	GPIO_OSPEEDER_OSPEEDR0
#define GPIO_MODER_MODE0		GPIO_MODER_MODER0
#define GPIO_PUPDR_PUPD0		GPIO_PUPDR_PUPDR0
#endif

/*
 * PRIVATE TYPES
 */

typedef GPIO_TypeDef GPIO_t;

/*
 * PRIVATE PROTOTYPES
 */

#ifdef GPIO_USE_IRQS
static inline void EXTIx_IRQHandler(int n);
static void EXTIx_EnableIRQn(int n);
static void GPIO_ConfigInterrupt(int gpio_index, int n, GPIO_IT_Dir_t dir);
#endif //GPIO_USE_IRQS

static void GPIO_ConfigAlternate(GPIO_Pin_t pins, uint32_t af);

static uint32_t GPIO_SWARBitDouble(uint32_t s);
static inline GPIO_t * GPIO_GetPort(GPIO_Pin_t pins);

/*
 * PRIVATE VARIABLES
 */

#ifdef GPIO_USE_IRQS
VoidFunction_t gCallback[16] = { 0 };
#endif //GPIO_USE_IRQS

/*
 * PUBLIC FUNCTIONS
 */

void GPIO_Write(GPIO_Pin_t pins, GPIO_State_t state)
{
	if (state)
	{
		GPIO_Set(pins);
	}
	else
	{
		GPIO_Reset(pins);
	}
}

void GPIO_EnableAlternate(GPIO_Pin_t pins, GPIO_Flag_t flags, uint32_t af)
{
	GPIO_ConfigAlternate(pins, af);
	GPIO_Init(pins, GPIO_Mode_Alternate | GPIO_Speed_High | flags);
}

#ifdef GPIO_USE_IRQS
void GPIO_OnChange(GPIO_Pin_t pin, GPIO_IT_Dir_t dir, VoidFunction_t callback)
{
	int n = 0;
	while ((pin & (1 << n)) == 0) { n++; }

	gCallback[n] = callback;

	GPIO_ConfigInterrupt(pin >> 16, n, dir);

	EXTIx_EnableIRQn(n);
}
#endif //GPIO_USE_IRQS

void GPIO_Init(GPIO_Pin_t pins, GPIO_Flag_t mode)
{
	GPIO_t * gpio = GPIO_GetPort(pins);
	pins &= GPIO_Pin_All;
	uint32_t pinmask = GPIO_SWARBitDouble(pins);

	GPIO_Mode_t dir = mode & GPIO_Mode_MASK;

	if (dir == GPIO_Mode_Alternate || dir == GPIO_Mode_Output)
	{
		uint32_t speed = (mode & GPIO_Speed_MASK) >> GPIOCFG_SPEED_POS;
		MODIFY_REG( gpio->OSPEEDR, pinmask * GPIO_OSPEEDER_OSPEED0, pinmask * speed );
		MODIFY_REG( gpio->OTYPER, pins, (mode & GPIO_Flag_OpenDrain) ? pins : 0 );
	}

	MODIFY_REG( gpio->MODER, pinmask * GPIO_MODER_MODE0, pinmask * dir);
	uint32_t pull = (mode & GPIO_Pull_MASK) >> GPIOCFG_PULL_POS;
	MODIFY_REG( gpio->PUPDR, pinmask * GPIO_PUPDR_PUPD0, pinmask * pull);
}

void GPIO_Set(GPIO_Pin_t pins)
{
	GPIO_GetPort(pins)->BSRR = pins & GPIO_Pin_All;
}

void GPIO_Reset(GPIO_Pin_t pins)
{
	GPIO_GetPort(pins)->BRR = pins & GPIO_Pin_All;
}

GPIO_State_t GPIO_Read(GPIO_Pin_t pins)
{
	return (GPIO_GetPort(pins)->IDR & (pins & GPIO_Pin_All)) > 0;
}

GPIO_Pin_t GPIO_ReadPort(GPIO_Pin_t pins)
{
	return GPIO_GetPort(pins)->IDR & pins;
}

/*
 * PRIVATE FUNCTIONS
 */

static inline GPIO_t * GPIO_GetPort(GPIO_Pin_t pins)
{
	uint32_t index = pins >> 16;
	return (GPIO_t*)(GPIOA_BASE + (index * (GPIOB_BASE - GPIOA_BASE)));
}

static void GPIO_ConfigAlternate(GPIO_Pin_t pins, uint32_t af)
{
	GPIO_t * gpio = GPIO_GetPort(pins);
	pins &= GPIO_Pin_All;
	uint32_t pos = 0;
	while (pins)
	{
		if (pins & 0x1)
		{
			uint32_t alt_offset = (pos & 0x7) * 4;
			MODIFY_REG(gpio->AFR[pos >> 3], (0xF << alt_offset), (af << alt_offset));
		}
		pins >>= 1;
		pos++;
	}
}

#ifdef GPIO_USE_IRQS
static void GPIO_ConfigInterrupt(int gpio_index, int n, GPIO_IT_Dir_t dir)
{
	uint32_t pin = 1 << n;
	if (dir == GPIO_IT_None)
	{
		// Disable the EXTI channel.
		CLEAR_BIT(EXTI->IMR, pin);
	}
	else
	{
		// Assign the EXTI channel to the given GPIO.
		__HAL_RCC_SYSCFG_CLK_ENABLE();
		uint32_t offset = (n & 0x3) * 4;
		MODIFY_REG(SYSCFG->EXTICR[n >> 2], 0xF << offset, gpio_index << offset);

		// Configure the EXTI channel
		SET_BIT(EXTI->IMR, pin);
		MODIFY_REG(EXTI->RTSR, pin, (dir & GPIO_IT_Rising) ? pin : 0);
		MODIFY_REG(EXTI->FTSR, pin, (dir & GPIO_IT_Falling) ? pin : 0);
	}
}
#endif

static uint32_t GPIO_SWARBitDouble(uint32_t s)
{
	s = (s & ~0xFF00FF00) | ((s & 0xFF00FF00) << 8);
	s = (s & ~0xF0F0F0F0) | ((s & 0xF0F0F0F0) << 4);
	s = (s & ~0xCCCCCCCC) | ((s & 0xCCCCCCCC) << 2);
	s = (s & ~0xAAAAAAAA) | ((s & 0xAAAAAAAA) << 1);
	return s;
}


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
