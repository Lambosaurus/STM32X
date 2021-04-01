
#include "USB_CDC.h"

#include "USB_EP.h"
#include "USB_PCD.h"
#include "USB_CTL.h"
#include "USB_Defs.h"

#include "Core.h"

/*
 * PRIVATE DEFINITIONS
 */

#ifdef USB_CDC_BFR_SIZE
#define CDC_BFR_SIZE 	USB_CDC_BFR_SIZE
#else
#define CDC_BFR_SIZE 512
#endif

#define CDC_BFR_WRAP(v) ((v) & (CDC_BFR_SIZE - 1))

#if (CDC_BFR_WRAP(CDC_BFR_SIZE) != 0)
#error "USB_CDC_BFR_SIZE must be a power of two"
#endif


#define CDC_IN_EP                                   0x81
#define CDC_OUT_EP                                  0x01
#define CDC_CMD_EP                                  0x82

#define CDC_BINTERVAL                          		0x10
#define CDC_PACKET_SIZE								USB_PACKET_SIZE
#define CDC_CMD_PACKET_SIZE                         8

#define CDC_SEND_ENCAPSULATED_COMMAND               0x00U
#define CDC_GET_ENCAPSULATED_RESPONSE               0x01U
#define CDC_SET_COMM_FEATURE                        0x02U
#define CDC_GET_COMM_FEATURE                        0x03U
#define CDC_CLEAR_COMM_FEATURE                      0x04U
#define CDC_SET_LINE_CODING                         0x20U
#define CDC_GET_LINE_CODING                         0x21U
#define CDC_SET_CONTROL_LINE_STATE                  0x22U
#define CDC_SEND_BREAK                              0x23U


/*
 * PRIVATE TYPES
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
		uint32_t data[CDC_CMD_PACKET_SIZE/4];
	}cmd;
	uint8_t lineCoding[7];
} CDC_t;

/*
 * PRIVATE PROTOTYPES
 */

static void USB_CDC_ReceiveData(uint8_t* data, uint32_t count);
static void USB_CDC_Control(uint8_t cmd, uint8_t* data, uint16_t length);

/*
 * PRIVATE VARIABLES
 */

__ALIGNED(4) const uint8_t cUSB_CDC_ConfigDescriptor[USB_CDC_CONFIG_DESC_SIZE] =
{
  //Configuration Descriptor
  0x09,                     // bLength: Configuration Descriptor size
  USB_DESC_TYPE_CONFIGURATION, // bDescriptorType: Configuration
  USB_CDC_CONFIG_DESC_SIZE,  // wTotalLength:no of returned bytes
  0x00,
  0x02,                     // bNumInterfaces: 2 interface
  0x01,                     // bConfigurationValue: Configuration value
  0x00,                     // iConfiguration: Index of string descriptor describing the configuration
  0xC0,                     // bmAttributes: self powered
  USB_MAX_POWER,            // MaxPower 100 mA
  //---------------------------------------------------------------------------

  // Interface Descriptor
  0x09,                     // bLength: Interface Descriptor size
  USB_DESC_TYPE_INTERFACE,  // bDescriptorType: Interface
  // Interface descriptor type
  0x00,                     // bInterfaceNumber: Number of Interface
  0x00,                     // bAlternateSetting: Alternate setting
  0x01,                     // bNumEndpoints: One endpoints used
  0x02,                     // bInterfaceClass: Communication Interface Class
  0x02,                     // bInterfaceSubClass: Abstract Control Model
  0x01,                     // bInterfaceProtocol: Common AT commands
  0x00,                     // iInterface:

  // Header Functional Descriptor
  0x05,                     // bLength: Endpoint Descriptor size
  0x24,                     // bDescriptorType: CS_INTERFACE
  0x00,                     // bDescriptorSubtype: Header Func Desc
  0x10,                     // bcdCDC: spec release number
  0x01,

  // Call Management Functional Descriptor
  0x05,                     // bFunctionLength
  0x24,                     // bDescriptorType: CS_INTERFACE
  0x01,                     // bDescriptorSubtype: Call Management Func Desc
  0x00,                     // bmCapabilities: D0+D1
  0x01,                     // bDataInterface: 1

  // ACM Functional Descriptor
  0x04,                     // bFunctionLength
  0x24,                     // bDescriptorType: CS_INTERFACE
  0x02,                     // bDescriptorSubtype: Abstract Control Management desc
  0x02,                     // bmCapabilities

  // Union Functional Descriptor
  0x05,                     // bFunctionLength
  0x24,                     // bDescriptorType: CS_INTERFACE
  0x06,                     // bDescriptorSubtype: Union func desc
  0x00,                     // bMasterInterface: Communication class interface
  0x01,                     // bSlaveInterface0: Data Class Interface

  // Endpoint 2 Descriptor
  0x07,                     // bLength: Endpoint Descriptor size
  USB_DESC_TYPE_ENDPOINT,   // bDescriptorType: Endpoint
  CDC_CMD_EP,               // bEndpointAddress
  0x03,                     // bmAttributes: Interrupt
  LOBYTE(CDC_CMD_PACKET_SIZE),  // wMaxPacketSize:
  HIBYTE(CDC_CMD_PACKET_SIZE),
  CDC_BINTERVAL,            // bInterval:
  /*---------------------------------------------------------------------------*/

  // Data class interface descriptor
  0x09,                     // bLength: Endpoint Descriptor size
  USB_DESC_TYPE_INTERFACE,  // bDescriptorType:
  0x01,                     // bInterfaceNumber: Number of Interface
  0x00,                     // bAlternateSetting: Alternate setting
  0x02,                     // bNumEndpoints: Two endpoints used
  0x0A,                     // bInterfaceClass: CDC
  0x00,                     // bInterfaceSubClass:
  0x00,                     // bInterfaceProtocol:
  0x00,                     // iInterface:

  // Endpoint OUT Descriptor
  0x07,                     // bLength: Endpoint Descriptor size
  USB_DESC_TYPE_ENDPOINT,   // bDescriptorType: Endpoint
  CDC_OUT_EP,               // bEndpointAddress
  0x02,                     // bmAttributes: Bulk
  LOBYTE(CDC_PACKET_SIZE),  // wMaxPacketSize:
  HIBYTE(CDC_PACKET_SIZE),
  0x00,                     // bInterval: ignore for Bulk transfer

  // Endpoint IN Descriptor
  0x07,                     // bLength: Endpoint Descriptor size
  USB_DESC_TYPE_ENDPOINT,   // bDescriptorType: Endpoint
  CDC_IN_EP,                // bEndpointAddress
  0x02,                     // bmAttributes: Bulk
  LOBYTE(CDC_PACKET_SIZE),  // wMaxPacketSize:
  HIBYTE(CDC_PACKET_SIZE),
  0x00                      // bInterval: ignore for Bulk transfer
};

static uint8_t gRxBuffer[CDC_PACKET_SIZE];
static CDCBuffer_t gRx;

static CDC_t gCDC;

/*
 * PUBLIC FUNCTIONS
 */

void USB_CDC_Init(uint8_t config)
{
	gRx.head = gRx.tail = 0;
	gCDC.txBusy = false;

	// Data endpoints
	USB_EP_Open(CDC_IN_EP, USB_EP_TYPE_BULK, CDC_PACKET_SIZE);
	USB_EP_Open(CDC_OUT_EP, USB_EP_TYPE_BULK, CDC_PACKET_SIZE);
	USB_EP_Open(CDC_CMD_EP, USB_EP_TYPE_BULK, CDC_CMD_PACKET_SIZE);

	USB_EP_Read(CDC_OUT_EP, gRxBuffer, CDC_PACKET_SIZE);

	// 115200bps, 1stop, no parity, 8bit
	uint8_t lineCoding[] = { 0x00, 0xC2, 0x01, 0x00, 0x00, 0x00, 0x08 };
	memcpy(gCDC.lineCoding, lineCoding, sizeof(gCDC.lineCoding));
}

void USB_CDC_Deinit(void)
{
	USB_EP_Close(CDC_IN_EP);
	USB_EP_Close(CDC_OUT_EP);
	USB_EP_Close(CDC_CMD_EP);
	gRx.head = gRx.tail = 0;
}

void USB_CDC_Write(const uint8_t * data, uint32_t count)
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
			uint32_t packet_size = count > CDC_PACKET_SIZE ? CDC_PACKET_SIZE : count;
			gCDC.txBusy = true;
			USB_EP_Write(CDC_IN_EP, data, count);
			count -= packet_size;
			data += packet_size;
		}
	}
}

uint32_t USB_CDC_ReadReady(void)
{
	// Assume these reads are atomic
	uint32_t count = CDC_BFR_WRAP(gRx.head - gRx.tail);
	return count;
}

uint32_t USB_CDC_Read(uint8_t * data, uint32_t count)
{
	uint32_t ready = USB_CDC_ReadReady();

	if (count > ready)
	{
		count = ready;
	}
	if (count > 0)
	{
		uint32_t tail = gRx.tail;
		uint32_t newtail = CDC_BFR_WRAP( tail + count );
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

void USB_CDC_CtlRxReady(void)
{
	if (gCDC.cmd.opcode != 0xFF)
	{
		USB_CDC_Control(gCDC.cmd.opcode, (uint8_t *)gCDC.cmd.data,  gCDC.cmd.size);
		gCDC.cmd.opcode = 0xFF;
	}
}

void USB_CDC_Setup(USB_SetupRequest_t * req)
{
	if (req->wLength)
	{
		if (req->bmRequest & 0x80U)
		{
			USB_CDC_Control(req->bRequest, (uint8_t *)gCDC.cmd.data, req->wLength);
			USB_CTL_Send((uint8_t *)gCDC.cmd.data, req->wLength);
		}
		else
		{
			gCDC.cmd.opcode = req->bRequest;
			gCDC.cmd.size = req->wLength;
			USB_CTL_Receive((uint8_t *)gCDC.cmd.data, req->wLength);
		}
	}
	else
	{
		USB_CDC_Control(req->bRequest, (uint8_t *)req, 0U);
	}
}

/*
 * PRIVATE FUNCTIONS
 */

static void USB_CDC_Control(uint8_t cmd, uint8_t* data, uint16_t length)
{
	switch(cmd)
	{
	case CDC_SET_LINE_CODING:
		memcpy(gCDC.lineCoding, data, sizeof(gCDC.lineCoding));
		break;
	case CDC_GET_LINE_CODING:
		memcpy(data, gCDC.lineCoding, sizeof(gCDC.lineCoding));
		break;
	case CDC_SEND_ENCAPSULATED_COMMAND:
	case CDC_GET_ENCAPSULATED_RESPONSE:
	case CDC_SET_COMM_FEATURE:
	case CDC_GET_COMM_FEATURE:
	case CDC_CLEAR_COMM_FEATURE:
	case CDC_SET_CONTROL_LINE_STATE:
	case CDC_SEND_BREAK:
	default:
		break;
	}
}

static void USB_CDC_ReceiveData(uint8_t* data, uint32_t count)
{
	// Minus 1 because head == tail represents the empty condition.
	uint32_t space = CDC_BFR_WRAP(gRx.tail - gRx.head - 1);

	if (count > space)
	{
		// Discard any data that we cannot insert into the buffer.
		count = space;
	}
	if (count > 0)
	{
		uint32_t head = gRx.head;
		uint32_t newhead = CDC_BFR_WRAP( head + count );
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

	USB_EP_Read(CDC_OUT_EP, gRxBuffer, CDC_PACKET_SIZE);
}

/*
 * DELETE LATER
 */

void USB_CDC_DataIn(uint8_t epnum)
{
	USBD_HandleTypeDef * pdev = &hUsbDeviceFS;
	if ((pdev->ep_in[epnum].total_length > 0U) && ((pdev->ep_in[epnum].total_length % hpcd_USB_FS.IN_ep[epnum].maxpacket) == 0U))
	{
		// Send ZLP
		USB_EP_Write(epnum, NULL, 0U);
	}
	else
	{
		gCDC.txBusy = false;
	}
}

void USB_CDC_DataOut(uint8_t epnum)
{
	USB_CDC_ReceiveData(gRxBuffer, USB_EP_RxCount(epnum));
}
