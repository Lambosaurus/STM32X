
#include "Core.h"

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
static void CORE_InitSysClk(void);
static void CORE_InitSysTick(void);

/*
 * PRIVATE VARIABLES
 */

static volatile uint32_t gTicks = 0;

/*
 * PUBLIC FUNCTIONS
 */

void CORE_Init(void)
{
	__HAL_FLASH_PREREAD_BUFFER_ENABLE();
	__HAL_RCC_SYSCFG_CLK_ENABLE();
	__HAL_RCC_PWR_CLK_ENABLE();

	CORE_InitSysTick();
	CORE_InitSysClk();
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

uint32_t CORE_GetTick(void)
{
	return gTicks;
}

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

	GPIO_InitTypeDef gpio = {0};
	gpio.Mode = GPIO_MODE_ANALOG;
	gpio.Pull = GPIO_NOPULL;

	// SWCLK and SWDIO on PA13, PA14
	gpio.Pin = GPIO_PIN_All & ~(GPIO_PIN_13 | GPIO_PIN_14);
	HAL_GPIO_Init(GPIOA, &gpio);

	gpio.Pin = GPIO_PIN_All;
	HAL_GPIO_Init(GPIOB, &gpio);

	gpio.Pin = GPIO_PIN_All;
	HAL_GPIO_Init(GPIOC, &gpio);
}

void CORE_InitSysClk(void)
{
	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

	RCC_OscInitTypeDef osc = {0};
#ifdef CORE_USE_HSE
	osc.OscillatorType 		= RCC_OSCILLATORTYPE_HSE;
	osc.HSEState 			= RCC_HSE_ON;
	osc.PLL.PLLState 		= RCC_PLL_ON;
	osc.PLL.PLLSource 		= RCC_PLLSOURCE_HSE;
	osc.PLL.PLLMUL 			= RCC_PLL_MUL2;
	osc.PLL.PLLDIV 			= RCC_PLL_DIV1;
#else
	osc.OscillatorType 		= RCC_OSCILLATORTYPE_HSI;
	osc.HSIState 			= RCC_HSI_ON;
	osc.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
	osc.PLL.PLLState 		= RCC_PLL_ON;
	osc.PLL.PLLSource 		= RCC_PLLSOURCE_HSI;
	osc.PLL.PLLMUL 			= RCC_PLL_MUL4;
	osc.PLL.PLLDIV			= RCC_PLL_DIV2;

#endif //CORE_USE_HSE
	HAL_RCC_OscConfig(&osc);

	RCC_ClkInitTypeDef clk = {0};
	clk.ClockType 		= RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
	clk.SYSCLKSource 	= RCC_SYSCLKSOURCE_PLLCLK;
	clk.AHBCLKDivider 	= RCC_SYSCLK_DIV1;
	clk.APB1CLKDivider 	= RCC_HCLK_DIV1;
	clk.APB2CLKDivider  = RCC_HCLK_DIV1;
	HAL_RCC_ClockConfig(&clk, FLASH_LATENCY_1);
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
}

