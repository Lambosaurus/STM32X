
#include "CLK.h"

/*
 * PRIVATE DEFINITIONS
 */


// Define HSI frequency
#if defined(STM32L0)
#define CLK_HSI_FREQ						16000000

#define __CLK_PLL_CONFIG(src, mul, div)		__HAL_RCC_PLL_CONFIG(src, mul, div)

#define _PWR_IS_DBP_SET()					(PWR->CR & PWR_CR_DBP)
#define _PWR_SET_DBP()						(PWR->CR |= PWR_CR_DBP)

#define FLASH_LATENCY						FLASH_LATENCY_1

#elif defined(STM32F0)
#define CLK_HSI_FREQ						8000000
#define __CLK_PLL_CONFIG(src, mul, prediv)	__HAL_RCC_PLL_CONFIG(src, prediv, mul)

#define RCC_PLL_DIV1						RCC_PREDIV_DIV1
#define RCC_PLL_DIV2						RCC_PREDIV_DIV2
#define RCC_PLL_DIV3						RCC_PREDIV_DIV3
#define RCC_PLL_DIV4						RCC_PREDIV_DIV4
#define RCC_PLL_DIV5						RCC_PREDIV_DIV5
#define RCC_PLL_DIV6						RCC_PREDIV_DIV6
#define RCC_PLL_DIV7						RCC_PREDIV_DIV7
#define RCC_PLL_DIV8						RCC_PREDIV_DIV8

#define _PWR_IS_DBP_SET()					(PWR->CR & PWR_CR_DBP)
#define _PWR_SET_DBP()						(PWR->CR |= PWR_CR_DBP)

#define FLASH_LATENCY						FLASH_LATENCY_1

#elif defined(STM32G0) || defined(STM32WL)
#define CLK_HSI_FREQ						16000000

#define __CLK_PLL_CONFIG(src, mul, div)		__HAL_RCC_PLL_CONFIG(src, RCC_PLLM_DIV1, mul, RCC_PLLP_DIV2, RCC_PLLQ_DIV2, div)
// Todo: use this style of PLL config for other
#define RCC_PLL_MULX_IS_VALID(x)			(x >= 8 && x <= 86)
#define RCC_PLL_MULX(x)						(x)

#define RCC_PLL_DIVX_IS_VALID(x)			(x >= 2 && x <= 32)
#define RCC_PLL_DIVX(x)						((x - 1) << RCC_PLLCFGR_PLLR_Pos)

#define _PWR_IS_DBP_SET()					(PWR->CR1 & PWR_CR1_DBP)
#define _PWR_SET_DBP()						(PWR->CR1 |= PWR_CR1_DBP)

#define FLASH_LATENCY						FLASH_LATENCY_2

#if defined(SMT32G0)
#define RCC_PLL_SYSCLK						RCC_PLLRCLK
#endif

#define RCC_CSR_RTCSEL						RCC_BDCR_RTCSEL
#define CSR									BDCR

// If the RNG module is required - then crank it.
#define CLK_RNG_MSI_RANGE					RCC_MSIRANGE_6
#define CLK_WAKEUP_CLK 						RCC_STOP_WAKEUPCLOCK_HSI

#endif


#if defined(CLK_USE_MSI)

#define CLK_SYSCLK_SRC			RCC_SYSCLKSOURCE_MSI
#define CLK_MSI_RANGE			RCC_MSIRANGE_6
#if (CLK_SYSCLK_FREQ != 4194304)
#error "CLK_SYSCLK_FREQ must be 4194304 when CLK_USE_MSI is defined."
#endif

#elif defined(CLK_USE_HSE)

#define CLK_PLL_SRC_FREQ		CLK_HSE_FREQ
#define CLK_PLL_SRC				RCC_PLLSOURCE_HSE
#define CLK_SYSCLK_SRC			RCC_SYSCLKSOURCE_HSE

#else // CLK_USE_HSI

#define CLK_USE_HSI
#define CLK_PLL_SRC_FREQ		CLK_HSI_FREQ
#define CLK_PLL_SRC				RCC_PLLSOURCE_HSI
#define CLK_SYSCLK_SRC			RCC_SYSCLKSOURCE_HSI

#endif

// Is PLL required?
#if ((CLK_SYSCLK_FREQ != CLK_PLL_SRC_FREQ) && !(defined(RCC_SYSCLKSOURCE_MSI) && (CLK_SYSCLK_SRC == RCC_SYSCLKSOURCE_MSI)))

#if (defined(STM32G0) || defined(STM32WL)) && (CLK_PLL_SRC_FREQ > 16000000)
#error "Changes required change RCC_PLLM to keep the PLL input in a 4-16MHz range"
#endif

#define CLK_USE_PLL
#include "CLK_PLL.inl.h"
#undef 	CLK_SYSCLK_SRC
#define CLK_SYSCLK_SRC			RCC_SYSCLKSOURCE_PLLCLK

#endif

/*
 * PRIVATE TYPES
 */

/*
 * PRIVATE PROTOTYPES
 */

#ifdef CLK_USE_LSE
static void CLK_ResetBackupDomain(void);
#endif
static void CLK_AccessBackupDomain(void);

/*
 * PRIVATE VARIABLES
 */

/*
 * PUBLIC FUNCTIONS
 */

void CLK_InitSYSCLK(void)
{
	__HAL_FLASH_SET_LATENCY(FLASH_LATENCY);
#ifdef CLK_WAKEUP_CLK
	__HAL_RCC_WAKEUPSTOP_CLK_CONFIG(CLK_WAKEUP_CLK);
#endif

	/*
	 * ENABLE OSCILLATORS
	 * Enable any required oscillators
	 */

#ifdef CLK_USE_HSE
#ifdef CLK_HSE_BYPASS
	__HAL_RCC_HSE_CONFIG(RCC_HSE_BYPASS_PWR);
#else
	__HAL_RCC_HSE_CONFIG(RCC_HSE_ON);
#endif
	while(__HAL_RCC_GET_FLAG(RCC_FLAG_HSERDY) == 0U);
#endif
#ifdef CLK_USE_HSI
	__HAL_RCC_HSI_CALIBRATIONVALUE_ADJUST(RCC_HSICALIBRATION_DEFAULT);
	__HAL_RCC_HSI_ENABLE();
	while(__HAL_RCC_GET_FLAG(RCC_FLAG_HSIRDY) == 0U);
#endif
#ifdef CLK_USE_MSI
	__HAL_RCC_MSI_ENABLE();
	while(__HAL_RCC_GET_FLAG(RCC_FLAG_MSIRDY) == 0U);
	__HAL_RCC_MSI_RANGE_CONFIG(CLK_MSI_RANGE);
	__HAL_RCC_MSI_CALIBRATIONVALUE_ADJUST(RCC_MSICALIBRATION_DEFAULT);
#endif

#ifdef CLK_USE_PLL
	// PLL must be disables for configuration.
	__HAL_RCC_PLL_DISABLE();
	while(__HAL_RCC_GET_FLAG(RCC_FLAG_PLLRDY) != 0U);
	__CLK_PLL_CONFIG(CLK_PLL_SRC, CLK_PLL_MUL_CFG, CLK_PLL_DIV_CFG);
	__HAL_RCC_PLL_ENABLE();
#if defined(STM32G0) || defined(STM32WL)
	__HAL_RCC_PLLCLKOUT_ENABLE(RCC_PLL_SYSCLK);
#endif
	while(__HAL_RCC_GET_FLAG(RCC_FLAG_PLLRDY) == 0U);
#endif

	/*
	 * CONFIGURE CLOCKS
	 * Select the sources and dividers for internal clocks
	 */

	// Configure AHBCLK divider
	MODIFY_REG(RCC->CFGR, RCC_CFGR_HPRE, RCC_SYSCLK_DIV1);

	// Apply SYSCLK source
	__HAL_RCC_SYSCLK_CONFIG(CLK_SYSCLK_SRC);
#if ( defined(RCC_SYSCLKSOURCE_MSI) && CLK_SYSCLK_SRC == RCC_SYSCLKSOURCE_MSI)
	while (__HAL_RCC_GET_SYSCLK_SOURCE() != RCC_SYSCLKSOURCE_STATUS_MSI);
#elif (CLK_SYSCLK_SRC == RCC_SYSCLKSOURCE_HSI)
	while (__HAL_RCC_GET_SYSCLK_SOURCE() != RCC_SYSCLKSOURCE_STATUS_HSI);
#elif (CLK_SYSCLK_SRC == RCC_SYSCLKSOURCE_HSE)
	while (__HAL_RCC_GET_SYSCLK_SOURCE() != RCC_SYSCLKSOURCE_STATUS_HSE);
#elif (CLK_SYSCLK_SRC == RCC_SYSCLKSOURCE_PLLCLK)
	while (__HAL_RCC_GET_SYSCLK_SOURCE() != RCC_SYSCLKSOURCE_STATUS_PLLCLK);
#endif

	// Configure PCLK dividers (peripheral clock)


#if defined(STM32L0) || defined(STM32WL) || defined(STM32G0)
	// STM32L0's have a second PCLK. The shift by 3 is defined like this in the HAL.
	MODIFY_REG(RCC->CFGR, RCC_CFGR_PPRE1, RCC_HCLK_DIV1);
	MODIFY_REG(RCC->CFGR, RCC_CFGR_PPRE2, RCC_HCLK_DIV1 << 3);
#elif defined(STM32F0)
	MODIFY_REG(RCC->CFGR, RCC_CFGR_PPRE, RCC_HCLK_DIV1);
#endif


	/*
	 * DISABLE OSCILLATORS
	 * Unused oscillators should be turned off.
	 * Note they are disabled AFTER sysclk source is redirected
	 */

#ifndef CLK_USE_HSI
	__HAL_RCC_HSI_DISABLE();
#endif
#if (defined(RCC_SYSCLKSOURCE_MSI) && !defined(CLK_USE_MSI))
	__HAL_RCC_MSI_DISABLE();
#endif
}


#ifdef USB_ENABLE
void CLK_EnableUSBCLK(void)
{
	__HAL_RCC_USB_CONFIG(RCC_USBCLKSOURCE_HSI48);
	__HAL_RCC_HSI48_ENABLE();
	while(!__HAL_RCC_GET_FLAG(RCC_FLAG_HSI48RDY));
}
void CLK_DisableUSBCLK(void)
{
	__HAL_RCC_HSI48_DISABLE();
	// No need to wait for disable.
}
#endif


void CLK_EnableLSO(void)
{
#ifdef CLK_USE_LSE
	//__HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_HIGH);
	CLK_AccessBackupDomain();
#ifdef CLK_LSE_BYPASS
	__HAL_RCC_LSE_CONFIG(RCC_LSE_BYPASS);
#else
	__HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_LOW);
	__HAL_RCC_LSE_CONFIG(RCC_LSE_ON);
#endif
	while(!__HAL_RCC_GET_FLAG(RCC_FLAG_LSERDY));
	CLK_ResetBackupDomain();
	__HAL_RCC_RTC_CONFIG(RCC_RTCCLKSOURCE_LSE);
#else
	__HAL_RCC_LSI_ENABLE();
	while (!__HAL_RCC_GET_FLAG(RCC_FLAG_LSIRDY));
	CLK_AccessBackupDomain();
	__HAL_RCC_RTC_CONFIG(RCC_RTCCLKSOURCE_LSI);
#endif
}

void CLK_DisableLSO(void)
{
#ifdef CLK_USE_LSE
	__HAL_RCC_LSE_CONFIG(RCC_LSE_OFF);
#else
	__HAL_RCC_LSI_DISABLE();
#endif
}

void CLK_EnableADCCLK(void)
{
	// ADC CLK is driven off the HSI on STM32L0
#if (!defined(STM32F0)) && !defined(CLK_USE_HSI)
	__HAL_RCC_HSI_ENABLE();
	while(__HAL_RCC_GET_FLAG(RCC_FLAG_HSIRDY) == 0);
#endif

#if defined(STM32WL)
	__HAL_RCC_ADC_CONFIG(RCC_ADCCLKSOURCE_HSI);
#endif
}

void CLK_DisableADCCLK(void)
{
#if (!defined(STM32F0)) && !defined(CLK_USE_HSI)
	__HAL_RCC_HSI_DISABLE();
#endif
}

void CLK_EnableRNGCLK(void)
{
#if (defined(STM32G0) || defined(STM32WL)) && !defined(CLK_USE_MSI)
	__HAL_RCC_MSI_ENABLE();
	__HAL_RCC_MSI_RANGE_CONFIG(CLK_RNG_MSI_RANGE);
	while(__HAL_RCC_GET_FLAG(RCC_FLAG_MSIRDY) == 0U);
	__HAL_RCC_RNG_CONFIG(RCC_RNGCLKSOURCE_MSI);
#endif
}

void CLK_DisableRNGCLK(void)
{
#if (defined(STM32G0) || defined(STM32WL)) && !defined(CLK_USE_MSI)
	__HAL_RCC_MSI_DISABLE();
#endif
}

uint32_t CLK_SelectPrescalar(uint32_t src_freq, uint32_t div_min, uint32_t div_max, uint32_t * dst_freq)
{
	// TODO: This is really a log2 problem.
	// This can possibly be reduced to a call to FIRST_SET_BIT.

	uint32_t k = 0;
	uint32_t div = div_min;
	src_freq /= div_min;

	while (div <= div_max && src_freq > *dst_freq)
	{
		div <<= 2;
		src_freq >>= 1;
		k++;
	}

	*dst_freq = src_freq;
	return k;
}

/*
 * PRIVATE FUNCTIONS
 */

#ifdef CLK_USE_LSE
static void CLK_ResetBackupDomain(void)
{
	// RTC Clock selection can be changed only if the Backup Domain is reset
	uint32_t csr = (RCC->CSR & ~(RCC_CSR_RTCSEL));
	__HAL_RCC_BACKUPRESET_FORCE();
	__HAL_RCC_BACKUPRESET_RELEASE();
	RCC->CSR = csr;
}
#endif

static void CLK_AccessBackupDomain(void)
{
	// Get access to backup domain
	if (!_PWR_IS_DBP_SET())
	{
		_PWR_SET_DBP();
		while (!_PWR_IS_DBP_SET());
	}
}

/*
 * INTERRUPT ROUTINES
 */

