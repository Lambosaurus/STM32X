
#include "USB_Defs.h"
#include "USB_Class.h"

#include "USB_CTL.h"
#include "USB_EP.h"
#include "USB_PCD.h"


/*
 * PRIVATE DEFINITIONS
 */

#define CTL_IN_EP		0x80
#define CTL_OUT_EP		0x00

#define CTL_EP_SIZE		USB_MAX_EP0_SIZE

typedef enum {
	USB_STATE_DEFAULT,
	USB_STATE_ADDRESSED,
	USB_STATE_CONFIGURED,
	USB_STATE_SUSPENDED
} USB_CTL_State_t;

/*
 * PRIVATE TYPES
 */

/*
 * PUBLIC TYPES
 */

/*
 * PRIVATE PROTOTYPES
 */

static void USB_CTL_EndpointRequest(USB_SetupRequest_t  *req);
static void USB_CTL_DeviceRequest(USB_SetupRequest_t *req);
static void USB_CTL_InterfaceRequest(USB_SetupRequest_t  *req);
static void USB_CTL_StandardClassRequest(USB_SetupRequest_t  *req);

static void USB_CTL_SetAddress(USB_SetupRequest_t * req);
static void USB_CTL_SetFeature(USB_SetupRequest_t *req);
static void USB_CTL_ClearFeature(USB_SetupRequest_t *req);
static void USB_CTL_GetConfig(USB_SetupRequest_t *req);
static void USB_CTL_SetConfig(USB_SetupRequest_t *req);
static void USB_CTL_GetStatus(USB_SetupRequest_t *req);
static void USB_CTL_GetDescriptor(USB_SetupRequest_t *req);

static uint16_t USB_CTL_GetLangIdDescriptor(uint8_t * data);
static uint16_t USB_CTL_GetStrDescriptor(uint8_t * data, const char * str);
static uint16_t USB_CTL_GetSerialDescriptor(uint8_t * data);

static void USB_CTL_SendStatus(void);
//static void USB_CTL_Send(uint8_t * data, uint16_t size);
static void USB_CTL_ReceiveStatus(void);
//static void USB_CTL_Receive(uint8_t * data, uint16_t size);
//static void USB_CTL_Error(void);


/*
 * PRIVATE VARIABLES
 */

#define CTL_BUFFER_SIZE		(MAX((USB_MAX_STRING_SIZE+1)*2, CTL_EP_SIZE))

static struct {
	uint8_t address;
	uint8_t class_config;
	uint8_t buffer[CTL_BUFFER_SIZE];
	uint8_t state;
} gCTL;

__ALIGNED(4) const uint8_t cUsbDeviceDescriptor[USB_LEN_DEV_DESC] =
{
	USB_LEN_DEV_DESC,           // bLength
	USB_DESC_TYPE_DEVICE,       // bDescriptorType
	0x00,                       // bcdUSB
	0x02,
	USB_CLASS_CLASSID,          // bDeviceClass
	USB_CLASS_SUBCLASSID,       // bDeviceSubClass
	USB_CLASS_PROTOCOLID,       // bDeviceProtocol
	CTL_EP_SIZE,                // bMaxPacketSize
	LOBYTE(USB_VID),            // idVendor
	HIBYTE(USB_VID),            // idVendor
	LOBYTE(USB_PID),            // idProduct
	HIBYTE(USB_PID),        	// idProduct
	0x00,                       // bcdDevice rel. 2.00
	0x02,
	USBD_IDX_MFC_STR,           // Index of manufacturer  string
	USBD_IDX_PRODUCT_STR,       // Index of product string
	USBD_IDX_SERIAL_STR,        // Index of serial number string
	USB_MAX_NUM_CONFIGURATION  // bNumConfigurations
};

/*
 * PUBLIC FUNCTIONS
 */


void USB_CTL_Init(void)
{
	USB_EP_Open(CTL_IN_EP, USB_EP_TYPE_CTRL, CTL_EP_SIZE);
	USB_EP_Open(CTL_OUT_EP, USB_EP_TYPE_CTRL, CTL_EP_SIZE);
	gCTL.address = 0;
	gCTL.class_config = 0;
	gCTL.state = USB_STATE_DEFAULT;
}

void USB_CTL_Deinit(void)
{
	// Dont bother closing the CTL EP's
	if (gCTL.class_config != 0)
	{
		gCTL.class_config = 0;
		USB_CLASS_DEINIT();
	}
}

void USB_CTL_Reset(void)
{
	// Clear existing endpoint layouts.
	USB_CTL_Deinit();
	USB_EP_Reset();
	USB_PCD_SetAddress(0);

	// Reinit the CTRL EP's
	USB_CTL_Init();

	hUsbDeviceFS.ep0_state = USBD_EP0_IDLE;
	hUsbDeviceFS.dev_remote_wakeup = 0U;
}

void USB_CTL_HandleSetup(uint8_t * data)
{
	USB_SetupRequest_t req;
	req.bmRequest = *(data);
	req.bRequest = *(data + 1U);
	req.wValue = SWAPBYTE(data + 2U);
	req.wIndex = SWAPBYTE(data + 4U);
	req.wLength = SWAPBYTE(data + 6U);

	hUsbDeviceFS.ep0_state = USBD_EP0_SETUP;
	hUsbDeviceFS.ep0_data_len = req.wLength;

	switch (req.bmRequest & 0x1F)
	{
	case USB_REQ_RECIPIENT_DEVICE:
		USB_CTL_DeviceRequest(&req);
		break;
	case USB_REQ_RECIPIENT_INTERFACE:
		USB_CTL_InterfaceRequest(&req);
		break;
	case USB_REQ_RECIPIENT_ENDPOINT:
		USB_CTL_EndpointRequest(&req);
		break;
	default:
		USB_EP_Stall(req.bmRequest & 0x80);
		break;
	}
}

/*
 * PRIVATE FUNCTIONS
 */

void USB_CTL_Send(uint8_t * data, uint16_t size)
{
	USBD_HandleTypeDef * pdev = &hUsbDeviceFS;
	pdev->ep0_state = USBD_EP0_DATA_IN;
	pdev->ep_in[0].total_length = size;
	pdev->ep_in[0].rem_length   = size;
	USB_EP_Write(CTL_IN_EP, data, size);
}

void USB_CTL_Receive(uint8_t * data, uint16_t size)
{
	USBD_HandleTypeDef * pdev = &hUsbDeviceFS;
	pdev->ep0_state = USBD_EP0_DATA_OUT;
	pdev->ep_out[0].total_length = size;
	pdev->ep_out[0].rem_length   = size;
	USB_EP_Read(CTL_OUT_EP, data, size);
}

void USB_CTL_ReceiveStatus(void)
{
	USBD_HandleTypeDef * pdev = &hUsbDeviceFS;
	pdev->ep0_state = USBD_EP0_STATUS_OUT;
	USB_EP_Read(CTL_OUT_EP, NULL, 0);
}

static void USB_CTL_SendStatus(void)
{
	hUsbDeviceFS.ep0_state = USBD_EP0_STATUS_IN;
	USB_EP_Write(0x00U, NULL, 0U);
}

static void USB_CTL_EndpointRequest(USB_SetupRequest_t  *req)
{
	uint8_t endpoint = LOBYTE(req->wIndex);

	switch (req->bmRequest & USB_REQ_TYPE_MASK)
	{
	case USB_REQ_TYPE_CLASS:
	case USB_REQ_TYPE_VENDOR:
		USB_CLASS_SETUP(req);
		return;
	case USB_REQ_TYPE_STANDARD:
		if ((req->bmRequest & 0x60U) == 0x20U)
		{
			// Indicates this standard request is directed at the class
			USB_CTL_StandardClassRequest(req);
			return;
		}

		switch (req->bRequest)
		{
		case USB_REQ_SET_FEATURE:
			switch (gCTL.state)
			{
			case USB_STATE_ADDRESSED:
				if ((endpoint != CTL_OUT_EP) && (endpoint != CTL_IN_EP))
				{
					USB_EP_Stall(endpoint);
					USB_EP_Stall(CTL_IN_EP);
					return;
				}
				break;
			case USB_STATE_CONFIGURED:
				if (req->wValue == USB_FEATURE_EP_HALT)
				{
					if ((endpoint != CTL_OUT_EP) && (endpoint != CTL_IN_EP) && (req->wLength == 0x00U))
					{
						USB_EP_Stall(endpoint);
					}
				}
				USB_CTL_SendStatus();
				return;
			}
			break;
		case USB_REQ_CLEAR_FEATURE:
			switch (gCTL.state)
			{
			case USB_STATE_ADDRESSED:
				if ((endpoint & 0x7FU) != 0x00U)
				{
					USB_EP_Stall(endpoint);
					USB_EP_Stall(CTL_IN_EP);
				}
				break;
			case USB_STATE_CONFIGURED:
				if (req->wValue == USB_FEATURE_EP_HALT)
				{
					if ((endpoint & 0x7FU) != 0x00U)
					{
						USB_EP_Destall(endpoint);
					}
					USB_CTL_SendStatus();
					return;
				}
				break;
			}
			break;
		case USB_REQ_GET_STATUS:
			switch (gCTL.state)
			{
			case USBD_STATE_ADDRESSED:
			case USBD_STATE_CONFIGURED:
				if (USB_EP_IsOpen(endpoint))
				{
					uint16_t status = USB_EP_IsStalled(endpoint) ? 0x0001 : 0x0000;
					USB_CTL_Send((uint8_t *)&status, 2);
					return;
				}
				break;
			}
			break;
		}
		break;
	}
	USB_CTL_Error();
}

static void USB_CTL_StandardClassRequest(USB_SetupRequest_t  *req)
{
	USBD_HandleTypeDef * pdev = &hUsbDeviceFS;
	if (gCTL.state == USB_STATE_CONFIGURED)
	{
		switch (req->bRequest)
		{
			case USB_REQ_GET_STATUS:
			{
				// status is always 0 for classes
				uint16_t status_info = 0;
				USB_CTL_Send((uint8_t *)&status_info, 2U);
				return;
			}
			case USB_REQ_GET_INTERFACE:
			{
				// Alternate interfaces not supported.
				uint8_t ifalt = 0;
				USB_CTL_Send(&ifalt, 1);
				return;
			}
			case USB_REQ_SET_INTERFACE:
				return;
		}
	}
	USB_CTL_Error();
}

static void USB_CTL_DeviceRequest(USB_SetupRequest_t *req)
{
	switch (req->bmRequest & USB_REQ_TYPE_MASK)
	{
	case USB_REQ_TYPE_CLASS:
	case USB_REQ_TYPE_VENDOR:
		USB_CLASS_SETUP(req);
		return;
	case USB_REQ_TYPE_STANDARD:
		switch (req->bRequest)
		{
		case USB_REQ_GET_DESCRIPTOR:
			USB_CTL_GetDescriptor(req);
			return;
		case USB_REQ_SET_ADDRESS:
			USB_CTL_SetAddress(req);
			return;
		case USB_REQ_SET_CONFIGURATION:
			USB_CTL_SetConfig(req);
			return;
		case USB_REQ_GET_CONFIGURATION:
			USB_CTL_GetConfig(req);
			return;
		case USB_REQ_GET_STATUS:
			USB_CTL_GetStatus(req);
			return;
		case USB_REQ_SET_FEATURE:
			USB_CTL_SetFeature(req);
			return;
		case USB_REQ_CLEAR_FEATURE:
			USB_CTL_ClearFeature(req);
			return;
		}
		break;
	}
	USB_CTL_Error();
}


static void USB_CTL_SetFeature(USB_SetupRequest_t * req)
{
	if (req->wValue == USB_FEATURE_REMOTE_WAKEUP)
	{
		hUsbDeviceFS.dev_remote_wakeup = 1U;
		USB_CTL_SendStatus();
	}
}

static void USB_CTL_ClearFeature(USB_SetupRequest_t * req)
{
	switch (gCTL.state)
	{
	case USB_STATE_DEFAULT:
	case USB_STATE_ADDRESSED:
	case USB_STATE_CONFIGURED:
		if (req->wValue == USB_FEATURE_REMOTE_WAKEUP)
		{
			hUsbDeviceFS.dev_remote_wakeup = 0U;
			USB_CTL_SendStatus();
		}
		return;
	}
	USB_CTL_Error();
}

static void USB_CTL_SetAddress(USB_SetupRequest_t * req)
{
	if ((req->wIndex == 0) && (req->wLength == 0) && (req->wValue < 128))
	{
		if (gCTL.state != USB_STATE_CONFIGURED)
		{
			uint8_t address = (uint8_t)(req->wValue) & 0x7FU;
			gCTL.state = (address != 0) ? USB_STATE_ADDRESSED : USB_STATE_DEFAULT;
			gCTL.address = address;
			USB_CTL_SendStatus();
			return;
		}
	}
	USB_CTL_Error();
}

static void USB_CTL_SetConfig(USB_SetupRequest_t * req)
{
	uint8_t config = (uint8_t)(req->wValue);
	if (config <= USB_MAX_NUM_CONFIGURATION)
	{
		if (gCTL.class_config != 0)
		{
			gCTL.class_config = 0;
			USB_CLASS_DEINIT();
		}

		switch (gCTL.state)
		{
		case USB_STATE_ADDRESSED:
		case USB_STATE_CONFIGURED:
			if (config == 0)
			{
				gCTL.state = USB_STATE_ADDRESSED;
			}
			else
			{
				gCTL.state = USB_STATE_CONFIGURED;
				gCTL.class_config = config;
				USB_CLASS_INIT(config);
			}
			USB_CTL_SendStatus();
			return;
		}
	}
	USB_CTL_Error();
}

static void USB_CTL_GetConfig(USB_SetupRequest_t * req)
{
	if (req->wLength == 1)
	{
		switch (gCTL.state)
		{
		case USB_STATE_DEFAULT:
		case USB_STATE_ADDRESSED:
		case USB_STATE_CONFIGURED:
			USB_CTL_Send(&gCTL.class_config, 1);
			return;
		}
	}
	USB_CTL_Error();
}

static void USB_CTL_GetStatus(USB_SetupRequest_t * req)
{
	switch (gCTL.state)
	{
	case USBD_STATE_DEFAULT:
	case USBD_STATE_ADDRESSED:
	case USBD_STATE_CONFIGURED:
		if (req->wLength == 0x2U)
		{
#ifdef USB_SELF_POWERED
			uint16_t status = USB_CONFIG_SELF_POWERED;
#else
			uint16_t status = 0;
#endif
			if (hUsbDeviceFS.dev_remote_wakeup)
			{
				status |= USB_CONFIG_REMOTE_WAKEUP;
			}
			USB_CTL_Send((uint8_t *)&status, 2);
			return;
		}
	}
	USB_CTL_Error();
}

static void USB_CTL_InterfaceRequest(USB_SetupRequest_t * req)
{
	switch (req->bmRequest & USB_REQ_TYPE_MASK)
	{
	case USB_REQ_TYPE_CLASS:
	case USB_REQ_TYPE_VENDOR:
	case USB_REQ_TYPE_STANDARD:
		switch (gCTL.state)
		{
		case USB_STATE_DEFAULT:
		case USB_STATE_ADDRESSED:
		case USB_STATE_CONFIGURED:
			if (LOBYTE(req->wIndex) <= USB_MAX_NUM_INTERFACES)
			{
				USB_CLASS_SETUP(req);
				if ((req->wLength == 0U))
				{
					USB_CTL_SendStatus();
				}
				return;
			}
			break;
		}
		break;
	}
	USB_CTL_Error();
}


static void USB_CTL_GetDescriptor(USB_SetupRequest_t * req)
{
	uint16_t len = 0;
	uint8_t *data = NULL;

	switch (HIBYTE(req->wValue))
	{
#ifdef USB_USE_LPM
	case USB_DESC_TYPE_BOS:
#error "This BOS descriptor must be implemented for LPM mode"
		break;
#endif
	case USB_DESC_TYPE_DEVICE:
		data = (uint8_t *)cUsbDeviceDescriptor;
		len = sizeof(cUsbDeviceDescriptor);
		break;

	case USB_DESC_TYPE_CONFIGURATION:
		data = (uint8_t *)USB_CLASS_DEVICE_DESCRIPTOR;
		len = sizeof(USB_CLASS_DEVICE_DESCRIPTOR);
		break;

	case USB_DESC_TYPE_STRING:
		// These descriptors will be copied into the CTL buffer.
		data = gCTL.buffer;
		switch ((uint8_t)(req->wValue))
		{
		case USBD_IDX_LANGID_STR:
			len = USB_CTL_GetLangIdDescriptor(data);
			break;
		case USBD_IDX_MFC_STR:
			len = USB_CTL_GetStrDescriptor(data, USB_MANUFACTURER_STRING);
			break;
		case USBD_IDX_PRODUCT_STR:
			len = USB_CTL_GetStrDescriptor(data, USB_PRODUCT_STRING);
			break;
		case USBD_IDX_SERIAL_STR:
			len = USB_CTL_GetSerialDescriptor(data);
			break;
		case USBD_IDX_CONFIG_STR:
			len = USB_CTL_GetStrDescriptor(data, USB_CONFIGURATION_STRING);
			break;
		case USBD_IDX_INTERFACE_STR:
			len = USB_CTL_GetStrDescriptor(data, USB_INTERFACE_STRING);
			break;
		}
		break;

	case USB_DESC_TYPE_OTHER_SPEED_CONFIGURATION:
	case USB_DESC_TYPE_DEVICE_QUALIFIER:
		// We do not support full speed mode, so stalling on these requests is valid
		break;
	}

	if (len == 0)
	{
		USB_CTL_Error();
	}
	else if (req->wLength == 0)
	{
		// No data was requested.
		USB_CTL_SendStatus();
	}
	else
	{
		// Note: a partial descriptor may be requested
		USB_CTL_Send(data, MIN(len, req->wLength));
	}
}

static uint16_t USB_CTL_GetStrDescriptor(uint8_t * data, const char * str)
{
	uint16_t i = 2;
	while (*str != 0)
	{
		data[i++] = *str++;
		data[i++] = 0;
	}
	data[0] = i;
	data[1] = USB_DESC_TYPE_STRING;
	return i;
}

static void USB_CTL_IntToUnicode(uint32_t value, uint8_t * data, uint32_t size)
{
	while (size--)
	{
		uint32_t v = value >> 28;
		value <<= 4;
		*data++ = (v < 0x0A) ? v + '0' : v + ('A' - 10);
		*data++ = 0;
	}
}

static uint16_t USB_CTL_GetSerialDescriptor(uint8_t * data)
{
	uint32_t s0 = *((uint32_t*)(UID_BASE + 0));
	uint32_t s1 = *((uint32_t*)(UID_BASE + 4));
	uint32_t s2 = *((uint32_t*)(UID_BASE + 8));
	s0 += s2;

	// We are slamming our 96bit (12) UID into a 6 byte string.
	// Is this a good idea??
	// One byte goes to 2 unicode characters 0x1F => { 0, '1', 0, 'F' }

	data[0] = 24 + 2;
	data[1] = USB_DESC_TYPE_STRING;
	USB_CTL_IntToUnicode(s0, data + 2, 8);
	USB_CTL_IntToUnicode(s1, data + 18, 4);
	return 24 + 2;
}

static uint16_t USB_CTL_GetLangIdDescriptor(uint8_t * data)
{
	data[0] = 4;
	data[1] = USB_DESC_TYPE_STRING;
	data[2] = LOBYTE(USB_LANGID);
	data[3] = HIBYTE(USB_LANGID);
	return 4;
}

void USB_CTL_Error(void)
{
	USB_EP_Stall(CTL_IN_EP);
	USB_EP_Stall(CTL_OUT_EP);
}

/*
 * INTERRUPT ROUTINES
 */

void USB_CTL_DataOutStage(uint8_t epnum, uint8_t * pdata)
{
	USBD_HandleTypeDef * pdev = &hUsbDeviceFS;
	USBD_EndpointTypeDef *pep;

	if (epnum == 0U)
	{
		pep = &pdev->ep_out[0];

		if (pdev->ep0_state == USBD_EP0_DATA_OUT)
		{
			if (pep->rem_length > pep->maxpacket)
			{
				pep->rem_length -= pep->maxpacket;

				USB_EP_Read(CTL_OUT_EP, pdata, MIN(pep->rem_length, pep->maxpacket));
			}
			else
			{
				if ((pdev->pClass->EP0_RxReady != NULL) &&
						(gCTL.state == USB_STATE_CONFIGURED))
				{
					pdev->pClass->EP0_RxReady(pdev);
				}
				USB_CTL_SendStatus();
			}
		}
		else
		{
			if (pdev->ep0_state == USBD_EP0_STATUS_OUT)
			{
				/*
				 * STATUS PHASE completed, update ep0_state to idle
				 */
				pdev->ep0_state = USBD_EP0_IDLE;
				USB_EP_Stall(CTL_OUT_EP);
			}
		}
	}
	else if ((pdev->pClass->DataOut != NULL) &&
			(gCTL.state == USB_STATE_CONFIGURED))
	{
		pdev->pClass->DataOut(pdev, epnum);
	}
}

void USB_CTL_DataInStage(uint8_t epnum, uint8_t * pdata)
{
	USBD_HandleTypeDef * pdev = &hUsbDeviceFS;
	USBD_EndpointTypeDef *pep;

	if (epnum == 0U)
	{
		pep = &pdev->ep_in[0];

		if (pdev->ep0_state == USBD_EP0_DATA_IN)
		{
			if (pep->rem_length > pep->maxpacket)
			{
				pep->rem_length -= pep->maxpacket;
				USB_EP_Write(CTL_IN_EP, pdata, pep->rem_length);
				USB_EP_Read(CTL_OUT_EP, NULL, 0);
			}
			else
			{
				/* last packet is MPS multiple, so send ZLP packet */
				if ((pep->total_length % pep->maxpacket == 0U) &&
						(pep->total_length >= pep->maxpacket) &&
						(pep->total_length < pdev->ep0_data_len))
				{
					USB_EP_Write(CTL_IN_EP, NULL, 0);
					pdev->ep0_data_len = 0U;
					USB_EP_Read(CTL_OUT_EP, NULL, 0);
				}
				else
				{
					if ((pdev->pClass->EP0_TxSent != NULL) &&
							(gCTL.state == USB_STATE_CONFIGURED))
					{
						pdev->pClass->EP0_TxSent(pdev);
					}
					USB_EP_Stall(CTL_IN_EP);
					USB_CTL_ReceiveStatus();
				}
			}
		}
		else
		{
			if ((pdev->ep0_state == USBD_EP0_STATUS_IN) ||
					(pdev->ep0_state == USBD_EP0_IDLE))
			{
				USB_EP_Stall(CTL_IN_EP);
			}
		}

		if (gCTL.address != 0)
		{
			USB_PCD_SetAddress(gCTL.address);
			gCTL.address = 0;
		}
	}
	else if ((pdev->pClass->DataIn != NULL) &&
			(gCTL.state == USB_STATE_CONFIGURED))
	{
		pdev->pClass->DataIn(pdev, epnum);
	}
}

/*
void USB_CTL_Suspend(void)
{
	USBD_HandleTypeDef * pdev = &hUsbDeviceFS;
	pdev->dev_old_state =  pdev->dev_state;
	pdev->dev_state  = USBD_STATE_SUSPENDED;
}

void USB_CTL_Resume(void)
{
	USBD_HandleTypeDef * pdev = &hUsbDeviceFS;
	if (pdev->dev_state == USBD_STATE_SUSPENDED)
	{
		pdev->dev_state = pdev->dev_old_state;
	}
}
*/

