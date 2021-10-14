
#include "CLK.h"

/*
 * PRIVATE DEFINITIONS
 */


// Define HSI frequency
#if defined(STM32L0)
#define CLK_HSI_FREQ			16000000
#elif defined(STM32F0)
#define CLK_HSI_FREQ			8000000
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

#else

#define CLK_USE_HSI				// This is the default case
#define CLK_PLL_SRC_FREQ		CLK_HSI_FREQ
#define CLK_PLL_SRC				RCC_PLLSOURCE_HSI
#define CLK_SYSCLK_SRC			RCC_SYSCLKSOURCE_HSI

#endif


// Is PLL required?
#if ((CLK_SYSCLK_FREQ != CLK_PLLSRC_FREQ) && !(CLK_SYSCLK_SRC == RCC_SYSCLKSOURCE_MSI))
#define CLK_USE_PLL

#define CLK_PLL_MUL				4
#define CLK_PLL_DIV				((CLK_PLL_SRC_FREQ * CLK_PLL_MUL) / CLK_SYSCLK_FREQ)
#if (((CLK_PLL_SRC_FREQ * CLK_PLL_MUL) / CLK_PLL_DIV) != CLK_SYSCLK_FREQ)
#error "CLK_SYSCLK_FREQ too difficult to achieve"
#endif
#undef 	CLK_SYSCLK_SRC
#define CLK_SYSCLK_SRC			RCC_SYSCLKSOURCE_PLLCLK

// Select the PLL MUL/DIV (There must be a smarter way)
#if (CLK_PLL_MUL == 2)
#define CLK_PLL_MUL_CFG			RCC_PLL_MUL2
#elif (CLK_PLL_MUL == 3)
#define CLK_PLL_MUL_CFG			RCC_PLL_MUL3
#elif (CLK_PLL_MUL == 4)
#define CLK_PLL_MUL_CFG			RCC_PLL_MUL4
#elif (CLK_PLL_MUL == 6)
#define CLK_PLL_MUL_CFG			RCC_PLL_MUL6
#elif (CLK_PLL_MUL == 8)
#define CLK_PLL_MUL_CFG			RCC_PLL_MUL8
#elif (CLK_PLL_MUL == 12)
#define CLK_PLL_MUL_CFG			RCC_PLL_MUL12
#elif (CLK_PLL_MUL == 16)
#define CLK_PLL_MUL_CFG			RCC_PLL_MUL16
#elif (CLK_PLL_MUL == 32)
#define CLK_PLL_MUL_CFG			RCC_PLL_MUL32
#elif (CLK_PLL_MUL == 48)
#define CLK_PLL_MUL_CFG			RCC_PLL_MUL48
#else
#error "Unavailable PLL multiplier"
#endif

#if (CLK_PLL_DIV == 1)
#define CLK_PLL_DIV_CFG			RCC_PLL_DIV1
#elif (CLK_PLL_DIV == 2)
#define CLK_PLL_DIV_CFG			RCC_PLL_DIV2
#elif (CLK_PLL_DIV == 3)
#define CLK_PLL_DIV_CFG			RCC_PLL_DIV3
#elif (CLK_PLL_DIV == 4)
#define CLK_PLL_DIV_CFG			RCC_PLL_DIV4
#else
#error "Unavailable PLL divider"
#endif

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
	__HAL_FLASH_SET_LATENCY(FLASH_LATENCY_1);

	/*
	 * ENABLE OSCILLATORS
	 * Enable any required oscillators
	 */

#ifdef CLK_USE_HSE
	__HAL_RCC_HSE_CONFIG(RCC_HSE_ON);
	while(__HAL_RCC_GET_FLAG(RCC_FLAG_HSERDY) == 0U);
#endif
#ifdef CLK_USE_HSI
	__HAL_RCC_HSI_CALIBRATIONVALUE_ADJUST(RCC_HSICALIBRATION_DEFAULT);
	__HAL_RCC_HSI_CONFIG(RCC_HSI_ON);
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
	__HAL_RCC_PLL_CONFIG(CLK_PLL_SRC, CLK_PLL_MUL_CFG, CLK_PLL_DIV_CFG);
	__HAL_RCC_PLL_ENABLE();
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
#if (CLK_SYSCLK_SRC == RCC_SYSCLKSOURCE_MSI)
	while (__HAL_RCC_GET_SYSCLK_SOURCE() != RCC_SYSCLKSOURCE_STATUS_MSI);
#elif (CLK_SYSCLK_SRC == RCC_SYSCLKSOURCE_STATUS_HSI)
	while (__HAL_RCC_GET_SYSCLK_SOURCE() != RCC_SYSCLKSOURCE_STATUS_HSI);
#elif (CLK_SYSCLK_SRC == RCC_SYSCLKSOURCE_STATUS_HSE)
	while (__HAL_RCC_GET_SYSCLK_SOURCE() != RCC_SYSCLKSOURCE_STATUS_HSE);
#elif (CLK_SYSCLK_SRC == RCC_SYSCLKSOURCE_PLLCLK)
	while (__HAL_RCC_GET_SYSCLK_SOURCE() != RCC_SYSCLKSOURCE_STATUS_PLLCLK);
#endif

	// Configure PCLK dividers (peripheral clock)
	MODIFY_REG(RCC->CFGR, RCC_CFGR_PPRE1, RCC_HCLK_DIV1);

#ifdef STM32L0
	// STM32L0's have a second PCLK. The shift by 3 is defined like this in the HAL.
	MODIFY_REG(RCC->CFGR, RCC_CFGR_PPRE2, RCC_HCLK_DIV1 << 3);
#endif

	/*
	 * DISABLE OSCILLATORS
	 * Unused oscillators should be turned off.
	 * Note they are disabled AFTER sysclk source is redirected
	 */

#ifndef CLK_USE_HSI
	__HAL_RCC_HSI_CONFIG(RCC_HSI_OFF);
#endif
#ifndef CLK_USE_MSI
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
	// ADC CLK is driven off the HSI.
#ifndef CLK_USE_HSI
	__HAL_RCC_HSI_CONFIG(RCC_HSI_ON);
	while(__HAL_RCC_GET_FLAG(RCC_FLAG_HSIRDY) == 0U);
#endif
}

void CLK_DisableADCCLK(void)
{
#ifndef CLK_USE_HSI
	__HAL_RCC_HSI_CONFIG(RCC_HSI_OFF);
#endif
}

/*
 * PRIVATE FUNCTIONS
 */

#ifdef CLK_USE_LSE
static void CLK_ResetBackupDomain(void)
{
	uint32_t csr = (RCC->CSR & ~(RCC_CSR_RTCSEL));
	// RTC Clock selection can be changed only if the Backup Domain is reset
	__HAL_RCC_BACKUPRESET_FORCE();
	__HAL_RCC_BACKUPRESET_RELEASE();
	RCC->CSR = csr;
}
#endif

static void CLK_AccessBackupDomain(void)
{
	if (HAL_IS_BIT_CLR(PWR->CR, PWR_CR_DBP))
	{
		// Get access to backup domain
		SET_BIT(PWR->CR, PWR_CR_DBP);
		while(HAL_IS_BIT_CLR(PWR->CR, PWR_CR_DBP));
	}
}

/*
 * INTERRUPT ROUTINES
 */

