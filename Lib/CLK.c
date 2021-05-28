
#include "CLK.h"

/*
 * PRIVATE DEFINITIONS
 */

/*
 * PRIVATE TYPES
 */

/*
 * PRIVATE PROTOTYPES
 */

#ifdef CLK_USE_LSE
static void CLK_ResetBackupDomain(void)
#endif
static void CLK_AccessBackupDomain(void);

/*
 * PRIVATE VARIABLES
 */

/*
 * PUBLIC FUNCTIONS
 */

void CLK_Init(void)
{
	RCC_OscInitTypeDef osc = {0};
#ifdef CLK_USE_HSE
	osc.OscillatorType 		= RCC_OSCILLATORTYPE_HSE;
	osc.HSEState 			= RCC_HSE_ON;
	osc.PLL.PLLState 		= RCC_PLL_ON;
	osc.PLL.PLLSource 		= RCC_PLLSOURCE_HSE;
	osc.PLL.PLLMUL 			= RCC_PLL_MUL2;
#ifdef STM32F0
	osc.PLL.PREDIV			= RCC_PREDIV_DIV1;
#else
	osc.PLL.PLLDIV 			= RCC_PLL_DIV1;
#endif
#else
	osc.OscillatorType 		= RCC_OSCILLATORTYPE_HSI;
	osc.HSIState 			= RCC_HSI_ON;
	osc.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
	osc.PLL.PLLState 		= RCC_PLL_ON;
	osc.PLL.PLLSource 		= RCC_PLLSOURCE_HSI;
	osc.PLL.PLLMUL 			= RCC_PLL_MUL4;
#ifdef STM32F0
	osc.PLL.PREDIV			= RCC_PREDIV_DIV2;
#else
	osc.PLL.PLLDIV 			= RCC_PLL_DIV2;
#endif
#endif //CLK_USE_HSE
	HAL_RCC_OscConfig(&osc);

	RCC_ClkInitTypeDef clk = {0};
	clk.ClockType 		= RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1;
	clk.SYSCLKSource 	= RCC_SYSCLKSOURCE_PLLCLK;
	clk.AHBCLKDivider 	= RCC_SYSCLK_DIV1;
	clk.APB1CLKDivider 	= RCC_HCLK_DIV1;
#ifdef STM32L0
	clk.ClockType 		|= RCC_CLOCKTYPE_PCLK2;
	clk.APB2CLKDivider  = RCC_HCLK_DIV1;
#endif
	HAL_RCC_ClockConfig(&clk, FLASH_LATENCY_1);
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
#ifdef CLK_LSE_BYPASS
	__HAL_RCC_LSE_CONFIG(RCC_LSE_BYPASS);
#else
	__HAL_RCC_LSE_CONFIG(RCC_LSE_ON);
#endif
	while(!__HAL_RCC_GET_FLAG(RCC_FLAG_LSERDY));

	CLK_AccessBackupDomain();
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

