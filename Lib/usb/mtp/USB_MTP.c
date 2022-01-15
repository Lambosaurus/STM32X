
#include "USB_MTP.h"

#ifdef USB_CLASS_MTP
#include "CORE.h"
#include "../USB_EP.h"
#include "../USB_CTL.h"

#include "MTP.h"
#include "MTP_FS.h"

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
#define MTP_EVT_EP							IN_EP(1)

#define MTP_PACKET_SIZE						USB_PACKET_SIZE


/*
 * PRIVATE TYPES
 */

/*
 * PRIVATE PROTOTYPES
 */

static void USB_MTP_TransmitDone(uint32_t count);
static void USB_MTP_Receive(uint32_t count);
static void USB_MTP_EnterState(MTP_State_t state);
static void USB_MTP_EventTransmitDone(uint32_t count);

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
	USB_DESCR_BLOCK_ENDPOINT( MTP_EVT_EP, 0x03, MTP_EVT_SIZE, 0x10),
};


static struct {
	uint8_t state;
	volatile bool event_busy;
	MTP_Operation_t operation;
	MTP_Container_t container;
	MTP_t * mtp;
} gMtp;


/*
 * PUBLIC FUNCTIONS
 */

void USB_MTP_Mount(MTP_t * mtp)
{
	gMtp.mtp = mtp;
}

void USB_MTP_Init(uint8_t config)
{
	// Data endpoints
	USB_EP_Open(MTP_IN_EP, USB_EP_TYPE_BULK, MTP_PACKET_SIZE, USB_MTP_TransmitDone);
	USB_EP_Open(MTP_OUT_EP, USB_EP_TYPE_BULK, MTP_PACKET_SIZE, USB_MTP_Receive);
	USB_EP_Open(MTP_EVT_EP, USB_EP_TYPE_INTR, MTP_EVT_SIZE, USB_MTP_EventTransmitDone);

	gMtp.state = MTP_Reset(gMtp.mtp);
	gMtp.event_busy = false;
	USB_MTP_EnterState(gMtp.state);
}

void USB_MTP_Deinit(void)
{
	USB_EP_Close(MTP_IN_EP);
	USB_EP_Close(MTP_OUT_EP);
	USB_EP_Close(MTP_EVT_EP);
}

void USB_MTP_Setup(USB_SetupRequest_t * req)
{
	// TODO .....
}

bool USB_MTP_SubmitEvent(MTP_Event_t * event)
{
	// Wait for the endpoint to be free. Timeout should be reviewed.
	uint32_t start = CORE_GetTick();
	while (CORE_GetTick() - start < 50 && !gMtp.event_busy)

	if (!gMtp.event_busy)
	{
		// TODO: This is an interrupt hazard.
		// If we had other events generated in IRQ's perhaps.
		gMtp.event_busy = true;
		USB_EP_Write(MTP_EVT_EP, (uint8_t *)event, event->length);
		return true;
	}
	return false;
}

/*
 * PRIVATE FUNCTIONS
 */

static void USB_MTP_EventTransmitDone(uint32_t count)
{
	gMtp.event_busy = false;
}

static void USB_MTP_TransmitDone(uint32_t count)
{
	switch (gMtp.state)
	{
	case MTP_State_TxDataLast:

		if (gMtp.container.packet_size != 0 && (gMtp.container.packet_size % MTP_PACKET_SIZE) == 0)
		{
			gMtp.container.packet_size = 0;
			break;
		}
		// Fallthrough.

	case MTP_State_TxData:
		gMtp.state = MTP_NextData(gMtp.mtp, &gMtp.operation, &gMtp.container);
		break;

	default:
		// We should NOT have got here.
	case MTP_State_TxResponse:
		// When a response is sent, we await next operation
		gMtp.state = MTP_State_RxOperation;
		break;
	}

	USB_MTP_EnterState(gMtp.state);
}

static void USB_MTP_Receive(uint32_t count)
{
	switch (gMtp.state)
	{
	case MTP_State_RxOperation:
		gMtp.state = MTP_HandleOperation(gMtp.mtp, &gMtp.operation, &gMtp.container);
		break;
	case MTP_State_RxData:
		gMtp.container.packet_size = count;
		gMtp.state = MTP_HandleData(gMtp.mtp, &gMtp.operation, &gMtp.container);
		break;
	default:
		// Unexpected cases. Await next operation.
		gMtp.state = MTP_State_RxOperation;
		break;
	}

	USB_MTP_EnterState(gMtp.state);
}

static void USB_MTP_EnterState(MTP_State_t state)
{
	switch (gMtp.state)
	{
	case MTP_State_RxOperation:
		// Await next operations
		USB_EP_Read(MTP_OUT_EP, (uint8_t *)(&gMtp.operation), MTP_OPERATION_SIZE);
		break;
	case MTP_State_RxData:
		USB_EP_Read(MTP_OUT_EP, (uint8_t *)(&gMtp.container), gMtp.container.packet_size);
		break;
	case MTP_State_TxData:
	case MTP_State_TxResponse:
	case MTP_State_TxDataLast:
		// Pump out the received data.
		USB_EP_Write(MTP_IN_EP, (uint8_t *)(&gMtp.container), gMtp.container.packet_size);
		break;
	}
}

#endif //USB_CLASS_MTP

