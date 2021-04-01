
#include "USB.h"
#include "Core.h"

#include "usb/USB_PCD.h"

#include "usbd_core.h"
#include "usbd_desc.h"
#include "usb/USB_CDC.h"
#include "usb/USB_EP.h"


/*
 * PRIVATE DEFINITIONS
 */

#if defined(STM32L0)

#elif defined(STM32F0)

#endif

#define USB_GET_IRQ() 		(USB->ISTR)
#define USB_CLR_IRQ(flag)	(USB->ISTR &= ~flag)

#define CTL_IN_EP		0x80
#define CTL_OUT_EP		0x00

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

	hUsbDeviceFS.pDesc = &FS_Desc;
	hUsbDeviceFS.dev_state = USBD_STATE_DEFAULT;
	hUsbDeviceFS.id = DEVICE_FS;
	hUsbDeviceFS.pClass = &USBD_CDC;
	hUsbDeviceFS.pData = &hpcd_USB_FS;

	USB_PCD_Init();

	// Initialise the ctrl endpoints
	USB_EP_Open(CTL_IN_EP, USBD_EP_TYPE_CTRL, USB_PACKET_SIZE);
	USB_EP_Open(CTL_OUT_EP, USBD_EP_TYPE_CTRL, USB_PACKET_SIZE);

	USB_PCD_Start();

}

void USB_Deinit(void)
{
	USB_PCD_Stop();
	USBx_Deinit();
}

void USB_Tx(const uint8_t * data, uint32_t count)
{
	USB_CDC_Tx(data, count);
}

uint32_t USB_Rx(uint8_t * data, uint32_t size)
{
	return USB_CDC_Rx(data, size);
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
		HAL_PCD_ResetCallback(hpcd);
		USB_PCD_SetAddress(0U);
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
		// Force low-power mode in the macrocell
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
