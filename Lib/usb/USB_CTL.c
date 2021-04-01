
#include "USB_CTL.h"
#include "USB_EP.h"
#include "USB_PCD.h"

#include "USB_Class.h"

/*
 * PRIVATE DEFINITIONS
 */

#define CTL_IN_EP		0x80
#define CTL_OUT_EP		0x00

#define CTL_EP_SIZE		USB_MAX_EP0_SIZE

/*
 * PRIVATE TYPES
 */

/*
 * PUBLIC TYPES
 */

/*
 * PRIVATE PROTOTYPES
 */

static void USB_CTL_EndpointRequest(USBD_SetupReqTypedef  *req);
static void USB_CTL_DeviceRequest(USBD_SetupReqTypedef *req);
static void USB_CTL_InterfaceRequest(USBD_SetupReqTypedef  *req);

//static void USB_CTL_Error(void);
static void USB_CTL_SetFeature(USBD_SetupReqTypedef *req);
static void USB_CTL_ClearFeature(USBD_SetupReqTypedef *req);

static void USB_CTL_SendStatus(void);
//static void USB_CTL_Send(uint8_t * data, uint16_t size);
static void USB_CTL_ReceiveStatus(void);
//static void USB_CTL_Receive(uint8_t * data, uint16_t size);

static void USBD_SetAddress(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req);
static void USBD_SetConfig(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req);
static void USBD_GetConfig(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req);
static void USBD_GetStatus(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req);
void USBD_GetString(uint8_t *desc, uint8_t *unicode, uint16_t *len);
static void USBD_GetDescriptor(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req);

/*
 * PRIVATE VARIABLES
 */

/*
 * PUBLIC FUNCTIONS
 */


void USB_CTL_Init(void)
{
	USB_EP_Open(CTL_IN_EP, USB_EP_TYPE_CTRL, CTL_EP_SIZE);
	USB_EP_Open(CTL_OUT_EP, USB_EP_TYPE_CTRL, CTL_EP_SIZE);
}

void USB_CTL_Deinit(void)
{
	// Dont bother closing the CTL EP's
}

void USB_CTL_Reset(void)
{
	// Clear existing endpoint layouts.
	USB_CLASS_DEINIT();
	USB_EP_Reset();

	USB_EP_Open(CTL_IN_EP, USB_EP_TYPE_CTRL, CTL_EP_SIZE);
	USB_EP_Open(CTL_OUT_EP, USB_EP_TYPE_CTRL, CTL_EP_SIZE);

	hUsbDeviceFS.dev_speed = USBD_SPEED_FULL;
	hUsbDeviceFS.dev_state = USBD_STATE_DEFAULT;
	hUsbDeviceFS.ep0_state = USBD_EP0_IDLE;
	hUsbDeviceFS.dev_config = 0U;
	hUsbDeviceFS.dev_remote_wakeup = 0U;

	USB_PCD_SetAddress(0U);
}

void USB_CTL_HandleSetup(uint8_t * data)
{
	USBD_SetupReqTypedef req;
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

static void USB_CTL_EndpointRequest(USBD_SetupReqTypedef  *req)
{
	uint8_t endpoint = LOBYTE(req->wIndex);
	USBD_HandleTypeDef * pdev = &hUsbDeviceFS;

	switch (req->bmRequest & USB_REQ_TYPE_MASK)
	{
	case USB_REQ_TYPE_CLASS:
	case USB_REQ_TYPE_VENDOR:
		USB_CLASS_SETUP(req);
		break;
	case USB_REQ_TYPE_STANDARD:
		if ((req->bmRequest & 0x60U) == 0x20U)
		{
			// This also indicates a class request?
			USB_CLASS_SETUP(req);
			return;
		}

		switch (req->bRequest)
		{
		case USB_REQ_SET_FEATURE:
			switch (pdev->dev_state)
			{
			case USBD_STATE_ADDRESSED:
				if ((endpoint != CTL_OUT_EP) && (endpoint != CTL_IN_EP))
				{
					USB_EP_Stall(endpoint);
					USB_EP_Stall(CTL_IN_EP);
				}
				else
				{
					USB_CTL_Error();
				}
				break;
			case USBD_STATE_CONFIGURED:
				if (req->wValue == USB_FEATURE_EP_HALT)
				{
					if ((endpoint != CTL_OUT_EP) && (endpoint != CTL_IN_EP) && (req->wLength == 0x00U))
					{
						USB_EP_Stall(endpoint);
					}
				}
				USB_CTL_SendStatus();
				break;
			default:
				USB_CTL_Error();
				break;
			}
			break;

			case USB_REQ_CLEAR_FEATURE:
				switch (pdev->dev_state)
				{
				case USBD_STATE_ADDRESSED:
					if ((endpoint & 0x7FU) != 0x00U)
					{
						USB_EP_Stall(endpoint);
						USB_EP_Stall(CTL_IN_EP);
					}
					else
					{
						USB_CTL_Error();
					}
					break;

				case USBD_STATE_CONFIGURED:
					if (req->wValue == USB_FEATURE_EP_HALT)
					{
						if ((endpoint & 0x7FU) != 0x00U)
						{
							USB_EP_Destall(endpoint);
						}
						USB_CTL_SendStatus();
					}
					break;

				default:
					USB_CTL_Error();
					break;
				}
				break;

			case USB_REQ_GET_STATUS:
				switch (pdev->dev_state)
				{
				case USBD_STATE_ADDRESSED:
				case USBD_STATE_CONFIGURED:
					if (!USB_EP_IsOpen(endpoint))
					{
						USB_CTL_Error();
						break;
					}
					else
					{
						uint16_t status = USB_EP_IsStalled(endpoint) ? 0x0001 : 0x0000;
						USB_CTL_Send((uint8_t *)&status, 2);
					}
					break;
				default:
					USB_CTL_Error();
					break;
				}
				break;
			default:
				USB_CTL_Error();
				break;
		}
		break;
	default:
		USB_CTL_Error();
		break;
	}
}



static void USB_CTL_DeviceRequest(USBD_SetupReqTypedef *req)
{
  USBD_HandleTypeDef * pdev = &hUsbDeviceFS;

  switch (req->bmRequest & USB_REQ_TYPE_MASK)
  {
    case USB_REQ_TYPE_CLASS:
    case USB_REQ_TYPE_VENDOR:
      pdev->pClass->Setup(pdev, req);
      break;

    case USB_REQ_TYPE_STANDARD:
      switch (req->bRequest)
      {
        case USB_REQ_GET_DESCRIPTOR:
          USBD_GetDescriptor(pdev, req);
          break;

        case USB_REQ_SET_ADDRESS:
          USBD_SetAddress(pdev, req);
          break;

        case USB_REQ_SET_CONFIGURATION:
          USBD_SetConfig(pdev, req);
          break;

        case USB_REQ_GET_CONFIGURATION:
          USBD_GetConfig(pdev, req);
          break;

        case USB_REQ_GET_STATUS:
          USBD_GetStatus(pdev, req);
          break;

        case USB_REQ_SET_FEATURE:
        	USB_CTL_SetFeature(req);
        	break;

        case USB_REQ_CLEAR_FEATURE:
        	USB_CTL_ClearFeature(req);
        	break;

        default:
          USB_CTL_Error();
          break;
      }
      break;

    default:
      USB_CTL_Error();
      break;
  }
}

static void USB_CTL_InterfaceRequest(USBD_SetupReqTypedef  *req)
{
	bool success = false;
	switch (req->bmRequest & USB_REQ_TYPE_MASK)
	{
	case USB_REQ_TYPE_CLASS:
	case USB_REQ_TYPE_VENDOR:
	case USB_REQ_TYPE_STANDARD:
		switch (hUsbDeviceFS.dev_state)
		{
		case USBD_STATE_DEFAULT:
		case USBD_STATE_ADDRESSED:
		case USBD_STATE_CONFIGURED:
			if (LOBYTE(req->wIndex) <= USBD_MAX_NUM_INTERFACES)
			{
				USB_CLASS_SETUP(req);
				success = true;
				if ((req->wLength == 0U))
				{
					USB_CTL_SendStatus();
				}
			}
			break;
		}
		break;
	}
	if (!success)
	{
		USB_CTL_Error();
	}
}

void USB_CTL_Error(void)
{
	USB_EP_Stall(CTL_IN_EP);
	USB_EP_Stall(CTL_OUT_EP);
}

static void USB_CTL_SetFeature(USBD_SetupReqTypedef *req)
{
	if (req->wValue == USB_FEATURE_REMOTE_WAKEUP)
	{
		hUsbDeviceFS.dev_remote_wakeup = 1U;
		USB_CTL_SendStatus();
	}
}

static void USB_CTL_ClearFeature(USBD_SetupReqTypedef *req)
{
	USBD_HandleTypeDef * pdev = &hUsbDeviceFS;
	switch (hUsbDeviceFS.dev_state)
	{
	case USBD_STATE_DEFAULT:
	case USBD_STATE_ADDRESSED:
	case USBD_STATE_CONFIGURED:
		if (req->wValue == USB_FEATURE_REMOTE_WAKEUP)
		{
			pdev->dev_remote_wakeup = 0U;
			USB_CTL_SendStatus();
		}
		break;

	default:
		USB_CTL_Error();
		break;
	}
}

/*
 * INTERRUPT ROUTINES
 */

// DELETE THIS

static void USBD_GetDescriptor(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
  uint16_t len = 0U;
  uint8_t *pbuf = NULL;
  uint8_t err = 0U;

  switch (req->wValue >> 8)
  {
#if (USBD_LPM_ENABLED == 1U)
    case USB_DESC_TYPE_BOS:
      if (pdev->pDesc->GetBOSDescriptor != NULL)
      {
        pbuf = pdev->pDesc->GetBOSDescriptor(pdev->dev_speed, &len);
      }
      else
      {
        USBD_CtlError(pdev, req);
        err++;
      }
      break;
#endif
    case USB_DESC_TYPE_DEVICE:
      pbuf = pdev->pDesc->GetDeviceDescriptor(pdev->dev_speed, &len);
      break;

    case USB_DESC_TYPE_CONFIGURATION:
      if (pdev->dev_speed == USBD_SPEED_HIGH)
      {
        pbuf = pdev->pClass->GetHSConfigDescriptor(&len);
        pbuf[1] = USB_DESC_TYPE_CONFIGURATION;
      }
      else
      {
        pbuf = pdev->pClass->GetFSConfigDescriptor(&len);
        pbuf[1] = USB_DESC_TYPE_CONFIGURATION;
      }
      break;

    case USB_DESC_TYPE_STRING:
      switch ((uint8_t)(req->wValue))
      {
        case USBD_IDX_LANGID_STR:
          if (pdev->pDesc->GetLangIDStrDescriptor != NULL)
          {
            pbuf = pdev->pDesc->GetLangIDStrDescriptor(pdev->dev_speed, &len);
          }
          else
          {
        	  USB_CTL_Error();
            err++;
          }
          break;

        case USBD_IDX_MFC_STR:
          if (pdev->pDesc->GetManufacturerStrDescriptor != NULL)
          {
            pbuf = pdev->pDesc->GetManufacturerStrDescriptor(pdev->dev_speed, &len);
          }
          else
          {
        	  USB_CTL_Error();
            err++;
          }
          break;

        case USBD_IDX_PRODUCT_STR:
          if (pdev->pDesc->GetProductStrDescriptor != NULL)
          {
            pbuf = pdev->pDesc->GetProductStrDescriptor(pdev->dev_speed, &len);
          }
          else
          {
        	  USB_CTL_Error();
            err++;
          }
          break;

        case USBD_IDX_SERIAL_STR:
          if (pdev->pDesc->GetSerialStrDescriptor != NULL)
          {
            pbuf = pdev->pDesc->GetSerialStrDescriptor(pdev->dev_speed, &len);
          }
          else
          {
        	  USB_CTL_Error();
            err++;
          }
          break;

        case USBD_IDX_CONFIG_STR:
          if (pdev->pDesc->GetConfigurationStrDescriptor != NULL)
          {
            pbuf = pdev->pDesc->GetConfigurationStrDescriptor(pdev->dev_speed, &len);
          }
          else
          {
        	  USB_CTL_Error();
            err++;
          }
          break;

        case USBD_IDX_INTERFACE_STR:
          if (pdev->pDesc->GetInterfaceStrDescriptor != NULL)
          {
            pbuf = pdev->pDesc->GetInterfaceStrDescriptor(pdev->dev_speed, &len);
          }
          else
          {
        	  USB_CTL_Error();
            err++;
          }
          break;

        default:
#if (USBD_SUPPORT_USER_STRING_DESC == 1U)
          if (pdev->pClass->GetUsrStrDescriptor != NULL)
          {
            pbuf = pdev->pClass->GetUsrStrDescriptor(pdev, (req->wValue), &len);
          }
          else
          {
        	  USB_CTL_Error();
            err++;
          }
          break;
#else
          USB_CTL_Error();
          err++;
#endif
      }
      break;

    case USB_DESC_TYPE_DEVICE_QUALIFIER:
      if (pdev->dev_speed == USBD_SPEED_HIGH)
      {
        pbuf = pdev->pClass->GetDeviceQualifierDescriptor(&len);
      }
      else
      {
    	  USB_CTL_Error();
        err++;
      }
      break;

    case USB_DESC_TYPE_OTHER_SPEED_CONFIGURATION:
      if (pdev->dev_speed == USBD_SPEED_HIGH)
      {
        pbuf = pdev->pClass->GetOtherSpeedConfigDescriptor(&len);
        pbuf[1] = USB_DESC_TYPE_OTHER_SPEED_CONFIGURATION;
      }
      else
      {
    	  USB_CTL_Error();
        err++;
      }
      break;

    default:
    	USB_CTL_Error();
      err++;
      break;
  }

  if (err != 0U)
  {
    return;
  }
  else
  {
    if ((len != 0U) && (req->wLength != 0U))
    {
      len = MIN(len, req->wLength);
      USB_CTL_Send(pbuf, len);
    }

    if (req->wLength == 0U)
    {
      USB_CTL_SendStatus();
    }
  }
}

static void USBD_SetAddress(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
  uint8_t  dev_addr;

  if ((req->wIndex == 0U) && (req->wLength == 0U) && (req->wValue < 128U))
  {
    dev_addr = (uint8_t)(req->wValue) & 0x7FU;

    if (pdev->dev_state == USBD_STATE_CONFIGURED)
    {
    	USB_CTL_Error();
    }
    else
    {
      pdev->dev_address = dev_addr;
      USB_PCD_SetAddress(dev_addr);
      USB_CTL_SendStatus();

      if (dev_addr != 0U)
      {
        pdev->dev_state = USBD_STATE_ADDRESSED;
      }
      else
      {
        pdev->dev_state = USBD_STATE_DEFAULT;
      }
    }
  }
  else
  {
	  USB_CTL_Error();
  }
}

static void USBD_SetConfig(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
  static uint8_t cfgidx;

  cfgidx = (uint8_t)(req->wValue);

  if (cfgidx > USBD_MAX_NUM_CONFIGURATION)
  {
	  USB_CTL_Error();
  }
  else
  {
    switch (pdev->dev_state)
    {
      case USBD_STATE_ADDRESSED:
        if (cfgidx)
        {
          pdev->dev_config = cfgidx;
          pdev->dev_state = USBD_STATE_CONFIGURED;
          USB_CLASS_INIT(cfgidx);
          USB_CTL_SendStatus();
        }
        else
        {
        	USB_CTL_SendStatus();
        }
        break;

      case USBD_STATE_CONFIGURED:
        if (cfgidx == 0U)
        {
          pdev->dev_state = USBD_STATE_ADDRESSED;
          pdev->dev_config = cfgidx;
          USB_CLASS_DEINIT();
          USB_CTL_SendStatus();
        }
        else if (cfgidx != pdev->dev_config)
        {
          USB_CLASS_DEINIT();

          /* set new configuration */
          pdev->dev_config = cfgidx;
          USB_CLASS_INIT(cfgidx);
          USB_CTL_SendStatus();
        }
        else
        {
        	USB_CTL_SendStatus();
        }
        break;

      default:
    	  USB_CTL_Error();
    	  USB_CLASS_DEINIT();
        break;
    }
  }
}

static void USBD_GetConfig(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
  if (req->wLength != 1U)
  {
	  USB_CTL_Error();
  }
  else
  {
    switch (pdev->dev_state)
    {
      case USBD_STATE_DEFAULT:
      case USBD_STATE_ADDRESSED:
        pdev->dev_default_config = 0U;
        USB_CTL_Send((uint8_t *)(void *)&pdev->dev_default_config, 1U);
        break;

      case USBD_STATE_CONFIGURED:
        USB_CTL_Send((uint8_t *)(void *)&pdev->dev_config, 1U);
        break;

      default:
    	  USB_CTL_Error();
        break;
    }
  }
}

static void USBD_GetStatus(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
  switch (pdev->dev_state)
  {
    case USBD_STATE_DEFAULT:
    case USBD_STATE_ADDRESSED:
    case USBD_STATE_CONFIGURED:
      if (req->wLength != 0x2U)
      {
    	  USB_CTL_Error();
        break;
      }

#if (USBD_SELF_POWERED == 1U)
      pdev->dev_config_status = USB_CONFIG_SELF_POWERED;
#else
      pdev->dev_config_status = 0U;
#endif

      if (pdev->dev_remote_wakeup)
      {
        pdev->dev_config_status |= USB_CONFIG_REMOTE_WAKEUP;
      }

      USB_CTL_Send((uint8_t *)(void *)&pdev->dev_config_status, 2U);
      break;

    default:
    	USB_CTL_Error();
      break;
  }
}

uint16_t USB_CTL_ToUnicode(uint8_t * unicode, char * str)
{
	uint16_t i = 2;
	while (*str != 0)
	{
		unicode[i++] = *str++;
		unicode[i++] = 0;
	}
	unicode[0] = i;
	unicode[1] = USB_DESC_TYPE_STRING;
	return i;
}

void USB_CTL_DataOutStage(uint8_t epnum, uint8_t *pdata)
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
            (pdev->dev_state == USBD_STATE_CONFIGURED))
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
           (pdev->dev_state == USBD_STATE_CONFIGURED))
  {
    pdev->pClass->DataOut(pdev, epnum);
  }
}

void USB_CTL_DataInStage(uint8_t epnum, uint8_t *pdata)
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
              (pdev->dev_state == USBD_STATE_CONFIGURED))
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
  }
  else if ((pdev->pClass->DataIn != NULL) &&
           (pdev->dev_state == USBD_STATE_CONFIGURED))
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

