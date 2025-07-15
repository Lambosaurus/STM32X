
#include "GPIO.h"
#include "IRQ.h"

/*
 * PRIVATE DEFINITIONS
 */

#if defined(STM32F0)
#define GPIO_SPEED_LOW 			GPIO_SPEED_FREQ_LOW
#define GPIO_SPEED_MEDIUM 		GPIO_SPEED_FREQ_MEDIUM
#define GPIO_SPEED_HIGH 		GPIO_SPEED_FREQ_HIGH

#define GPIO_OSPEEDER_OSPEED0 	GPIO_OSPEEDER_OSPEEDR0
#define GPIO_MODER_MODE0		GPIO_MODER_MODER0
#define GPIO_PUPDR_PUPD0		GPIO_PUPDR_PUPDR0

#elif defined(STM32G0) || defined(STM32WL)
#define GPIO_OSPEEDER_OSPEED0	GPIO_OSPEEDR_OSPEED0

#define IMR						IMR1
#define RTSR					RTSR1
#define FTSR					FTSR1

#endif

#ifndef GPIO_IRQ_PRIO
#define GPIO_IRQ_PRIO	0
#endif

/*
 * PRIVATE TYPES
 */

typedef GPIO_TypeDef GPIO_t;

/*
 * PRIVATE PROTOTYPES
 */

#ifdef GPIO_USE_IRQS
static void GPIO_EnableIRQ(int n);
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
	int n = FIRST_BIT_INDEX(pin);

	gCallback[n] = callback;

	GPIO_ConfigInterrupt(pin >> 16, n, dir);

	GPIO_EnableIRQ(n);
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
#ifdef __HAL_RCC_SYSCFG_CLK_ENABLE
		__HAL_RCC_SYSCFG_CLK_ENABLE();
#endif

#if defined(STM32G0)
		uint32_t offset = (n & 0x3) * 8;
		MODIFY_REG(EXTI->EXTICR[n >> 2], 0xF << offset, gpio_index << offset);
#else
		uint32_t offset = (n & 0x3) * 4;
		MODIFY_REG(SYSCFG->EXTICR[n >> 2], 0xF << offset, gpio_index << offset);
#endif

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
static void GPIO_EnableIRQ(int n)
{
#if defined(STM32WL)
	if (n <= 0) 		{ IRQ_Enable(IRQ_No_EXTI0, GPIO_IRQ_PRIO); }
	else if (n <= 1) 	{ IRQ_Enable(IRQ_No_EXTI1, GPIO_IRQ_PRIO); }
	else if (n <= 2) 	{ IRQ_Enable(IRQ_No_EXTI2, GPIO_IRQ_PRIO); }
	else if (n <= 3) 	{ IRQ_Enable(IRQ_No_EXTI3, GPIO_IRQ_PRIO); }
	else if (n <= 4) 	{ IRQ_Enable(IRQ_No_EXTI4, GPIO_IRQ_PRIO); }
	else if (n <= 9) 	{ IRQ_Enable(IRQ_No_EXTI5, GPIO_IRQ_PRIO); }
	else 				{ IRQ_Enable(IRQ_No_EXTI10, GPIO_IRQ_PRIO); }
#else
	if (n <= 1) 		{ IRQ_Enable(IRQ_No_EXTI0, GPIO_IRQ_PRIO); }
	else if (n <= 3) 	{ IRQ_Enable(IRQ_No_EXTI2, GPIO_IRQ_PRIO); }
	else 				{ IRQ_Enable(IRQ_No_EXTI4, GPIO_IRQ_PRIO); }
#endif
}
#endif //GPIO_USE_IRQS

/*
 * INTERRUPT ROUTINES
 */

#ifdef GPIO_USE_IRQS
void GPIO_IRQHandler(uint32_t n)
{
	gCallback[n]();
}

#endif //GPIO_USE_IRQS
