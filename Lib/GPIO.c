
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

typedef enum {
	GPIO_MODE_Input		= 0x0000,
	GPIO_MODE_Output 	= 0x0001,
	GPIO_MODE_Alternate = 0x0002,
	GPIO_MODE_Analog    = 0x0003,
	GPIO_MODE_MASK		= 0x0003,

	// Only required when Output or Alternate
	GPIO_CFG_PushPull   = 0x0000,
	GPIO_CFG_OpenDrain  = 0x0010,

	// Only required when Output or Alternate
	GPIO_SPEED_Slow     = 0x0000,
	GPIO_SPEED_Medium	= 0x0100,
	GPIO_SPEED_Fast		= 0x0200,
	GPIO_SPEED_High		= 0x0300,
	GPIO_SPEED_MASK     = 0x0300,

	GPIO_PULL_None		= 0x0000,
	GPIO_PULL_Pullup    = 0x1000,
	GPIO_PULL_Pulldown  = 0x2000,
	GPIO_PULL_MASK		= 0x3000,

	// Only expected for inputs
	GPIO_IT_Rising 		= 0x010000,
	GPIO_IT_Falling 	= 0x020000,
	GPIO_IT_Both 		= 0x030000,
	GPIO_IT_MASK		= 0x030000

} GPIO_Config_t;


/*
 * PRIVATE PROTOTYPES
 */

#ifdef GPIO_USE_IRQS
static inline void EXTIx_IRQHandler(int n);
static void EXTIx_EnableIRQn(int n);
#endif //GPIO_USE_IRQS

static void GPIO_Init(GPIO_t * gpio, uint32_t pins, GPIO_Config_t mode, uint32_t af);

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
	GPIO_Write(gpio, pin, state);
	GPIO_Init(gpio, pin, GPIO_MODE_Output, 0);
}

void GPIO_EnableInput(GPIO_t * gpio, uint32_t pin, GPIO_Pull_t pull)
{
	GPIO_Init(gpio, pin, GPIO_MODE_Input | pull, 0);
}

void GPIO_EnableAlternate(GPIO_t * gpio, uint32_t pin, uint32_t mode, uint32_t af)
{
	GPIO_Init(gpio, pin, GPIO_MODE_Alternate | GPIO_SPEED_High | mode, af);
}

#ifdef GPIO_USE_IRQS
void GPIO_EnableIRQ(GPIO_t * gpio, uint32_t pin, GPIO_Pull_t pull, GPIO_IT_Dir_t dir, VoidFunction_t callback)
{
	int n = 0;
	while ((pin & (1 << n)) == 0) { n++; }

	gCallback[n] = callback;

	GPIO_InitTypeDef init = {
	  .Mode = dir,
	  .Pin = pin,
	  .Pull = pull,
	  .Speed = GPIO_SPEED_HIGH,
	};
	HAL_GPIO_Init(gpio, &init);
	EXTIx_EnableIRQn(n);
}
#endif //GPIO_USE_IRQS

void GPIO_Deinit(GPIO_t * gpio, uint32_t pin)
{
	GPIO_Init(gpio, pin, GPIO_MODE_Analog, 0);
}

/*
 * PRIVATE FUNCTIONS
 */




/*
#define SIMD_B0 	0b1010101010101010
#define SIMD_B1 	0b1100110011001100
#define SIMD_B2 	0b1111000011110000
#define SIMD_B3 	0b1111111100000000

#define SIMD_B0 	0xAAAAAAAA
#define SIMD_B1 	0x00CCCCCC
#define SIMD_B2 	0x00F0F0F0
#define SIMD_B3 	0x0000FF00

uint32_t GPIO_BitDouble(uint32_t pins)
{
	s = (s & ~SIMD_B3) | ((s & SIMD_B3) << 8);
	s = (s & ~SIMD_B2) | ((s & SIMD_B2) << 4);
	s = (s & ~SIMD_B1) | ((s & SIMD_B1) << 2);
	s = (s & ~SIMD_B0) | ((s & SIMD_B0) << 1);
	return s;
}
*/

static void GPIO_Init(GPIO_t * gpio, uint32_t pins, GPIO_Config_t mode, uint32_t af)
{
	GPIO_Config_t dir = mode & GPIO_MODE_MASK;
	uint32_t pos = 0;
	while ( pins )
	{
		uint32_t pin = (1 << pos);
		if (pins & pin)
		{
			if (dir == GPIO_MODE_Alternate)
			{
				uint32_t alt_offset = (pos & 0x7) * 4;
				MODIFY_REG(gpio->AFR[pos >> 3], (0xF << alt_offset), (af << alt_offset));
			}

			uint32_t offset = pos * 2;
			if (dir == GPIO_MODE_Alternate || dir == GPIO_MODE_Output)
			{
				uint32_t speed = (mode & GPIO_SPEED_MASK) >> 8;

				MODIFY_REG( gpio->OSPEEDR, GPIO_OSPEEDER_OSPEED0 << offset, speed << offset );

				gpio->OTYPER &= ~pin;
				if (mode & GPIO_CFG_OpenDrain)
				{
					gpio->OTYPER |= pin;
				}
			}

			MODIFY_REG( gpio->MODER, GPIO_MODER_MODE0 << offset, dir << offset );
			uint32_t pupd = (mode & GPIO_PULL_MASK) >> 12;
			MODIFY_REG( gpio->PUPDR, GPIO_PUPDR_PUPD0 << offset, pupd << offset );

			if (mode & GPIO_IT_MASK)
			{
				__HAL_RCC_SYSCFG_CLK_ENABLE();
				uint32_t gpio_index = GPIO_GET_INDEX(gpio);
				uint32_t exti_offset = (pos & 0x3) * 4;
				MODIFY_REG( SYSCFG->EXTICR[pos >> 2], 0xF << exti_offset, gpio_index << exti_offset );
				SET_BIT(EXTI->IMR, pos);
				MODIFY_REG(EXTI->RTSR, pos, (mode & GPIO_IT_RISING) ? pos : 0);
				MODIFY_REG(EXTI->FTSR, pos, (mode & GPIO_IT_FALLING) ? pos : 0);
			}
		}
		pos++;
	}
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
