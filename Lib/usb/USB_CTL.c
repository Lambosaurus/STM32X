
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

//static void USB_CTL_SendStatus(void);


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
	USB_EP_Open(CTL_IN_EP, USBD_EP_TYPE_CTRL, CTL_EP_SIZE);
	USB_EP_Open(CTL_OUT_EP, USBD_EP_TYPE_CTRL, CTL_EP_SIZE);
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

	USB_EP_Open(CTL_IN_EP, USBD_EP_TYPE_CTRL, CTL_EP_SIZE);
	USB_EP_Open(CTL_OUT_EP, USBD_EP_TYPE_CTRL, CTL_EP_SIZE);

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
		/* Check if it is a class request */
		if ((req->bmRequest & 0x60U) == 0x20U)
		{
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
					if ((endpoint != CTL_OUT_EP) &&
						(endpoint != CTL_IN_EP) && (req->wLength == 0x00U))
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
			{
				if ((endpoint != CTL_OUT_EP) && (endpoint != CTL_IN_EP))
				{
				  USB_CTL_Error();
				  break;
				}
				USBD_EndpointTypeDef * pep = ((endpoint & 0x80U) == 0x80U) ? &pdev->ep_in[endpoint & 0x7FU] : \
					&pdev->ep_out[endpoint & 0x7FU];

				pep->status = 0x0000U;

				USBD_CtlSendData(pdev, (uint8_t *)(void *)&pep->status, 2U);
				break;
			}
			case USBD_STATE_CONFIGURED:
			{
				if ((endpoint & 0x80U) == 0x80U)
				{
					if (pdev->ep_in[endpoint & 0xFU].is_used == 0U)
					{
						USB_CTL_Error();
					  break;
					}
				}
				else
				{
					if (pdev->ep_out[endpoint & 0xFU].is_used == 0U)
					{
						USB_CTL_Error();
						break;
					}
				}

				USBD_EndpointTypeDef * pep = ((endpoint & 0x80U) == 0x80U) ? &pdev->ep_in[endpoint & 0x7FU] : \
					&pdev->ep_out[endpoint & 0x7FU];

				if ((endpoint == 0x00U) || (endpoint == 0x80U))
				{
					pep->status = 0x0000U;
				}
				else if (USB_EP_IsStalled(endpoint))
				{
					pep->status = 0x0001U;
				}
				else
				{
					pep->status = 0x0000U;
				}

				USBD_CtlSendData(pdev, (uint8_t *)(void *)&pep->status, 2U);
				break;
			}
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
	USBD_HandleTypeDef * pdev = &hUsbDeviceFS;
  switch (req->bmRequest & USB_REQ_TYPE_MASK)
  {
    case USB_REQ_TYPE_CLASS:
    case USB_REQ_TYPE_VENDOR:
    case USB_REQ_TYPE_STANDARD:
      switch (pdev->dev_state)
      {
        case USBD_STATE_DEFAULT:
        case USBD_STATE_ADDRESSED:
        case USBD_STATE_CONFIGURED:

          if (LOBYTE(req->wIndex) <= USBD_MAX_NUM_INTERFACES)
          {
            USB_CLASS_SETUP(req);

            if ((req->wLength == 0U))
            {
            	USB_CTL_SendStatus();
            }
          }
          else
          {
        	  USB_CTL_Error();
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

void USB_CTL_Error(void)
{
	USB_EP_Stall(CTL_IN_EP);
	USB_EP_Stall(CTL_OUT_EP);
}

static void USB_CTL_SetFeature(USBD_SetupReqTypedef *req)
{
	USBD_HandleTypeDef * pdev = &hUsbDeviceFS;
	if (req->wValue == USB_FEATURE_REMOTE_WAKEUP)
	{
		pdev->dev_remote_wakeup = 1U;
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
      (void)USBD_CtlSendData(pdev, pbuf, len);
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
          if (USBD_SetClassConfig(pdev, cfgidx) == USBD_FAIL)
          {
        	  USB_CTL_Error();
            return;
          }
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
          USBD_ClrClassConfig(pdev, cfgidx);
          USB_CTL_SendStatus();
        }
        else if (cfgidx != pdev->dev_config)
        {
          /* Clear old configuration */
          USBD_ClrClassConfig(pdev, (uint8_t)pdev->dev_config);

          /* set new configuration */
          pdev->dev_config = cfgidx;
          if (USBD_SetClassConfig(pdev, cfgidx) == USBD_FAIL)
          {
        	  USB_CTL_Error();
            return;
          }
          USB_CTL_SendStatus();
        }
        else
        {
        	USB_CTL_SendStatus();
        }
        break;

      default:
    	  USB_CTL_Error();
        USBD_ClrClassConfig(pdev, cfgidx);
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
        USBD_CtlSendData(pdev, (uint8_t *)(void *)&pdev->dev_default_config, 1U);
        break;

      case USBD_STATE_CONFIGURED:
        USBD_CtlSendData(pdev, (uint8_t *)(void *)&pdev->dev_config, 1U);
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

      USBD_CtlSendData(pdev, (uint8_t *)(void *)&pdev->dev_config_status, 2U);
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

void USB_CTL_SendStatus(void)
{
	hUsbDeviceFS.ep0_state = USBD_EP0_STATUS_IN;
	USB_EP_Tx(0x00U, NULL, 0U);
}


USBD_StatusTypeDef USBD_SetClassConfig(USBD_HandleTypeDef  *pdev, uint8_t cfgidx)
{
  USBD_StatusTypeDef ret = USBD_FAIL;

  if (pdev->pClass != NULL)
  {
    /* Set configuration  and Start the Class*/
    if (pdev->pClass->Init(pdev, cfgidx) == 0U)
    {
      ret = USBD_OK;
    }
  }

  return ret;
}

USBD_StatusTypeDef USBD_ClrClassConfig(USBD_HandleTypeDef  *pdev, uint8_t cfgidx)
{
  /* Clear configuration  and De-initialize the Class process*/
  pdev->pClass->DeInit(pdev, cfgidx);

  return USBD_OK;
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

        USBD_CtlContinueRx(pdev, pdata,
                           (uint16_t)MIN(pep->rem_length, pep->maxpacket));
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
  else
  {
    /* should never be in this condition */
    return USBD_FAIL;
  }

  return USBD_OK;
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

        USBD_CtlContinueSendData(pdev, pdata, (uint16_t)pep->rem_length);

        USB_EP_Rx(CTL_OUT_EP, NULL, 0);
      }
      else
      {
        /* last packet is MPS multiple, so send ZLP packet */
        if ((pep->total_length % pep->maxpacket == 0U) &&
            (pep->total_length >= pep->maxpacket) &&
            (pep->total_length < pdev->ep0_data_len))
        {
          USBD_CtlContinueSendData(pdev, NULL, 0U);
          pdev->ep0_data_len = 0U;

          USB_EP_Rx(CTL_OUT_EP, NULL, 0);
        }
        else
        {
          if ((pdev->pClass->EP0_TxSent != NULL) &&
              (pdev->dev_state == USBD_STATE_CONFIGURED))
          {
            pdev->pClass->EP0_TxSent(pdev);
          }
          USB_EP_Stall(CTL_IN_EP);
          USBD_CtlReceiveStatus(pdev);
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
  else
  {
    /* should never be in this condition */
    return USBD_FAIL;
  }

  return USBD_OK;
}

USBD_StatusTypeDef USBD_LL_Suspend(USBD_HandleTypeDef *pdev)
{
  pdev->dev_old_state =  pdev->dev_state;
  pdev->dev_state  = USBD_STATE_SUSPENDED;

  return USBD_OK;
}

USBD_StatusTypeDef USBD_LL_Resume(USBD_HandleTypeDef *pdev)
{
  if (pdev->dev_state == USBD_STATE_SUSPENDED)
  {
    pdev->dev_state = pdev->dev_old_state;
  }

  return USBD_OK;
}

USBD_StatusTypeDef USBD_LL_SOF(USBD_HandleTypeDef *pdev)
{
  if (pdev->dev_state == USBD_STATE_CONFIGURED)
  {
    if (pdev->pClass->SOF != NULL)
    {
      pdev->pClass->SOF(pdev);
    }
  }

  return USBD_OK;
}

USBD_StatusTypeDef USBD_LL_IsoINIncomplete(USBD_HandleTypeDef *pdev,
                                           uint8_t epnum)
{
  /* Prevent unused arguments compilation warning */
  UNUSED(pdev);
  UNUSED(epnum);

  return USBD_OK;
}

USBD_StatusTypeDef USBD_LL_IsoOUTIncomplete(USBD_HandleTypeDef *pdev,
                                            uint8_t epnum)
{
  /* Prevent unused arguments compilation warning */
  UNUSED(pdev);
  UNUSED(epnum);

  return USBD_OK;
}

USBD_StatusTypeDef USBD_LL_DevConnected(USBD_HandleTypeDef *pdev)
{
  /* Prevent unused argument compilation warning */
  UNUSED(pdev);

  return USBD_OK;
}

USBD_StatusTypeDef USBD_LL_DevDisconnected(USBD_HandleTypeDef *pdev)
{
  /* Free Class Resources */
  pdev->dev_state = USBD_STATE_DEFAULT;
  pdev->pClass->DeInit(pdev, (uint8_t)pdev->dev_config);

  return USBD_OK;
}

/**
* @brief  USBD_CtlSendData
*         send data on the ctl pipe
* @param  pdev: device instance
* @param  buff: pointer to data buffer
* @param  len: length of data to be sent
* @retval status
*/
USBD_StatusTypeDef USBD_CtlSendData(USBD_HandleTypeDef *pdev, uint8_t *pbuf, uint16_t len)
{
  /* Set EP0 State */
  pdev->ep0_state = USBD_EP0_DATA_IN;
  pdev->ep_in[0].total_length = len;
  pdev->ep_in[0].rem_length   = len;

  /* Start the transfer */
  USB_EP_Tx(CTL_IN_EP, pbuf, len);

  return USBD_OK;
}

/**
* @brief  USBD_CtlContinueSendData
*         continue sending data on the ctl pipe
* @param  pdev: device instance
* @param  buff: pointer to data buffer
* @param  len: length of data to be sent
* @retval status
*/
USBD_StatusTypeDef USBD_CtlContinueSendData(USBD_HandleTypeDef *pdev,
                                            uint8_t *pbuf, uint16_t len)
{
  USB_EP_Tx(CTL_IN_EP, pbuf, len);

  return USBD_OK;
}

/**
* @brief  USBD_CtlPrepareRx
*         receive data on the ctl pipe
* @param  pdev: device instance
* @param  buff: pointer to data buffer
* @param  len: length of data to be received
* @retval status
*/
USBD_StatusTypeDef USBD_CtlPrepareRx(USBD_HandleTypeDef *pdev,
                                     uint8_t *pbuf, uint16_t len)
{
  /* Set EP0 State */
  pdev->ep0_state = USBD_EP0_DATA_OUT;
  pdev->ep_out[0].total_length = len;
  pdev->ep_out[0].rem_length   = len;

  USB_EP_Rx(CTL_OUT_EP, pbuf, len);

  return USBD_OK;
}

/**
* @brief  USBD_CtlContinueRx
*         continue receive data on the ctl pipe
* @param  pdev: device instance
* @param  buff: pointer to data buffer
* @param  len: length of data to be received
* @retval status
*/
USBD_StatusTypeDef USBD_CtlContinueRx(USBD_HandleTypeDef *pdev,
                                      uint8_t *pbuf, uint16_t len)
{
  USB_EP_Rx(CTL_OUT_EP, pbuf, len);

  return USBD_OK;
}

/**
* @brief  USBD_CtlReceiveStatus
*         receive zero lzngth packet on the ctl pipe
* @param  pdev: device instance
* @retval status
*/
USBD_StatusTypeDef USBD_CtlReceiveStatus(USBD_HandleTypeDef *pdev)
{
  /* Set EP0 State */
  pdev->ep0_state = USBD_EP0_STATUS_OUT;
  USB_EP_Rx(CTL_OUT_EP, NULL, 0);
  return USBD_OK;
}
