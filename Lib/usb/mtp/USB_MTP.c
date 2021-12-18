
#include "USB_MTP.h"

#ifdef USB_CLASS_MTP
#include "../USB_EP.h"
#include "../USB_CTL.h"

#include "MTP_Defs.h"
#include <string.h>

/*
 * PRIVATE DEFINITIONS
 */


// Having adjustable endpoint and interface offsets is required for composite device support
#ifndef USB_MTP_ENDPOINT_BASE
#define USB_MTP_ENDPOINT_BASE				1
#endif

#ifndef USB_MTP_INTERFACE_BASE
#define USB_MTP_INTERFACE_BASE				0
#endif

#define IN_EP(n)							((USB_MTP_ENDPOINT_BASE + n) | 0x80)
#define OUT_EP(n)							(USB_MTP_ENDPOINT_BASE + n)

#define MTP_IN_EP                       	IN_EP(0)
#define MTP_OUT_EP                     		OUT_EP(0)
#define MTP_CMP_EP							IN_EP(1)

#define MTP_PACKET_SIZE						USB_PACKET_SIZE


/*
 * PRIVATE TYPES
 */

/*
 * PRIVATE PROTOTYPES
 */

static void USB_MTP_TransmitDone(uint32_t count);
static void USB_MTP_Receive(uint32_t count);

/*
 * PRIVATE VARIABLES
 */

__ALIGNED(4) const uint8_t cUSB_MTP_ConfigDescriptor[USB_MTP_CONFIG_DESC_SIZE] =
{
	USB_DESCR_BLOCK_CONFIGURATION(
			USB_MTP_CONFIG_DESC_SIZE,
			0x01, // 1 interfaces available
			0x01
			),
	USB_DESCR_BLOCK_INTERFACE(
			USB_MTP_INTERFACE_BASE,
			0x03, // 2 endpoints used
			0x06, // PTP device
			0x01, // MTP interface
			0x01
	),
	USB_DESCR_BLOCK_ENDPOINT( MTP_IN_EP, 0x02, MTP_PACKET_SIZE, 0),
	USB_DESCR_BLOCK_ENDPOINT( MTP_OUT_EP, 0x02, MTP_PACKET_SIZE, 0),
	USB_DESCR_BLOCK_ENDPOINT( MTP_CMP_EP, 0x03, MTP_CMD_SIZE, 0x10),
};


static struct {
	bool isBusy;
	MTP_Operation_t operation;
} gMtp;


/*
 * PUBLIC FUNCTIONS
 */

void USB_MTP_Init(uint8_t config)
{
	// Data endpoints
	USB_EP_Open(MTP_IN_EP, USB_EP_TYPE_BULK, MTP_PACKET_SIZE, USB_MTP_TransmitDone);
	USB_EP_Open(MTP_OUT_EP, USB_EP_TYPE_BULK, MTP_PACKET_SIZE, USB_MTP_Receive);
	USB_EP_Open(MTP_CMP_EP, USB_EP_TYPE_INTR, MTP_CMD_SIZE, NULL);

	USB_EP_Read(MTP_OUT_EP, (uint8_t *)(&gMtp.operation), MTP_OPERATION_SIZE);
}

void USB_MTP_Deinit(void)
{
	USB_EP_Close(MTP_IN_EP);
	USB_EP_Close(MTP_OUT_EP);
	USB_EP_Close(MTP_CMP_EP);
}

void USB_MTP_Setup(USB_SetupRequest_t * req)
{
}

/*
 * PRIVATE FUNCTIONS
 */


static uint8_t * MTP_Write32(uint8_t * dst, uint32_t value)
{
	*dst++ = (uint8_t)(value >> 24);
	*dst++ = (uint8_t)(value >> 16);
	*dst++ = (uint8_t)(value >> 8);
	*dst++ = (uint8_t)(value);
	return dst;
}

static uint8_t * MTP_Write16(uint8_t * dst, uint16_t value)
{
	*dst++ = (uint8_t)(value >> 8);
	*dst++ = (uint8_t)(value);
	return dst;
}

static uint8_t * MTP_WriteArray32(uint8_t * dst, const uint32_t * array, uint32_t count)
{
	dst = MTP_Write32(dst, count);
	while(count--)
	{
		dst = MTP_Write32(dst, *array++);
	}
	return dst;
}

static uint8_t * MTP_WriteArray16(uint8_t * dst, const uint16_t * array, uint32_t count)
{
	dst = MTP_Write32(dst, count);
	while(count--)
	{
		dst = MTP_Write16(dst, *array++);
	}
	return dst;
}

static uint8_t * MTP_WriteString(uint8_t * dst, const char * str)
{
	uint8_t len = strlen(str);

	// The null character should be written if string is not empty
	if (len > 0) { len += 1; }

	*dst++ = len;

	while (len--)
	{
		// Unicode format
		*dst++ = *str++;
		*dst++ = 0;
	}

	return dst;
}

static void USB_MTP_GetDeviceInfo(uint8_t * dst)
{
	// Standard version: 1.0
	dst = MTP_Write16(dst, 100);
	// MTP vendor extension: None
	dst = MTP_Write32(dst, 0xFFFFFFFF);
	// MTP version: 1.1
	dst = MTP_Write32(dst, 110);
	// MTP extentions: None?
	dst = MTP_WriteString(dst, "");
	// Functional mode: Standard
	dst = MTP_Write16(dst, 0);

	// Supported operations
	dst = MTP_WriteArray16(dst, NULL, 0);
	// Supported events
	dst = MTP_WriteArray16(dst, NULL, 0);
	// Device properties
	dst = MTP_WriteArray16(dst, NULL, 0);

	const uint16_t supported_formats[] = { MTP_OBJ_UNDEF };

	// Capture formats (formats emitted by the device)
	dst = MTP_WriteArray16(dst, supported_formats, LENGTH(supported_formats));
	// Playback formats (formats supported by the device)
	dst = MTP_WriteArray16(dst, supported_formats, LENGTH(supported_formats));

	// Manufacturer
	dst = MTP_WriteString(dst, USB_MANUFACTURER_STRING);
	// Model
	dst = MTP_WriteString(dst, USB_PRODUCT_STRING);
	// Device version
	dst = MTP_WriteString(dst, "1.0");
	// Device serial: TODO ...
	dst = MTP_WriteString(dst, "0000");
}

static void USB_MTP_TransmitDone(uint32_t count)
{
	 gMtp.isBusy = false;
}

static void USB_MTP_Receive(uint32_t count)
{
	__BKPT();

	//USB_EP_Read(MTP_OUT_EP, gHid.rx, sizeof(gHid.rx));
}


#endif //USB_CLASS_MTP

