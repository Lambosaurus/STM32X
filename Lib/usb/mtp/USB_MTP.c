
#include "USB_MTP.h"

#ifdef USB_CLASS_MTP
#include "../USB_EP.h"
#include "../USB_CTL.h"

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
			0x02, // 2 endpoints used
			0x03, // Human Interface Device
			0x00, // 0x01 for Boot compatible device
			0x00 // 0x01 for boot-compatible-keyboard, 2 for boot-compatible-mouse,
	),
	USB_DESCR_BLOCK_ENDPOINT( MTP_IN_EP, 0x02, MTP_PACKET_SIZE, 0),
	USB_DESCR_BLOCK_ENDPOINT( MTP_OUT_EP, 0x02, MTP_PACKET_SIZE, 0),
};


static struct {
	bool isBusy;
} gMtp;


/*
 * PUBLIC FUNCTIONS
 */

void USB_MTP_Init(uint8_t config)
{
	// Data endpoints
	USB_EP_Open(MTP_IN_EP, USB_EP_TYPE_BULK, MTP_PACKET_SIZE, USB_MTP_TransmitDone);
	USB_EP_Open(MTP_OUT_EP, USB_EP_TYPE_BULK, MTP_PACKET_SIZE, USB_MTP_Receive);

	//USB_EP_Read(MTP_OUT_EP, gHid.rx, sizeof(gHid.rx));
}

void USB_MTP_Deinit(void)
{
	USB_EP_Close(MTP_IN_EP);
	USB_EP_Close(MTP_OUT_EP);
}

void USB_MTP_Setup(USB_SetupRequest_t * req)
{
}

/*
 * PRIVATE FUNCTIONS
 */

static void USB_MTP_TransmitDone(uint32_t count)
{
	 gMtp.isBusy = false;
}

static void USB_MTP_Receive(uint32_t count)
{
	//USB_EP_Read(MTP_OUT_EP, gHid.rx, sizeof(gHid.rx));
}


#endif //USB_CLASS_MTP

