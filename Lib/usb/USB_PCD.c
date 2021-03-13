
#include "USB_PCD.h"

/*
 * PRIVATE DEFINITIONS
 */

#if defined(STM32L0)

#elif defined(STM32F0)

#endif

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

/*
 * PUBLIC FUNCTIONS
 */

void USB_PCD_Start(void)
{
	// Enable interrupt sources
	USB->CNTR = (uint16_t)(
			USB_CNTR_CTRM  | USB_CNTR_WKUPM |
            USB_CNTR_SUSPM | USB_CNTR_ERRM |
            USB_CNTR_SOFM | USB_CNTR_ESOFM |
            USB_CNTR_RESETM | USB_CNTR_L1REQM
			);
	// Enable DP/DM pullups
	USB->BCDR |= (uint16_t)USB_BCDR_DPPU;
}

void USB_PCD_Stop(void)
{
	USB->CNTR = 0;

	// Disable DP/DM pullups
	USB->BCDR &= (uint16_t)(~(USB_BCDR_DPPU));
}

void USB_PCD_EP_Open(uint8_t endpoint, uint8_t type, uint16_t maxpacket)
{
	PCD_HandleTypeDef * hpcd = &hpcd_USB_FS;
	PCD_EPTypeDef *ep;
	if (endpoint & 0x80U)
	{
		ep = &hpcd->IN_ep[endpoint & EP_ADDR_MSK];
		ep->is_in = 1U;
		hUsbDeviceFS.ep_in[endpoint & EP_ADDR_MSK].is_used = 1;
	}
	else
	{
		ep = &hpcd->OUT_ep[endpoint & EP_ADDR_MSK];
		ep->is_in = 0U;
		hUsbDeviceFS.ep_out[endpoint & EP_ADDR_MSK].is_used = 1;
	}
	ep->num = endpoint & EP_ADDR_MSK;
	ep->maxpacket = maxpacket;
	ep->type = type;
	USB_ActivateEndpoint(USB, ep);
}

void USB_PCD_EP_Close(uint8_t endpoint)
{
	PCD_HandleTypeDef * hpcd = &hpcd_USB_FS;
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
