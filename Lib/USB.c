
#include "USB.h"
#include "Core.h"

#include "usb/USB_PCD.h"
#include "usb/USB_CDC.h"
#include "usb/USB_EP.h"
#include "usb/USB_CTL.h"

#include "usbd_def.h"

/*
 * PRIVATE DEFINITIONS
 */

#if defined(STM32L0)

#elif defined(STM32F0)

#endif

#define USB_GET_IRQ() 		(USB->ISTR)
#define USB_CLR_IRQ(flag)	(USB->ISTR &= ~flag)

/*
 * PRIVATE TYPES
 */

/*
 * PRIVATE PROTOTYPES
 */

static void USBx_Init(void);
static void USBx_Deinit(void);

/*
 * PRIVATE VARIABLES
 */

/*
 * PUBLIC FUNCTIONS
 */

void USB_Init(void)
{
	CORE_EnableUSBClock();
	USBx_Init();

	hUsbDeviceFS.pClass = &USBD_CDC;

	USB_PCD_Init();
	USB_CTL_Init();
	USB_PCD_Start();
}

void USB_Deinit(void)
{
	USB_PCD_Stop();
	USB_CTL_Deinit();
	USB_PCD_Deinit();
	USBx_Deinit();
}

void USB_Write(const uint8_t * data, uint32_t count)
{
	USB_CDC_Write(data, count);
}

uint32_t USB_Read(uint8_t * data, uint32_t size)
{
	return USB_CDC_Read(data, size);
}

/*
 * PRIVATE FUNCTIONS
 */

static void USBx_Init(void)
{
	__HAL_RCC_USB_CLK_ENABLE();
	HAL_NVIC_SetPriority(USB_IRQn, 1, 1);
	HAL_NVIC_EnableIRQ(USB_IRQn);
}

static void USBx_Deinit(void)
{
	__HAL_RCC_USB_CLK_DISABLE();
	HAL_NVIC_DisableIRQ(USB_IRQn);
}

/*
 * INTERRUPT ROUTINES
 */

void USB_IRQHandler(void)
{
	PCD_HandleTypeDef * hpcd = &hpcd_USB_FS;

	uint32_t istr = USB_GET_IRQ();

	if (istr & USB_ISTR_CTR)
	{
		USB_EP_IRQHandler();
	}
	else if (istr & USB_ISTR_RESET)
	{
		USB_CLR_IRQ(USB_ISTR_RESET);
		USB_CTL_Reset();
	}
	else if (istr & USB_ISTR_PMAOVR)
	{
		USB_CLR_IRQ(USB_ISTR_PMAOVR);
	}
	else if (istr & USB_ISTR_WKUP)
	{
		// Clear LP & suspend modes.
		USB->CNTR &= ~(USB_CNTR_LPMODE | USB_CNTR_FSUSP);
		hpcd->LPM_State = LPM_L0;

		USB_CLR_IRQ(USB_ISTR_WKUP);
	}
	else if (istr & USB_ISTR_SUSP)
	{
		// Force low-power mode in the peripheral
		USB->CNTR |= USB_CNTR_FSUSP;
		// clear of the ISTR bit must be done after setting of CNTR_FSUSP
		USB_CLR_IRQ(USB_ISTR_SUSP);
		USB->CNTR |= USB_CNTR_LPMODE;
	}
	else if (istr & USB_ISTR_L1REQ)
	{
		USB_CLR_IRQ(USB_ISTR_L1REQ);

		if (hpcd->LPM_State == LPM_L0)
		{
			// Force suspend and low-power mode before going to L1 state
			USB->CNTR |= USB_CNTR_LPMODE | USB_CNTR_FSUSP;

			hpcd->LPM_State = LPM_L1;
			hpcd->BESL = ((uint32_t)hpcd->Instance->LPMCSR & USB_LPMCSR_BESL) >> 2;
		}
	}
}
