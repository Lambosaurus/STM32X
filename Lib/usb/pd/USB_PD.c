
#include "USB_PD.h"
#ifdef USB_PD

#include "CLK.h"

/*
 * PRIVATE DEFINITIONS
 */

#if defined(USB_PD_SOURCE)
#error "This has not yet been developed"
#elif defined(USB_PD_SINK)
#else
#error "Please define role as source or sink"
#endif

#if USB_PD == 1
#define UCPD		UCPD1
#elif USB_PD == 2
#define UCPD		UCPD2
#endif

#define _USB_PD_ENABLE(ucpd)	(ucpd->CFG1 |= UCPD_CFG1_UCPDEN)
#define _USB_PD_DISABLE(ucpd)	(ucpd->CFG1 &= ~UCPD_CFG1_UCPDEN)

#define UCPD_STROBE()			(SYSCFG->CFGR1 |= SYSCFG_CFGR1_UCPD1_STROBE | SYSCFG_CFGR1_UCPD2_STROBE)
#define UCPD_KCLK_FREQ()		(16000000) // Fed from the HSI16

/*
 * PRIVATE TYPES
 */

/*
 * PRIVATE PROTOTYPES
 */

/*
 * PRIVATE VARIABLES
 */

static void USB_PDx_Init(void);
static void USB_PDx_Deinit(void);

/*
 * PUBLIC FUNCTIONS
 */

void USB_PD_Init(void)
{
	USB_PDx_Init();

	uint32_t src_freq = UCPD_KCLK_FREQ();
	uint32_t ucpd_freq = 8000000; // Target is 6-12 MHz
	uint32_t hbit_freq = 600000; // Target is ~600KHz
	uint32_t ucpd_prescalar = CLK_SelectPrescalar(src_freq, 1, 16, &ucpd_freq);
	uint32_t hbit_prescalar = CLK_SelectPrescalar(ucpd_freq, 1, 64, &hbit_freq);

	uint32_t cfg1 =   (UCPD_CFG1_PSC_UCPDCLK_0 * ucpd_prescalar)
					| (UCPD_CFG1_TRANSWIN_0 * 9)
					| (UCPD_CFG1_IFRGAP_0 * 15)
					| (UCPD_CFG1_HBITCLKDIV_0 * hbit_prescalar);
	UCPD->CFG1 |= cfg1;

	uint32_t cfg2 = UCPD_CFG2_FORCECLK;
	UCPD->CFG2 = cfg2;

	_USB_PD_ENABLE(UCPD);

	uint32_t cr = UCPD_CR_CCENABLE_0 | UCPD_CR_CCENABLE_1;
#ifdef USB_PD_SINK
	cr |= UCPD_CR_ANAMODE;
#endif
	UCPD->CR = cr;

	UCPD_STROBE();
}

void USB_PD_Deinit(void)
{
	_USB_PD_DISABLE(UCPD);
	USB_PDx_Deinit();
	UCPD_STROBE();
}

USB_PD_Flag_t USB_PD_Read(void)
{
	uint32_t sr = UCPD->SR;
	if (sr & UCPD_SR_TYPEC_VSTATE_CC1)
	{
		USB_PD_Flag_t current = (sr & UCPD_SR_TYPEC_VSTATE_CC1) >> UCPD_SR_TYPEC_VSTATE_CC1_Pos;
		return USB_PD_Flag_CC1 | (current * USB_PD_Flag_500mA);
	}
	if (sr & UCPD_SR_TYPEC_VSTATE_CC2)
	{
		USB_PD_Flag_t current = (sr & UCPD_SR_TYPEC_VSTATE_CC2) >> UCPD_SR_TYPEC_VSTATE_CC2_Pos;
		return USB_PD_Flag_CC2 | (current * USB_PD_Flag_500mA);
	}
	return 0;
}

/*
 * PRIVATE FUNCTIONS
 */

static void USB_PDx_Init(void)
{
#if USB_PD == 1
	__HAL_RCC_UCPD1_CLK_ENABLE();
#elif USB_PD == 2
	__HAL_RCC_UCPD2_CLK_ENABLE();
#endif
}

static void USB_PDx_Deinit(void)
{
#if USB_PD == 1
	__HAL_RCC_UCPD1_CLK_DISABLE();
#elif USB_PD == 2
	__HAL_RCC_UCPD2_CLK_DISABLE();
#endif
}

/*
 * INTERRUPT ROUTINES
 */

#endif //USB_PD
