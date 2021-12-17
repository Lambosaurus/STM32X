
#include "USB_HID.h"

#ifdef USB_CLASS_HID
#include "../USB_EP.h"
#include "../USB_CTL.h"

#include "HID_Defs.h"
#include "Core.h"
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



#define HID_REQ_SET_PROTOCOL          0x0B
#define HID_REQ_GET_PROTOCOL          0x03

#define HID_REQ_SET_IDLE              0x0A
#define HID_REQ_GET_IDLE              0x02

#define HID_REQ_SET_REPORT            0x09
#define HID_REQ_GET_REPORT            0x01

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

__ALIGNED(4) const uint8_t cUSB_HID_ReportDescriptor[USB_HID_REPORT_DESC_SIZE] =
{
	USB_HID_REPORT_DESC_BODY
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


static struct {
	bool isBusy;
	uint8_t rx[10];
} gHid;


/*
 * PUBLIC FUNCTIONS
 */

void USB_HID_Init(uint8_t config)
{
	gHid.isBusy = false;

	// Data endpoints
	USB_EP_Open(HID_IN_EP, USB_EP_TYPE_INTR, HID_PACKET_SIZE, USB_HID_TransmitDone);
	USB_EP_Open(HID_OUT_EP, USB_EP_TYPE_INTR, HID_PACKET_SIZE, USB_HID_Receive);

	USB_EP_Read(HID_OUT_EP, gHid.rx, sizeof(gHid.rx));
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

void USB_HID_Report(HID_Report_t * report)
{
	if (!gHid.isBusy)
	{
		gHid.isBusy = true;
		USB_EP_Write(HID_IN_EP, (uint8_t*)report, 3);
	}
}

/*
 * static uint8_t  USBD_HID_Setup(USBD_HandleTypeDef *pdev,
                               USBD_SetupReqTypedef *req)
{
  USBD_HID_HandleTypeDef *hhid = (USBD_HID_HandleTypeDef *) pdev->pClassData;
  uint16_t len = 0U;
  uint8_t *pbuf = NULL;
  uint16_t status_info = 0U;
  USBD_StatusTypeDef ret = USBD_OK;

  switch (req->bmRequest & USB_REQ_TYPE_MASK)
  {
    case USB_REQ_TYPE_CLASS :
      switch (req->bRequest)
      {
        case HID_REQ_SET_PROTOCOL:
          hhid->Protocol = (uint8_t)(req->wValue);
          break;

        case HID_REQ_GET_PROTOCOL:
          USBD_CtlSendData(pdev, (uint8_t *)(void *)&hhid->Protocol, 1U);
          break;

        case HID_REQ_SET_IDLE:
          hhid->IdleState = (uint8_t)(req->wValue >> 8);
          break;

        case HID_REQ_GET_IDLE:
          USBD_CtlSendData(pdev, (uint8_t *)(void *)&hhid->IdleState, 1U);
          break;

        default:
          USBD_CtlError(pdev, req);
          ret = USBD_FAIL;
          break;
      }
      break;
    case USB_REQ_TYPE_STANDARD:
      switch (req->bRequest)
      {
        case USB_REQ_GET_STATUS:
          if (pdev->dev_state == USBD_STATE_CONFIGURED)
          {
            USBD_CtlSendData(pdev, (uint8_t *)(void *)&status_info, 2U);
          }
          else
          {
            USBD_CtlError(pdev, req);
            ret = USBD_FAIL;
          }
          break;

        case USB_REQ_GET_DESCRIPTOR:
          if (req->wValue >> 8 == HID_REPORT_DESC)
          {
            len = MIN(HID_MOUSE_REPORT_DESC_SIZE, req->wLength);
            pbuf = HID_MOUSE_ReportDesc;
          }
          else if (req->wValue >> 8 == HID_DESCRIPTOR_TYPE)
          {
            pbuf = USBD_HID_Desc;
            len = MIN(USB_HID_DESC_SIZ, req->wLength);
          }
          else
          {
            USBD_CtlError(pdev, req);
            ret = USBD_FAIL;
            break;
          }
          USBD_CtlSendData(pdev, pbuf, len);
          break;

        case USB_REQ_GET_INTERFACE :
          if (pdev->dev_state == USBD_STATE_CONFIGURED)
          {
            USBD_CtlSendData(pdev, (uint8_t *)(void *)&hhid->AltSetting, 1U);
          }
          else
          {
            USBD_CtlError(pdev, req);
            ret = USBD_FAIL;
          }
          break;

        case USB_REQ_SET_INTERFACE :
          if (pdev->dev_state == USBD_STATE_CONFIGURED)
          {
            hhid->AltSetting = (uint8_t)(req->wValue);
          }
          else
          {
            USBD_CtlError(pdev, req);
            ret = USBD_FAIL;
          }
          break;

        default:
          USBD_CtlError(pdev, req);
          ret = USBD_FAIL;
          break;
      }
      break;

    default:
      USBD_CtlError(pdev, req);
      ret = USBD_FAIL;
      break;
  }

  return ret;
}
 */

/*
 * PRIVATE FUNCTIONS
 */

static void USB_HID_TransmitDone(uint32_t count)
{
	 gHid.isBusy = false;
}

static void USB_HID_Receive(uint32_t count)
{

	USB_EP_Read(HID_OUT_EP, gHid.rx, sizeof(gHid.rx));
}

static void USB_HID_GetDescriptor(USB_SetupRequest_t * req)
{
	uint8_t type = HIBYTE(req->wValue);
	//uint8_t index = LOBYTE(req->wValue);

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

