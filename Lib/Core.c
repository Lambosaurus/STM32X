
#include "Core.h"
#include "GPIO.h"
#include "CLK.h"

/*
 * PRIVATE DEFINITIONS
 */

#define CORE_SYSTICK_FREQ	1000
#define MS_PER_SYSTICK		(1000 / CORE_SYSTICK_FREQ)

/*
 * PRIVATE TYPES
 */

/*
 * PRIVATE PROTOTYPES
 */

static void CORE_InitGPIO(void);
static void CORE_InitSysTick(void);

/*
 * PRIVATE VARIABLES
 */

#ifdef CORE_USE_TICK_IRQ
static VoidFunction_t gTickCallback;
#endif

volatile uint32_t gTicks = 0;

/*
 * PUBLIC FUNCTIONS
 */

void CORE_Init(void)
{
#if defined(STM32L0)
	__HAL_FLASH_PREREAD_BUFFER_ENABLE();
#elif defined(STM32F0)
	__HAL_FLASH_PREFETCH_BUFFER_ENABLE();
#endif
	__HAL_RCC_SYSCFG_CLK_ENABLE();
	__HAL_RCC_PWR_CLK_ENABLE();
#ifdef STM32L0
	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
#endif

	CLK_Init();
	CORE_InitSysTick();
	CORE_InitGPIO();
}

void CORE_Idle(void)
{
	// As long as systick is on, this will at least return each millisecond.
	__WFI();
}

void CORE_Delay(uint32_t ms)
{
	ms += MS_PER_SYSTICK; // Add to guarantee a minimum delay
	uint32_t start = CORE_GetTick();
	while (CORE_GetTick() - start < ms)
	{
		CORE_Idle();
	}
}

#ifdef CORE_USE_TICK_IRQ
void CORE_OnTick(VoidFunction_t callback)
{
	gTickCallback = callback;
}
#endif

/*
 * PRIVATE FUNCTIONS
 */

void CORE_InitSysTick(void)
{
	HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq() / CORE_SYSTICK_FREQ);
	HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

void CORE_InitGPIO(void)
{
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();

	// SWCLK and SWDIO on PA13, PA14
	GPIO_Deinit(GPIOA, GPIO_PIN_All & ~(GPIO_PIN_13 | GPIO_PIN_14));
	GPIO_Deinit(GPIOB, GPIO_PIN_All);
	GPIO_Deinit(GPIOC, GPIO_PIN_All);
}

/*
 * CALLBACK FUNCTIONS
 */

uint32_t HAL_GetTick(void)
{
	return gTicks;
}

/*
 * INTERRUPT ROUTINES
 */

void SysTick_Handler(void)
{
	gTicks += MS_PER_SYSTICK;

#ifdef CORE_USE_TICK_IRQ
	if (gTickCallback != NULL)
	{
		gTickCallback();
	}
#endif
}

