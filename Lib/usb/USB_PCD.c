
#include "USB_PCD.h"

/*
 * PRIVATE DEFINITIONS
 */

#if defined(STM32L0)

#elif defined(STM32F0)

#endif

#define PCD_ENDPOINTS		8

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
PCD_HandleTypeDef * hpcd = &hpcd_USB_FS;

/*
 * PUBLIC FUNCTIONS
 */

void USB_PCD_Init(void)
{
	hpcd_USB_FS.pData = &hUsbDeviceFS;
	hpcd_USB_FS.Instance = USB;

	for (uint32_t i = 0U; i < PCD_ENDPOINTS; i++)
	{
		hpcd->IN_ep[i].is_in = 1U;
		hpcd->IN_ep[i].num = i;
		hpcd->IN_ep[i].type = EP_TYPE_CTRL;
		hpcd->IN_ep[i].maxpacket = 0U;
		hpcd->IN_ep[i].xfer_buff = 0U;
		hpcd->IN_ep[i].xfer_len = 0U;
	}
	for (uint32_t i = 0U; i < PCD_ENDPOINTS; i++)
	{
		hpcd->OUT_ep[i].is_in = 0U;
		hpcd->OUT_ep[i].num = i;
		hpcd->OUT_ep[i].type = EP_TYPE_CTRL;
		hpcd->OUT_ep[i].maxpacket = 0U;
		hpcd->OUT_ep[i].xfer_buff = 0U;
		hpcd->OUT_ep[i].xfer_len = 0U;
	}

	USB->CNTR = USB_CNTR_FRES; // Issue reset
	USB->CNTR = 0U;
	USB->ISTR = 0U;
	USB->BTABLE = BTABLE_ADDRESS;

	hpcd->USB_Address = 0U;
	hpcd->State = HAL_PCD_STATE_READY;

#ifdef USB_USE_LPM
	hpcd->LPM_State = LPM_L0;
	USB->LPMCSR |= USB_LPMCSR_LMPEN;
	USB->LPMCSR |= USB_LPMCSR_LPMACK;
#endif
}

void USB_PCD_Start(void)
{
	// Enable interrupt sources
	USB->CNTR = USB_CNTR_CTRM  | USB_CNTR_WKUPM
              | USB_CNTR_SUSPM | USB_CNTR_ERRM
              | USB_CNTR_SOFM | USB_CNTR_ESOFM
              | USB_CNTR_RESETM | USB_CNTR_L1REQM;
	// Enable DP pullups
	USB->BCDR |= USB_BCDR_DPPU;
}

void USB_PCD_Stop(void)
{
	// disable all interrupts and force USB reset
	USB->CNTR = USB_CNTR_FRES;
	// clear interrupt status register
	USB->ISTR = 0U;
	// switch-off device
	USB->CNTR = USB_CNTR_FRES | USB_CNTR_PDWN;
	// Disable DP pullups
	USB->BCDR &= ~USB_BCDR_DPPU;

	// DELETE THIS.
	hpcd_USB_FS.State = HAL_PCD_STATE_RESET;
}

void USB_PCD_EP_Open(uint8_t endpoint, uint8_t type, uint16_t maxpacket)
{
	PCD_EPTypeDef *ep;
	if (endpoint & 0x80U)
	{
		ep = &hpcd->IN_ep[endpoint & EP_ADDR_MSK];
		hUsbDeviceFS.ep_in[endpoint & EP_ADDR_MSK].is_used = 1;
	}
	else
	{
		ep = &hpcd->OUT_ep[endpoint & EP_ADDR_MSK];
		hUsbDeviceFS.ep_out[endpoint & EP_ADDR_MSK].is_used = 1;
	}
	ep->maxpacket = maxpacket;
	ep->type = type;
	USB_ActivateEndpoint(USB, ep);
}

void USB_PCD_EP_Close(uint8_t endpoint)
{
	PCD_EPTypeDef *ep;
	if (endpoint & 0x80U)
	{
		ep = &hpcd->IN_ep[endpoint & EP_ADDR_MSK];
		hUsbDeviceFS.ep_in[endpoint & EP_ADDR_MSK].is_used = 0;
	}
	else
	{
		ep = &hpcd->OUT_ep[endpoint & EP_ADDR_MSK];
		hUsbDeviceFS.ep_out[endpoint & EP_ADDR_MSK].is_used = 0;
	}
	USB_DeactivateEndpoint(USB, ep);
}

void USB_PCD_EP_StartRx(uint8_t endpoint, uint8_t * data, uint32_t count)
{
	PCD_EPTypeDef * ep = &(hpcd_USB_FS.OUT_ep[endpoint & EP_ADDR_MSK]);
	ep->xfer_buff = data;
	ep->xfer_len = count;
	ep->xfer_count = 0;
	USB_EPStartXfer(USB, ep);
}

void USB_PCD_EP_StartTx(uint8_t endpoint, uint8_t * data, uint32_t count)
{
	PCD_EPTypeDef * ep = &(hpcd_USB_FS.IN_ep[endpoint & EP_ADDR_MSK]);
	ep->xfer_buff = data;
	ep->xfer_len = count;
	ep->xfer_fill_db = 1;
	ep->xfer_len_db = count;
	ep->xfer_count = 0;
	hUsbDeviceFS.ep_in[endpoint & EP_ADDR_MSK].total_length = count;
	USB_EPStartXfer(USB, ep);
}

/*
 * PRIVATE FUNCTIONS
 */

/*
 * INTERRUPT ROUTINES
 */
