
#include "USB_HID.h"

#ifdef USB_CLASS_HID
#include "../USB_EP.h"
#include "../USB_CTL.h"

#include <string.h>


/*
 * PRIVATE DEFINITIONS
 */


// Having adjustable endpoint and interface offsets is required for composite device support
#ifndef USB_HID_ENDPOINT_BASE
#define USB_HID_ENDPOINT_BASE				1
#endif

#ifndef USB_HID_INTERFACE_BASE
#define USB_HID_INTERFACE_BASE				0
#endif

#define IN_EP(n)							((USB_HID_ENDPOINT_BASE + n) | 0x80)
#define OUT_EP(n)							(USB_HID_ENDPOINT_BASE + n)

#define HID_IN_EP                       	IN_EP(0)
#define HID_OUT_EP                     		OUT_EP(0)

#define HID_PACKET_SIZE						USB_PACKET_SIZE

#define HID_INTERVAL						0x01

/*
 * PRIVATE TYPES
 */

/*
 * PRIVATE PROTOTYPES
 */

static void USB_HID_GetDescriptor(USB_SetupRequest_t * req);
static void USB_HID_TransmitDone(uint32_t count);
static void USB_HID_Receive(uint32_t count);

/*
 * PRIVATE VARIABLES
 */

#define USB_HID_REPORT_DESC_SIZE			0x30


__ALIGNED(4) const uint8_t cUSB_HID_ReportDescriptor[USB_HID_REPORT_DESC_SIZE] =
{

};

__ALIGNED(4) const uint8_t cUSB_HID_ConfigDescriptor[USB_HID_CONFIG_DESC_SIZE] =
{
	USB_DESCR_BLOCK_CONFIGURATION(
			USB_HID_CONFIG_DESC_SIZE,
			0x01, // 1 interfaces available
			0x01
			),
	USB_DESCR_BLOCK_INTERFACE(
			USB_HID_INTERFACE_BASE,
			0x02, // 2 endpoints used
			0x03, // Human Interface Device
			0x00, // 0x01 for Boot compatible device
			0x00 // 0x01 for boot-compatible-keyboard, 2 for boot-compatible-mouse,
	),
	USB_DESCR_BLOCK_HID(
			USB_HID_REPORT_DESC_SIZE
	),
	USB_DESCR_BLOCK_ENDPOINT( HID_IN_EP, 0x03, HID_PACKET_SIZE, HID_INTERVAL),
	USB_DESCR_BLOCK_ENDPOINT( HID_OUT_EP, 0x03, HID_PACKET_SIZE, HID_INTERVAL),
};

/*
 * PUBLIC FUNCTIONS
 */

void USB_HID_Init(uint8_t config)
{
	// Data endpoints
	USB_EP_Open(HID_IN_EP, USB_EP_TYPE_INTR, HID_PACKET_SIZE, USB_HID_TransmitDone);
	USB_EP_Open(HID_OUT_EP, USB_EP_TYPE_INTR, HID_PACKET_SIZE, USB_HID_Receive);
}

void USB_HID_Deinit(void)
{
	USB_EP_Close(HID_IN_EP);
	USB_EP_Close(HID_OUT_EP);
}

void USB_HID_Setup(USB_SetupRequest_t * req)
{
	switch (req->bRequest)
	{
	case USB_REQ_GET_DESCRIPTOR:
		USB_HID_GetDescriptor(req);
		break;

		// TODO: Implement silence support
	}
	return;
}

/*
 * PRIVATE FUNCTIONS
 */

static void USB_HID_TransmitDone(uint32_t count)
{

}

static void USB_HID_Receive(uint32_t count)
{

}

static void USB_HID_GetDescriptor(USB_SetupRequest_t * req)
{
	uint8_t type = HIBYTE(req->wValue);
	uint8_t index = LOBYTE(req->wValue);

	uint16_t len = 0;
	const uint8_t * data = NULL;

	switch (type)
	{
	case USB_DESC_TYPE_HID_REPORT:
		len = sizeof(cUSB_HID_ReportDescriptor);
		data = cUSB_HID_ReportDescriptor;
		break;
	}

	// Note: a partial descriptor may be requested
	USB_CTL_Send(data, MIN(len, req->wLength));
}

#endif //USB_CLASS_HID

