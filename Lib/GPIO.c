
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

//#define GPIOCFG_PULL_POS		4
#define GPIOCFG_SPEED_POS		8
#define GPIOCFG_FLAG_POS		12

typedef enum {
	GPIOMode_Input		= 0x00,
	GPIOMode_Output 	= 0x01,
	GPIOMode_Alternate 	= 0x02,
	GPIOMode_Analog    	= 0x03,
	GPIOMode_MASK		= 0x03,

	GPIOPull_None 		= GPIO_PULL_NONE,
	GPIOPull_Up 		= GPIO_PULL_UP,
	GPIOPull_Down 		= GPIO_PULL_DOWN,
	GPIOPull_MASK  		= GPIO_PULL_UP | GPIO_PULL_DOWN,

	// Only required when Output or Alternate
	GPIOSpeed_Slow     	= 0x00 << GPIOCFG_SPEED_POS,
	GPIOSpeed_Medium	= 0x01 << GPIOCFG_SPEED_POS,
	GPIOSpeed_Fast		= 0x02 << GPIOCFG_SPEED_POS,
	GPIOSpeed_High		= 0x03 << GPIOCFG_SPEED_POS,
	GPIOSpeed_MASK		= 0x03 << GPIOCFG_SPEED_POS,

	// Only relevant when Output or Alternate
	GPIOFlag_OpenDrain  = 0x01 << GPIOCFG_FLAG_POS,
} GPIOCfg_t;


/*
 * PRIVATE PROTOTYPES
 */

#ifdef GPIO_USE_IRQS
static inline void EXTIx_IRQHandler(int n);
static void EXTIx_EnableIRQn(int n);
#endif //GPIO_USE_IRQS

static void GPIO_Init(GPIO_t * gpio, uint32_t pins, GPIOCfg_t mode);
static inline void GPIO_ConfigInterrupt( GPIO_t * gpio, int n, GPIO_IT_Dir_t dir);
static inline void GPIO_ConfigAlternate( GPIO_t * gpio, uint32_t pins, uint32_t af);

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
	GPIO_Init(gpio, pin, GPIOMode_Output);
}

void GPIO_EnableInput(GPIO_t * gpio, uint32_t pin, GPIO_Pull_t pull)
{
	GPIO_Init(gpio, pin, GPIOMode_Input | pull);
}

void GPIO_EnableAlternate(GPIO_t * gpio, uint32_t pin, bool opendrain, uint32_t af)
{
	GPIO_ConfigAlternate(gpio, pin, af);
	GPIO_Init(gpio, pin, GPIOMode_Alternate | GPIOSpeed_High | (opendrain ? GPIOFlag_OpenDrain : 0));
}

#ifdef GPIO_USE_IRQS
void GPIO_EnableIRQ(GPIO_t * gpio, uint32_t pin, GPIO_Pull_t pull, GPIO_IT_Dir_t dir, VoidFunction_t callback)
{
	int n = 0;
	while ((pin & (1 << n)) == 0) { n++; }

	gCallback[n] = callback;

	GPIO_Init(gpio, pin, GPIOMode_Input | pull);
	GPIO_ConfigInterrupt(gpio, pin, dir);

	EXTIx_EnableIRQn(n);
}
#endif //GPIO_USE_IRQS

void GPIO_Deinit(GPIO_t * gpio, uint32_t pin)
{
	GPIO_Init(gpio, pin, GPIOMode_Analog);
}

/*
 * PRIVATE FUNCTIONS
 */


static inline void GPIO_ConfigAlternate( GPIO_t * gpio, uint32_t pins, uint32_t af)
{
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

static inline void GPIO_ConfigInterrupt( GPIO_t * gpio, int n, GPIO_IT_Dir_t dir)
{
	__HAL_RCC_SYSCFG_CLK_ENABLE();
	uint32_t gpio_index = GPIO_GET_INDEX(gpio);
	uint32_t offset = (4 * n & 0x3);
	MODIFY_REG(SYSCFG->EXTICR[n >> 2], 0xF << offset, gpio_index << offset);

	uint32_t pin = 1 << n;
	SET_BIT(EXTI->IMR, pin);
	MODIFY_REG(EXTI->RTSR, pin, (dir & GPIO_IT_RISING) ? pin : 0);
	MODIFY_REG(EXTI->FTSR, pin, (dir & GPIO_IT_FALLING) ? pin : 0);
}



#define SIMD_B0 	0xAAAAAAAA
#define SIMD_B1 	0x00CCCCCC
#define SIMD_B2 	0x00F0F0F0
#define SIMD_B3 	0x0000FF00

static inline uint32_t GPIO_BitDouble(uint32_t s)
{
	s = (s & ~SIMD_B3) | ((s & SIMD_B3) << 8);
	s = (s & ~SIMD_B2) | ((s & SIMD_B2) << 4);
	s = (s & ~SIMD_B1) | ((s & SIMD_B1) << 2);
	s = (s & ~SIMD_B0) | ((s & SIMD_B0) << 1);
	return s;
}

static void GPIO_Init(GPIO_t * gpio, uint32_t pins, GPIOCfg_t mode)
{
	uint32_t pinmask = GPIO_BitDouble(pins);

	GPIOCfg_t dir = mode & GPIOMode_MASK;

	if (dir == GPIOMode_Alternate || dir == GPIOMode_Output)
	{
		uint32_t speed = (mode & GPIOSpeed_MASK) >> GPIOCFG_SPEED_POS;
		MODIFY_REG( gpio->OSPEEDR, pinmask * GPIO_OSPEEDER_OSPEED0, pinmask * speed );
		MODIFY_REG( gpio->OTYPER, pins, (mode & GPIOFlag_OpenDrain) ? pins : 0 );
	}

	MODIFY_REG( gpio->MODER, pinmask * GPIO_MODER_MODE0, pinmask * dir);
	uint32_t pull = (mode & GPIOPull_MASK) >> GPIOCFG_PULL_POS;
	MODIFY_REG( gpio->PUPDR, pinmask * GPIO_PUPDR_PUPD0, pinmask * pull);
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
