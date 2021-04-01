
#include "USB_PCD.h"
#include "USB_EP.h"
#include "USB_CLASS.h"

/*
 * PRIVATE DEFINITIONS
 */

/*
 * PRIVATE TYPES
 */

/*
 * PRIVATE PROTOTYPES
 */

/*
 * PRIVATE VARIABLES
 */

// DELETE THIS
USBD_HandleTypeDef hUsbDeviceFS;
PCD_HandleTypeDef hpcd_USB_FS;

/*
 * PUBLIC FUNCTIONS
 */

void USB_PCD_Init(void)
{
	hpcd_USB_FS.pData = &hUsbDeviceFS;
	hpcd_USB_FS.Instance = USB;

	USB->CNTR = USB_CNTR_FRES; // Issue reset
	USB->CNTR = 0U;
	USB->ISTR = 0U;
	USB->BTABLE = BTABLE_ADDRESS;

	USB_EP_Init();

	hpcd_USB_FS.State = HAL_PCD_STATE_READY;

#ifdef USB_USE_LPM
	hpcd->LPM_State = LPM_L0;
	USB->LPMCSR |= USB_LPMCSR_LMPEN;
	USB->LPMCSR |= USB_LPMCSR_LPMACK;
#endif
}

void USB_PCD_Start(void)
{
	// Enable interrupt sources
	USB->CNTR = USB_CNTR_CTRM | USB_CNTR_RESETM
			  | USB_CNTR_WKUPM | USB_CNTR_SUSPM
            // | USB_CNTR_SOFM | USB_CNTR_ESOFM | USB_CNTR_ERRM
              | USB_CNTR_L1REQM;
			// USB_CNTR_RESUME remote wakeup mode?
	// Enable DP pullups
	USB->BCDR |= USB_BCDR_DPPU;
}

void USB_PCD_Stop(void)
{
	// disable all interrupts and force USB reset
	USB->CNTR = USB_CNTR_FRES;
	USB->ISTR = 0U;
	// switch-off device
	USB->CNTR = USB_CNTR_FRES | USB_CNTR_PDWN;
	// Disable DP pullups
	USB->BCDR &= ~USB_BCDR_DPPU;

	// DELETE THIS.
	hpcd_USB_FS.State = HAL_PCD_STATE_RESET;
}

void USB_PCD_SetAddress(uint8_t address)
{
	USB->DADDR = address | USB_DADDR_EF;
}

/*
 * PRIVATE FUNCTIONS
 */

/*
 * INTERRUPT ROUTINES
 */

