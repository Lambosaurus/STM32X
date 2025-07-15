
#include "Core.h"
#include "GPIO.h"
#include "CLK.h"
#include "US.h"

/*
 * PRIVATE DEFINITIONS
 */

#define _CORE_GET_RST_FLAGS()	(RCC->CSR)

#if defined(STM32L0)
#define _PWR_SET_PWR_REGULATOR(x)	(MODIFY_REG(PWR->CR, (PWR_CR_PDDS | PWR_CR_LPSDSR), x))

#if   (CLK_SYSCLK_FREQ <=  4194304)
#define CORE_VOLTAGE_RANGE					PWR_REGULATOR_VOLTAGE_SCALE3 // 1V2 core
#elif (CLK_SYSCLK_FREQ <= 16000000)
#define CORE_VOLTAGE_RANGE					PWR_REGULATOR_VOLTAGE_SCALE2 // 1V5 core
#else
#define CORE_VOLTAGE_RANGE					PWR_REGULATOR_VOLTAGE_SCALE1 // 1V8 core
#endif

#elif defined(STM32F0)
#define _PWR_SET_PWR_REGULATOR(x)			(MODIFY_REG(PWR->CR, PWR_CR_LPDS, x))

#elif defined(STM32G0)
#define _PWR_SET_PWR_REGULATOR(x)			(MODIFY_REG(PWR->CR1, PWR_CR1_LPR, x))
#define RCC_CSR_PORRSTF 					RCC_CSR_PWRRSTF

#define CORE_VOLTAGE_RANGE					PWR_REGULATOR_VOLTAGE_SCALE1
#define __HAL_PWR_VOLTAGESCALING_CONFIG		HAL_PWREx_ControlVoltageScaling

#elif defined(STM32WL)
#define CORE_VOLTAGE_RANGE					PWR_REGULATOR_VOLTAGE_SCALE1
#define _PWR_SET_PWR_REGULATOR(x)			(MODIFY_REG(PWR->CR1, PWR_CR1_LPR, x))
#define RCC_CSR_PORRSTF 					RCC_CSR_BORRSTF
#endif

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
#elif defined(STM32WL)
	//__HAL_FLASH_PREFETCH_BUFFER_ENABLE();
	//__HAL_FLASH_INSTRUCTION_CACHE_ENABLE();
#endif
#if !defined(STM32WL)
	__HAL_RCC_SYSCFG_CLK_ENABLE();
	__HAL_RCC_PWR_CLK_ENABLE();
#ifdef __HAL_PWR_VOLTAGESCALING_CONFIG
	__HAL_PWR_VOLTAGESCALING_CONFIG(CORE_VOLTAGE_RANGE);
#endif

#if defined(DEBUG) && (defined(STM32WL))
	HAL_DBGMCU_EnableDBGSleepMode();
	HAL_DBGMCU_EnableDBGStopMode();
#endif

	CLK_InitSYSCLK();
	CORE_InitSysTick();
	CORE_InitGPIO();
#ifdef	US_ENABLE
	US_Init();
#endif
}

void __attribute__ ((noinline)) CORE_Idle(void)
{
	// The push and pop of this function protects r0 from being clobbered during interrupt.
	// I do not understand why this is not preserved by the IRQ's push/pop.
	// If this function is inlined - then the usually pushed registers can get clobbered when returning from WFI.

	// As long as systick is on, this will at least return each millisecond.
	__WFI();
}


void CORE_Stop(void)
{
	// The tick may break the WFI if it occurs at the wrong time.
	HAL_SuspendTick();

#if defined(STM32WL)
	MODIFY_REG(PWR->CR1, PWR_CR1_LPMS, PWR_LOWPOWERMODE_STOP2);
#elif defined(STM32L0)
	SET_BIT(PWR->CR, PWR_CR_ULP | PWR_CR_FWU);
#endif

	// Select the low power regulator
	_PWR_SET_PWR_REGULATOR(PWR_LOWPOWERREGULATOR_ON);
#endif

	// WFI, but with the SLEEPDEEP bit set.
	SET_BIT(SCB->SCR, SCB_SCR_SLEEPDEEP_Msk);
	__WFI();
	CLEAR_BIT(SCB->SCR, SCB_SCR_SLEEPDEEP_Msk);
	_PWR_SET_PWR_REGULATOR(PWR_MAINREGULATOR_ON);

#ifdef STM32L0
	CLEAR_BIT(PWR->CR, PWR_CR_ULP | PWR_CR_FWU);
#endif

	// SYSCLK is defaulted to HSI on boot
	CLK_InitSYSCLK();
	HAL_ResumeTick();
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

void CORE_Reset(void)
{
	NVIC_SystemReset();
}

#ifdef CORE_USE_TICK_IRQ
void CORE_OnTick(VoidFunction_t callback)
{
	gTickCallback = callback;
}
#endif

CORE_ResetSource_t CORE_GetResetSource(void)
{
	uint32_t csr = _CORE_GET_RST_FLAGS();
	CORE_ResetSource_t src;
    if (csr & RCC_CSR_LPWRRSTF)
    {
    	src = CORE_ResetSource_Standby;
    }
    else if (csr & (RCC_CSR_WWDGRSTF | RCC_CSR_IWDGRSTF))
    {
    	// Join both watchdog sources together.
        src = CORE_ResetSource_Watchdog;
    }
    else if (csr & (RCC_CSR_SFTRSTF | RCC_CSR_OBLRSTF))
    {
    	// Joining Option byte load rst and software rst for now.
    	src = CORE_ResetSource_Software;
    }
    else if (csr & RCC_CSR_PORRSTF)
    {
    	src = CORE_ResetSource_PowerOn;
    }
    else if (csr & RCC_CSR_PINRSTF)
    {
    	src = CORE_ResetSource_Pin;
    }
    else
    {
        src = CORE_ResetSource_UNKNOWN;
    }
    // Flags will persist unless cleared
    __HAL_RCC_CLEAR_RESET_FLAGS();
    return src;
}

const uint32_t * CORE_GetUID(void)
{
	return (const uint32_t*)UID_BASE;
}

/*
 * PRIVATE FUNCTIONS
 */

void CORE_InitSysTick(void)
{
	HAL_SYSTICK_Config(CLK_GetHCLKFreq() / CORE_SYSTICK_FREQ);
	HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

void CORE_InitGPIO(void)
{
	__HAL_RCC_GPIOA_CLK_ENABLE();
#ifdef DEBUG
	// SWCLK and SWDIO on PA13, PA14
	GPIO_Deinit(GPIO_Port_A | (GPIO_Pin_All & ~(GPIO_Pin_13 | GPIO_Pin_14)));
#else
	GPIO_Deinit(GPIO_Port_A | GPIO_Pin_All);
#endif

	__HAL_RCC_GPIOB_CLK_ENABLE();
	GPIO_Deinit(GPIO_Port_B | GPIO_Pin_All);

	__HAL_RCC_GPIOC_CLK_ENABLE();
	GPIO_Deinit(GPIO_Port_C | GPIO_Pin_All);

#if defined(GPIOD)
	__HAL_RCC_GPIOD_CLK_ENABLE();
	GPIO_Deinit(GPIO_Port_D | GPIO_Pin_All);
#endif

#if defined(GPIOH)
	__HAL_RCC_GPIOH_CLK_ENABLE();
#endif
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

