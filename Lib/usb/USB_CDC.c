
#include "USB_CDC.h"
#include "USB_PCD.h"
#include "Core.h"

/*
 * PRIVATE DEFINITIONS
 */

#if defined(STM32L0)

#elif defined(STM32F0)

#endif

/*
 * PRIVATE TYPES
 */

#ifdef USB_CDC_BFR_SIZE
#define CDC_BFR_SIZE 	USB_CDC_BFR_SIZE
#else
#define CDC_BFR_SIZE 512
#endif
#if ((CDC_BFR_SIZE & (CDC_BFR_SIZE - 1)) != 0)
#error "USB_CDC_BFR_SIZE must be a power of two"
#endif

#define CDC_BFR_INCR(v, count) ((v + count) & (CDC_BFR_SIZE - 1))

#define _CDC_DISABLE_RX()		__disable_irq()
#define _CDC_ENABLE_RX()		__enable_irq()


/*
 * PUBLIC TYPES
 */

typedef struct
{
	uint8_t buffer[CDC_BFR_SIZE];
	uint32_t head;
	uint32_t tail;
} CDCBuffer_t;


typedef struct
{
	__IO bool txBusy;
	struct {
		uint8_t opcode;
		uint8_t size;
		// Not sure why this needs to be aligned?
		uint32_t data[USB_PACKET_SIZE/4];
	}cmd;
} CDC_t;

/*
 * PRIVATE PROTOTYPES
 */

static void USB_CDC_ReceiveData(uint8_t* data, uint32_t count);

/*
 * PRIVATE VARIABLES
 */

static uint8_t gRxBuffer[USB_PACKET_SIZE];
static CDCBuffer_t gRx;

static CDC_t gCDC;

/*
 * PUBLIC FUNCTIONS
 */

void USB_CDC_Init(void)
{
	gRx.head = gRx.tail = 0;
	gCDC.txBusy = false;

	// Data endpoints
	USB_PCD_EP_Open(CDC_IN_EP, USBD_EP_TYPE_BULK, USB_PACKET_SIZE, false);
	USB_PCD_EP_Open(CDC_OUT_EP, USBD_EP_TYPE_BULK, USB_PACKET_SIZE, false);
	// Command endpoint
	USB_PCD_EP_Open(CDC_CMD_EP, USBD_EP_TYPE_BULK, USB_PACKET_SIZE, false);

	USB_PCD_EP_StartRx(CDC_OUT_EP, gRxBuffer, USB_PACKET_SIZE);
}

void USB_CDC_Deinit(void)
{
	USB_PCD_EP_Close(CDC_IN_EP);
	USB_PCD_EP_Close(CDC_OUT_EP);
	USB_PCD_EP_Close(CDC_CMD_EP);
	gRx.head = gRx.tail = 0;
}

void USB_CDC_Tx(const uint8_t * data, uint32_t count)
{
	// This will block if the transmitter is not free, or multiple packets are sent out.
	uint32_t tide = CORE_GetTick();
	while (count)
	{
		if (gCDC.txBusy)
		{
			// Wait for transmit to be free. Abort if it does not come free.
			if (CORE_GetTick() - tide > 10)
			{
				break;
			}
			CORE_Idle();
		}
		else
		{
			// Transmit a packet
			uint32_t packet_size = count > USB_PACKET_SIZE ? USB_PACKET_SIZE : count;
			gCDC.txBusy = true;
			USB_PCD_EP_StartTx(CDC_IN_EP, data, count);
			count -= packet_size;
			data += packet_size;
		}
	}
}

uint32_t USB_CDC_RxReady(void)
{
	_CDC_DISABLE_RX();
	uint32_t count = gRx.head >= gRx.tail
				   ? gRx.head - gRx.tail
				   : CDC_BFR_SIZE + gRx.head - gRx.tail;
	_CDC_ENABLE_RX();
	return count;
}

uint32_t USB_CDC_Rx(uint8_t * data, uint32_t count)
{
	uint32_t ready = USB_CDC_RxReady();

	if (count > ready)
	{
		count = ready;
	}
	if (count > 0)
	{
		uint32_t tail = gRx.tail;
		uint32_t newtail = CDC_BFR_INCR( tail, count );
		if (newtail > tail)
		{
			// We can read continuously from the buffer
			memcpy(data, gRx.buffer + tail, count);
		}
		else
		{
			// We read to end of buffer, then read from the start
			uint32_t chunk = CDC_BFR_SIZE - tail;
			memcpy(data, gRx.buffer + tail, chunk);
			memcpy(data + chunk, gRx.buffer, count - chunk);
		}
		gRx.tail = newtail;
	}
	return count;
}

/*
 * PRIVATE FUNCTIONS
 */

void USB_CDC_Control(uint8_t cmd, uint8_t* data, uint16_t length)
{
	switch(cmd)
	{
	case CDC_SEND_ENCAPSULATED_COMMAND:
	case CDC_GET_ENCAPSULATED_RESPONSE:
	case CDC_SET_COMM_FEATURE:
	case CDC_GET_COMM_FEATURE:
	case CDC_CLEAR_COMM_FEATURE:
	case CDC_SET_LINE_CODING:
	case CDC_GET_LINE_CODING:
	case CDC_SET_CONTROL_LINE_STATE:
	case CDC_SEND_BREAK:
	default:
		break;
	}
}

static void USB_CDC_ReceiveData(uint8_t* data, uint32_t count)
{
	// Minus 1 because head == tail represents the empty condition.
	uint32_t space = gRx.head >= gRx.tail
		   ? (CDC_BFR_SIZE - 1) + gRx.tail - gRx.head
		   : gRx.tail - gRx.head - 1;

	if (count > space)
	{
		// Discard any data that we cannot insert into the buffer.
		count = space;
	}
	if (count > 0)
	{
		uint32_t head = gRx.head;
		uint32_t newhead = CDC_BFR_INCR( head, count );
		if (newhead > head)
		{
			// We can write continuously into the buffer
			memcpy(gRx.buffer + head, data, count);
		}
		else
		{
			// We write to end of buffer, then write from the start
			uint32_t chunk = CDC_BFR_SIZE - head;
			memcpy(gRx.buffer + head, data, chunk);
			memcpy(gRx.buffer, data + chunk, count - chunk);
		}
		gRx.head = newhead;
	}

	USB_PCD_EP_StartRx(CDC_OUT_EP, gRxBuffer, USB_PACKET_SIZE);
}

/*
 * INTERRUPT ROUTINES
 */


/*
 * DELETE LATER
 */







#include "usbd_ctlreq.h"



static uint8_t  USBD_CDC_Init(USBD_HandleTypeDef *pdev, uint8_t cfgidx);
static uint8_t  USBD_CDC_DeInit(USBD_HandleTypeDef *pdev, uint8_t cfgidx);
static uint8_t  USBD_CDC_Setup(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req);
static uint8_t  USBD_CDC_DataIn(USBD_HandleTypeDef *pdev, uint8_t epnum);
static uint8_t  USBD_CDC_DataOut(USBD_HandleTypeDef *pdev, uint8_t epnum);
static uint8_t  USBD_CDC_EP0_RxReady(USBD_HandleTypeDef *pdev);
static uint8_t  *USBD_CDC_GetFSCfgDesc(uint16_t *length);
static uint8_t  *USBD_CDC_GetHSCfgDesc(uint16_t *length);
static uint8_t  *USBD_CDC_GetOtherSpeedCfgDesc(uint16_t *length);
static uint8_t  *USBD_CDC_GetOtherSpeedCfgDesc(uint16_t *length);
uint8_t  *USBD_CDC_GetDeviceQualifierDescriptor(uint16_t *length);

/* USB Standard Device Descriptor */
__ALIGN_BEGIN static uint8_t USBD_CDC_DeviceQualifierDesc[USB_LEN_DEV_QUALIFIER_DESC] __ALIGN_END =
{
  USB_LEN_DEV_QUALIFIER_DESC,
  USB_DESC_TYPE_DEVICE_QUALIFIER,
  0x00,
  0x02,
  0x00,
  0x00,
  0x00,
  0x40,
  0x01,
  0x00,
};


/* CDC interface class callbacks structure */
USBD_ClassTypeDef  USBD_CDC =
{
  USBD_CDC_Init,
  USBD_CDC_DeInit,
  USBD_CDC_Setup,
  NULL,                 /* EP0_TxSent, */
  USBD_CDC_EP0_RxReady,
  USBD_CDC_DataIn,
  USBD_CDC_DataOut,
  NULL,
  NULL,
  NULL,
  USBD_CDC_GetHSCfgDesc,
  USBD_CDC_GetFSCfgDesc,
  USBD_CDC_GetOtherSpeedCfgDesc,
  USBD_CDC_GetDeviceQualifierDescriptor,
};

/* USB CDC device Configuration Descriptor */
__ALIGN_BEGIN uint8_t USBD_CDC_CfgHSDesc[USB_CDC_CONFIG_DESC_SIZ] __ALIGN_END =
{
  /*Configuration Descriptor*/
  0x09,   /* bLength: Configuration Descriptor size */
  USB_DESC_TYPE_CONFIGURATION,      /* bDescriptorType: Configuration */
  USB_CDC_CONFIG_DESC_SIZ,                /* wTotalLength:no of returned bytes */
  0x00,
  0x02,   /* bNumInterfaces: 2 interface */
  0x01,   /* bConfigurationValue: Configuration value */
  0x00,   /* iConfiguration: Index of string descriptor describing the configuration */
  0xC0,   /* bmAttributes: self powered */
  0x32,   /* MaxPower 0 mA */

  /*---------------------------------------------------------------------------*/

  /*Interface Descriptor */
  0x09,   /* bLength: Interface Descriptor size */
  USB_DESC_TYPE_INTERFACE,  /* bDescriptorType: Interface */
  /* Interface descriptor type */
  0x00,   /* bInterfaceNumber: Number of Interface */
  0x00,   /* bAlternateSetting: Alternate setting */
  0x01,   /* bNumEndpoints: One endpoints used */
  0x02,   /* bInterfaceClass: Communication Interface Class */
  0x02,   /* bInterfaceSubClass: Abstract Control Model */
  0x01,   /* bInterfaceProtocol: Common AT commands */
  0x00,   /* iInterface: */

  /*Header Functional Descriptor*/
  0x05,   /* bLength: Endpoint Descriptor size */
  0x24,   /* bDescriptorType: CS_INTERFACE */
  0x00,   /* bDescriptorSubtype: Header Func Desc */
  0x10,   /* bcdCDC: spec release number */
  0x01,

  /*Call Management Functional Descriptor*/
  0x05,   /* bFunctionLength */
  0x24,   /* bDescriptorType: CS_INTERFACE */
  0x01,   /* bDescriptorSubtype: Call Management Func Desc */
  0x00,   /* bmCapabilities: D0+D1 */
  0x01,   /* bDataInterface: 1 */

  /*ACM Functional Descriptor*/
  0x04,   /* bFunctionLength */
  0x24,   /* bDescriptorType: CS_INTERFACE */
  0x02,   /* bDescriptorSubtype: Abstract Control Management desc */
  0x02,   /* bmCapabilities */

  /*Union Functional Descriptor*/
  0x05,   /* bFunctionLength */
  0x24,   /* bDescriptorType: CS_INTERFACE */
  0x06,   /* bDescriptorSubtype: Union func desc */
  0x00,   /* bMasterInterface: Communication class interface */
  0x01,   /* bSlaveInterface0: Data Class Interface */

  /*Endpoint 2 Descriptor*/
  0x07,                           /* bLength: Endpoint Descriptor size */
  USB_DESC_TYPE_ENDPOINT,   /* bDescriptorType: Endpoint */
  CDC_CMD_EP,                     /* bEndpointAddress */
  0x03,                           /* bmAttributes: Interrupt */
  LOBYTE(CDC_CMD_PACKET_SIZE),     /* wMaxPacketSize: */
  HIBYTE(CDC_CMD_PACKET_SIZE),
  CDC_HS_BINTERVAL,                           /* bInterval: */
  /*---------------------------------------------------------------------------*/

  /*Data class interface descriptor*/
  0x09,   /* bLength: Endpoint Descriptor size */
  USB_DESC_TYPE_INTERFACE,  /* bDescriptorType: */
  0x01,   /* bInterfaceNumber: Number of Interface */
  0x00,   /* bAlternateSetting: Alternate setting */
  0x02,   /* bNumEndpoints: Two endpoints used */
  0x0A,   /* bInterfaceClass: CDC */
  0x00,   /* bInterfaceSubClass: */
  0x00,   /* bInterfaceProtocol: */
  0x00,   /* iInterface: */

  /*Endpoint OUT Descriptor*/
  0x07,   /* bLength: Endpoint Descriptor size */
  USB_DESC_TYPE_ENDPOINT,      /* bDescriptorType: Endpoint */
  CDC_OUT_EP,                        /* bEndpointAddress */
  0x02,                              /* bmAttributes: Bulk */
  LOBYTE(CDC_DATA_HS_MAX_PACKET_SIZE),  /* wMaxPacketSize: */
  HIBYTE(CDC_DATA_HS_MAX_PACKET_SIZE),
  0x00,                              /* bInterval: ignore for Bulk transfer */

  /*Endpoint IN Descriptor*/
  0x07,   /* bLength: Endpoint Descriptor size */
  USB_DESC_TYPE_ENDPOINT,      /* bDescriptorType: Endpoint */
  CDC_IN_EP,                         /* bEndpointAddress */
  0x02,                              /* bmAttributes: Bulk */
  LOBYTE(CDC_DATA_HS_MAX_PACKET_SIZE),  /* wMaxPacketSize: */
  HIBYTE(CDC_DATA_HS_MAX_PACKET_SIZE),
  0x00                               /* bInterval: ignore for Bulk transfer */
} ;


/* USB CDC device Configuration Descriptor */
__ALIGN_BEGIN uint8_t USBD_CDC_CfgFSDesc[USB_CDC_CONFIG_DESC_SIZ] __ALIGN_END =
{
  /*Configuration Descriptor*/
  0x09,   /* bLength: Configuration Descriptor size */
  USB_DESC_TYPE_CONFIGURATION,      /* bDescriptorType: Configuration */
  USB_CDC_CONFIG_DESC_SIZ,                /* wTotalLength:no of returned bytes */
  0x00,
  0x02,   /* bNumInterfaces: 2 interface */
  0x01,   /* bConfigurationValue: Configuration value */
  0x00,   /* iConfiguration: Index of string descriptor describing the configuration */
  0xC0,   /* bmAttributes: self powered */
  0x32,   /* MaxPower 0 mA */

  /*---------------------------------------------------------------------------*/

  /*Interface Descriptor */
  0x09,   /* bLength: Interface Descriptor size */
  USB_DESC_TYPE_INTERFACE,  /* bDescriptorType: Interface */
  /* Interface descriptor type */
  0x00,   /* bInterfaceNumber: Number of Interface */
  0x00,   /* bAlternateSetting: Alternate setting */
  0x01,   /* bNumEndpoints: One endpoints used */
  0x02,   /* bInterfaceClass: Communication Interface Class */
  0x02,   /* bInterfaceSubClass: Abstract Control Model */
  0x01,   /* bInterfaceProtocol: Common AT commands */
  0x00,   /* iInterface: */

  /*Header Functional Descriptor*/
  0x05,   /* bLength: Endpoint Descriptor size */
  0x24,   /* bDescriptorType: CS_INTERFACE */
  0x00,   /* bDescriptorSubtype: Header Func Desc */
  0x10,   /* bcdCDC: spec release number */
  0x01,

  /*Call Management Functional Descriptor*/
  0x05,   /* bFunctionLength */
  0x24,   /* bDescriptorType: CS_INTERFACE */
  0x01,   /* bDescriptorSubtype: Call Management Func Desc */
  0x00,   /* bmCapabilities: D0+D1 */
  0x01,   /* bDataInterface: 1 */

  /*ACM Functional Descriptor*/
  0x04,   /* bFunctionLength */
  0x24,   /* bDescriptorType: CS_INTERFACE */
  0x02,   /* bDescriptorSubtype: Abstract Control Management desc */
  0x02,   /* bmCapabilities */

  /*Union Functional Descriptor*/
  0x05,   /* bFunctionLength */
  0x24,   /* bDescriptorType: CS_INTERFACE */
  0x06,   /* bDescriptorSubtype: Union func desc */
  0x00,   /* bMasterInterface: Communication class interface */
  0x01,   /* bSlaveInterface0: Data Class Interface */

  /*Endpoint 2 Descriptor*/
  0x07,                           /* bLength: Endpoint Descriptor size */
  USB_DESC_TYPE_ENDPOINT,   /* bDescriptorType: Endpoint */
  CDC_CMD_EP,                     /* bEndpointAddress */
  0x03,                           /* bmAttributes: Interrupt */
  LOBYTE(CDC_CMD_PACKET_SIZE),     /* wMaxPacketSize: */
  HIBYTE(CDC_CMD_PACKET_SIZE),
  CDC_FS_BINTERVAL,                           /* bInterval: */
  /*---------------------------------------------------------------------------*/

  /*Data class interface descriptor*/
  0x09,   /* bLength: Endpoint Descriptor size */
  USB_DESC_TYPE_INTERFACE,  /* bDescriptorType: */
  0x01,   /* bInterfaceNumber: Number of Interface */
  0x00,   /* bAlternateSetting: Alternate setting */
  0x02,   /* bNumEndpoints: Two endpoints used */
  0x0A,   /* bInterfaceClass: CDC */
  0x00,   /* bInterfaceSubClass: */
  0x00,   /* bInterfaceProtocol: */
  0x00,   /* iInterface: */

  /*Endpoint OUT Descriptor*/
  0x07,   /* bLength: Endpoint Descriptor size */
  USB_DESC_TYPE_ENDPOINT,      /* bDescriptorType: Endpoint */
  CDC_OUT_EP,                        /* bEndpointAddress */
  0x02,                              /* bmAttributes: Bulk */
  LOBYTE(CDC_DATA_FS_MAX_PACKET_SIZE),  /* wMaxPacketSize: */
  HIBYTE(CDC_DATA_FS_MAX_PACKET_SIZE),
  0x00,                              /* bInterval: ignore for Bulk transfer */

  /*Endpoint IN Descriptor*/
  0x07,   /* bLength: Endpoint Descriptor size */
  USB_DESC_TYPE_ENDPOINT,      /* bDescriptorType: Endpoint */
  CDC_IN_EP,                         /* bEndpointAddress */
  0x02,                              /* bmAttributes: Bulk */
  LOBYTE(CDC_DATA_FS_MAX_PACKET_SIZE),  /* wMaxPacketSize: */
  HIBYTE(CDC_DATA_FS_MAX_PACKET_SIZE),
  0x00                               /* bInterval: ignore for Bulk transfer */
};

__ALIGN_BEGIN uint8_t USBD_CDC_OtherSpeedCfgDesc[USB_CDC_CONFIG_DESC_SIZ] __ALIGN_END =
{
  0x09,   /* bLength: Configuation Descriptor size */
  USB_DESC_TYPE_OTHER_SPEED_CONFIGURATION,
  USB_CDC_CONFIG_DESC_SIZ,
  0x00,
  0x02,   /* bNumInterfaces: 2 interfaces */
  0x01,   /* bConfigurationValue: */
  0x04,   /* iConfiguration: */
  0xC0,   /* bmAttributes: */
  0x32,   /* MaxPower 100 mA */

  /*Interface Descriptor */
  0x09,   /* bLength: Interface Descriptor size */
  USB_DESC_TYPE_INTERFACE,  /* bDescriptorType: Interface */
  /* Interface descriptor type */
  0x00,   /* bInterfaceNumber: Number of Interface */
  0x00,   /* bAlternateSetting: Alternate setting */
  0x01,   /* bNumEndpoints: One endpoints used */
  0x02,   /* bInterfaceClass: Communication Interface Class */
  0x02,   /* bInterfaceSubClass: Abstract Control Model */
  0x01,   /* bInterfaceProtocol: Common AT commands */
  0x00,   /* iInterface: */

  /*Header Functional Descriptor*/
  0x05,   /* bLength: Endpoint Descriptor size */
  0x24,   /* bDescriptorType: CS_INTERFACE */
  0x00,   /* bDescriptorSubtype: Header Func Desc */
  0x10,   /* bcdCDC: spec release number */
  0x01,

  /*Call Management Functional Descriptor*/
  0x05,   /* bFunctionLength */
  0x24,   /* bDescriptorType: CS_INTERFACE */
  0x01,   /* bDescriptorSubtype: Call Management Func Desc */
  0x00,   /* bmCapabilities: D0+D1 */
  0x01,   /* bDataInterface: 1 */

  /*ACM Functional Descriptor*/
  0x04,   /* bFunctionLength */
  0x24,   /* bDescriptorType: CS_INTERFACE */
  0x02,   /* bDescriptorSubtype: Abstract Control Management desc */
  0x02,   /* bmCapabilities */

  /*Union Functional Descriptor*/
  0x05,   /* bFunctionLength */
  0x24,   /* bDescriptorType: CS_INTERFACE */
  0x06,   /* bDescriptorSubtype: Union func desc */
  0x00,   /* bMasterInterface: Communication class interface */
  0x01,   /* bSlaveInterface0: Data Class Interface */

  /*Endpoint 2 Descriptor*/
  0x07,                           /* bLength: Endpoint Descriptor size */
  USB_DESC_TYPE_ENDPOINT,         /* bDescriptorType: Endpoint */
  CDC_CMD_EP,                     /* bEndpointAddress */
  0x03,                           /* bmAttributes: Interrupt */
  LOBYTE(CDC_CMD_PACKET_SIZE),     /* wMaxPacketSize: */
  HIBYTE(CDC_CMD_PACKET_SIZE),
  CDC_FS_BINTERVAL,                           /* bInterval: */

  /*---------------------------------------------------------------------------*/

  /*Data class interface descriptor*/
  0x09,   /* bLength: Endpoint Descriptor size */
  USB_DESC_TYPE_INTERFACE,  /* bDescriptorType: */
  0x01,   /* bInterfaceNumber: Number of Interface */
  0x00,   /* bAlternateSetting: Alternate setting */
  0x02,   /* bNumEndpoints: Two endpoints used */
  0x0A,   /* bInterfaceClass: CDC */
  0x00,   /* bInterfaceSubClass: */
  0x00,   /* bInterfaceProtocol: */
  0x00,   /* iInterface: */

  /*Endpoint OUT Descriptor*/
  0x07,   /* bLength: Endpoint Descriptor size */
  USB_DESC_TYPE_ENDPOINT,      /* bDescriptorType: Endpoint */
  CDC_OUT_EP,                        /* bEndpointAddress */
  0x02,                              /* bmAttributes: Bulk */
  0x40,                              /* wMaxPacketSize: */
  0x00,
  0x00,                              /* bInterval: ignore for Bulk transfer */

  /*Endpoint IN Descriptor*/
  0x07,   /* bLength: Endpoint Descriptor size */
  USB_DESC_TYPE_ENDPOINT,     /* bDescriptorType: Endpoint */
  CDC_IN_EP,                        /* bEndpointAddress */
  0x02,                             /* bmAttributes: Bulk */
  0x40,                             /* wMaxPacketSize: */
  0x00,
  0x00                              /* bInterval */
};


static uint8_t  USBD_CDC_Init(USBD_HandleTypeDef *pdev, uint8_t cfgidx)
{
	USB_CDC_Init();
	pdev->pClassData = (void*)1;
	return 0;
}

static uint8_t  USBD_CDC_DeInit(USBD_HandleTypeDef *pdev, uint8_t cfgidx)
{
	USB_CDC_Deinit();
	pdev->pClassData = NULL;
	return 0;
}


static uint8_t  USBD_CDC_Setup(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
	uint8_t ret = USBD_FAIL;
	switch (req->bmRequest & USB_REQ_TYPE_MASK)
	{
	case USB_REQ_TYPE_CLASS :
		ret = USBD_OK;
		if (req->wLength)
		{
			if (req->bmRequest & 0x80U)
			{
				USB_CDC_Control(req->bRequest, (uint8_t *)gCDC.cmd.data, req->wLength);
				USBD_CtlSendData(pdev, (uint8_t *)gCDC.cmd.data, req->wLength);
			}
			else
			{
				gCDC.cmd.opcode = req->bRequest;
				gCDC.cmd.size = req->wLength;
				USBD_CtlPrepareRx(pdev, (uint8_t *)gCDC.cmd.data, req->wLength);
			}
		}
		else
		{
			USB_CDC_Control(req->bRequest, (uint8_t *)req, 0U);
		}
		break;
	case USB_REQ_TYPE_STANDARD:
		if (pdev->dev_state != USBD_STATE_CONFIGURED)
		{
			switch (req->bRequest)
			{
			case USB_REQ_GET_STATUS:
			{
				uint16_t status_info = 0;
				ret = USBD_CtlSendData(pdev, (uint8_t *)(void *)&status_info, 2U);
				break;
			}
			case USB_REQ_GET_INTERFACE:
			{
				uint8_t ifalt = 0;
				ret = USBD_CtlSendData(pdev, &ifalt, 1);
				break;
			}
			case USB_REQ_SET_INTERFACE:
				ret = USBD_OK;
				break;
			}
		}
	}

	if (ret == USBD_FAIL)
	{
		USBD_CtlError(pdev, req);
	}

	return ret;
}


static uint8_t  USBD_CDC_DataIn(USBD_HandleTypeDef *pdev, uint8_t epnum)
{
	PCD_HandleTypeDef *hpcd = pdev->pData;

	if ((pdev->ep_in[epnum].total_length > 0U) && ((pdev->ep_in[epnum].total_length % hpcd->IN_ep[epnum].maxpacket) == 0U))
	{
		/* Send ZLP */
		USB_PCD_EP_StartTx(epnum, NULL, 0U);
	}
	else
	{
		gCDC.txBusy = false;
	}
	return USBD_OK;
}

static uint8_t  USBD_CDC_DataOut(USBD_HandleTypeDef *pdev, uint8_t epnum)
{
	USB_CDC_ReceiveData(gRxBuffer, USBD_LL_GetRxDataSize(pdev, epnum));
	return USBD_OK;
}

static uint8_t  USBD_CDC_EP0_RxReady(USBD_HandleTypeDef *pdev)
{
	if (gCDC.cmd.opcode != 0xFF)
	{
		USB_CDC_Control(gCDC.cmd.opcode, (uint8_t *)gCDC.cmd.data,  gCDC.cmd.size);
		gCDC.cmd.opcode = 0xFF;
	}
	return USBD_OK;
}

static uint8_t  *USBD_CDC_GetFSCfgDesc(uint16_t *length)
{
  *length = sizeof(USBD_CDC_CfgFSDesc);
  return USBD_CDC_CfgFSDesc;
}

static uint8_t  *USBD_CDC_GetHSCfgDesc(uint16_t *length)
{
  *length = sizeof(USBD_CDC_CfgHSDesc);
  return USBD_CDC_CfgHSDesc;
}


static uint8_t  *USBD_CDC_GetOtherSpeedCfgDesc(uint16_t *length)
{
  *length = sizeof(USBD_CDC_OtherSpeedCfgDesc);
  return USBD_CDC_OtherSpeedCfgDesc;
}


uint8_t  *USBD_CDC_GetDeviceQualifierDescriptor(uint16_t *length)
{
  *length = sizeof(USBD_CDC_DeviceQualifierDesc);
  return USBD_CDC_DeviceQualifierDesc;
}

uint8_t USBD_CDC_ReceivePacket(USBD_HandleTypeDef *pdev)
{
	USB_PCD_EP_StartRx(CDC_OUT_EP, gRxBuffer, USB_PACKET_SIZE);
    return USBD_OK;
}

