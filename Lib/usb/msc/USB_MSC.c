
#include "USB_MSC.h"

#ifdef USB_CLASS_MSC
#include "../USB_EP.h"
#include "../USB_CTL.h"
#include "Core.h"
#include <string.h>

#include "usbd_msc_scsi.h"

/*
 * PRIVATE DEFINITIONS
 */


#define MSC_PACKET_SIZE								USB_PACKET_SIZE

#define USBD_BOT_CBW_SIGNATURE             0x43425355U
#define USBD_BOT_CSW_SIGNATURE             0x53425355U
#define USBD_BOT_CBW_LENGTH                31U
#define USBD_BOT_CSW_LENGTH                13U
#define USBD_BOT_MAX_DATA                  256U


/* BOT Status */
#define USBD_BOT_STATUS_NORMAL             0U
#define USBD_BOT_STATUS_RECOVERY           1U
#define USBD_BOT_STATUS_ERROR              2U


#define USBD_DIR_IN                        0U
#define USBD_DIR_OUT                       1U
#define USBD_BOTH_DIR                      2U


#define MSC_LUN_COUNT						1
#define MSC_MAX_LUN							(MSC_LUN_COUNT-1)

#ifndef MSC_MEDIA_PACKET
#define MSC_MEDIA_PACKET             512U
#endif

#define MSC_MAX_FS_PACKET            0x40U
#define MSC_MAX_HS_PACKET            0x200U

#define BOT_GET_MAX_LUN              0xFE
#define BOT_RESET                    0xFF


/*
 * PRIVATE TYPES
 */

/*typedef struct
{

} MSC_t;*/

/*
 * PRIVATE PROTOTYPES
 */

void MSC_BOT_Init(void);
void MSC_BOT_Reset(void);
void MSC_BOT_DeInit(void);

void USB_MSC_TransmitDone(uint32_t size);
void USB_MSC_Receive(uint32_t size);
void MSC_BOT_CplClrFeature(uint8_t epnum);

static void MSC_BOT_CBW_Decode(uint32_t size);
static void MSC_BOT_SendData(uint8_t *pbuf, uint16_t len);
static void MSC_BOT_Abort(void);

/*
 * PRIVATE VARIABLES
 */

__ALIGNED(4) const uint8_t cUSB_MSC_ConfigDescriptor[USB_MSC_CONFIG_DESC_SIZE] =
{
	USB_DESCR_BLOCK_CONFIGURATION(
			USB_MSC_CONFIG_DESC_SIZE,
			0x01, // 1 interfaces available
			0x01
			),
	USB_DESCR_BLOCK_INTERFACE(
			0x00,
			0x02, // 2 endpoints used
			0x08, // Mass Storage Class
			0x06, // SCSI transparent
			0x50 // Unknown protocol
	),
	USB_DESCR_BLOCK_ENDPOINT( MSC_IN_EP, 0x02, MSC_PACKET_SIZE, 0x00 ),
	USB_DESCR_BLOCK_ENDPOINT( MSC_OUT_EP, 0x02, MSC_PACKET_SIZE, 0x00 ),
};

//static MSC_t gMSC;

static USBD_MSC_BOT_HandleTypeDef gHmsc;
static USBD_MSC_BOT_HandleTypeDef * const hmsc = &gHmsc;

static SCSI_t gScsi = { .pClassData = &gHmsc };

/*
 * PUBLIC FUNCTIONS
 */

void USB_MSC_SetStorage(USBD_StorageTypeDef * storage)
{
	gScsi.pUserData = storage;
}

void USB_MSC_Init(uint8_t config)
{
	// Data endpoints
	USB_EP_Open(MSC_IN_EP, USB_EP_TYPE_BULK, MSC_PACKET_SIZE, USB_MSC_TransmitDone);
	USB_EP_Open(MSC_OUT_EP, USB_EP_TYPE_BULK, MSC_PACKET_SIZE, USB_MSC_Receive);
	MSC_BOT_Init();
}

void USB_MSC_Deinit(void)
{
	MSC_BOT_DeInit();
	USB_EP_Close(MSC_IN_EP);
	USB_EP_Close(MSC_OUT_EP);
}

void USB_MSC_Setup(USB_SetupRequest_t * req)
{
	switch (req->bRequest)
	{
	case BOT_GET_MAX_LUN:
	  if (req->wValue == 0 && req->wLength == 1 && req->bmRequest & 0x80)
	  {
		  uint8_t max_lun = MSC_MAX_LUN;
		  USB_CTL_Send(&max_lun, sizeof(max_lun));
		  return;
	  }
	  break;
	case BOT_RESET :
	  if (req->wValue == 0U && req->wLength == 0 && !(req->bmRequest & 0x80U))
	  {
		  MSC_BOT_Reset();
		  return;
	  }
	  break;
	}
	// Error occurred. Probably shouldn't ignore?
}

/*
 * PRIVATE FUNCTIONS
 */


void MSC_BOT_Init(void)
{
  hmsc->bot_state = USBD_BOT_IDLE;
  hmsc->bot_status = USBD_BOT_STATUS_NORMAL;

  hmsc->scsi_sense_tail = 0U;
  hmsc->scsi_sense_head = 0U;

  USB_EP_Read(MSC_OUT_EP, (uint8_t *)&hmsc->cbw, USBD_BOT_CBW_LENGTH);
}

void MSC_BOT_Reset(void)
{
  hmsc->bot_state  = USBD_BOT_IDLE;
  hmsc->bot_status = USBD_BOT_STATUS_RECOVERY;

  USB_EP_Read(MSC_OUT_EP, (uint8_t *)&hmsc->cbw, USBD_BOT_CBW_LENGTH);
}

void MSC_BOT_DeInit(void)
{
  hmsc->bot_state = USBD_BOT_IDLE;
}

void USB_MSC_TransmitDone(uint32_t size)
{
  switch (hmsc->bot_state)
  {
    case USBD_BOT_DATA_IN:
      if (SCSI_ProcessCmd(&gScsi, hmsc->cbw.bLUN, &hmsc->cbw.CB[0]) < 0)
      {
        MSC_BOT_SendCSW(USBD_CSW_CMD_FAILED);
      }
      break;

    case USBD_BOT_SEND_DATA:
    case USBD_BOT_LAST_DATA_IN:
      MSC_BOT_SendCSW(USBD_CSW_CMD_PASSED);
      break;

    default:
      break;
  }
}


void USB_MSC_Receive(uint32_t size)
{
  switch (hmsc->bot_state)
  {
    case USBD_BOT_IDLE:
      MSC_BOT_CBW_Decode(size);
      break;

    case USBD_BOT_DATA_OUT:

      if (SCSI_ProcessCmd(&gScsi, hmsc->cbw.bLUN, &hmsc->cbw.CB[0]) < 0)
      {
        MSC_BOT_SendCSW(USBD_CSW_CMD_FAILED);
      }
      break;

    default:
      break;
  }
}

/**
* @brief  MSC_BOT_CBW_Decode
*         Decode the CBW command and set the BOT state machine accordingly
* @param  pdev: device instance
* @retval None
*/
static void  MSC_BOT_CBW_Decode(uint32_t size)
{
  hmsc->csw.dTag = hmsc->cbw.dTag;
  hmsc->csw.dDataResidue = hmsc->cbw.dDataLength;

  if ((size != USBD_BOT_CBW_LENGTH) ||
      (hmsc->cbw.dSignature != USBD_BOT_CBW_SIGNATURE) ||
      (hmsc->cbw.bLUN > 1U) ||
      (hmsc->cbw.bCBLength < 1U) || (hmsc->cbw.bCBLength > 16U))
  {

    SCSI_SenseCode(&gScsi, hmsc->cbw.bLUN, ILLEGAL_REQUEST, INVALID_CDB);

    hmsc->bot_status = USBD_BOT_STATUS_ERROR;
    MSC_BOT_Abort();
  }
  else
  {
    if (SCSI_ProcessCmd(&gScsi, hmsc->cbw.bLUN, &hmsc->cbw.CB[0]) < 0)
    {
      if (hmsc->bot_state == USBD_BOT_NO_DATA)
      {
        MSC_BOT_SendCSW(USBD_CSW_CMD_FAILED);
      }
      else
      {
        MSC_BOT_Abort();
      }
    }
    /*Burst xfer handled internally*/
    else if ((hmsc->bot_state != USBD_BOT_DATA_IN) &&
             (hmsc->bot_state != USBD_BOT_DATA_OUT) &&
             (hmsc->bot_state != USBD_BOT_LAST_DATA_IN))
    {
      if (hmsc->bot_data_length > 0U)
      {
        MSC_BOT_SendData(hmsc->bot_data, hmsc->bot_data_length);
      }
      else if (hmsc->bot_data_length == 0U)
      {
        MSC_BOT_SendCSW(USBD_CSW_CMD_PASSED);
      }
      else
      {
        MSC_BOT_Abort();
      }
    }
    else
    {
      return;
    }
  }
}

/**
* @brief  MSC_BOT_SendData
*         Send the requested data
* @param  pdev: device instance
* @param  buf: pointer to data buffer
* @param  len: Data Length
* @retval None
*/
static void  MSC_BOT_SendData(uint8_t *pbuf, uint16_t len)
{
  uint16_t length = (uint16_t)MIN(hmsc->cbw.dDataLength, len);

  hmsc->csw.dDataResidue -= len;
  hmsc->csw.bStatus = USBD_CSW_CMD_PASSED;
  hmsc->bot_state = USBD_BOT_SEND_DATA;

  USB_EP_Write( MSC_IN_EP, pbuf, length );
}

/**
* @brief  MSC_BOT_SendCSW
*         Send the Command Status Wrapper
* @param  pdev: device instance
* @param  status : CSW status
* @retval None
*/
void  MSC_BOT_SendCSW(uint8_t CSW_Status)
{
  hmsc->csw.dSignature = USBD_BOT_CSW_SIGNATURE;
  hmsc->csw.bStatus = CSW_Status;
  hmsc->bot_state = USBD_BOT_IDLE;

  USB_EP_Write(MSC_IN_EP, (uint8_t *)&hmsc->csw, USBD_BOT_CSW_LENGTH);
  /* Prepare EP to Receive next Cmd */
  USB_EP_Read(MSC_OUT_EP, (uint8_t *)&hmsc->cbw, USBD_BOT_CBW_LENGTH);
}

/**
* @brief  MSC_BOT_Abort
*         Abort the current transfer
* @param  pdev: device instance
* @retval status
*/
static void  MSC_BOT_Abort(void)
{
  if ((hmsc->cbw.bmFlags == 0U) &&
      (hmsc->cbw.dDataLength != 0U) &&
      (hmsc->bot_status == USBD_BOT_STATUS_NORMAL))
  {
	  USB_EP_Stall(MSC_OUT_EP);
  }

  USB_EP_Stall(MSC_IN_EP);

  if (hmsc->bot_status == USBD_BOT_STATUS_ERROR)
  {
    USB_EP_Read(MSC_OUT_EP, (uint8_t *)&hmsc->cbw, USBD_BOT_CBW_LENGTH);
  }
}

/**
* @brief  MSC_BOT_CplClrFeature
*         Complete the clear feature request
* @param  pdev: device instance
* @param  epnum: endpoint index
* @retval None
*/

void  MSC_BOT_CplClrFeature(uint8_t epnum)
{
  if (hmsc->bot_status == USBD_BOT_STATUS_ERROR) /* Bad CBW Signature */
  {
	  USB_EP_Stall(MSC_IN_EP);
	  hmsc->bot_status = USBD_BOT_STATUS_NORMAL;
  }
  else if (((epnum & 0x80U) == 0x80U) && (hmsc->bot_status != USBD_BOT_STATUS_RECOVERY))
  {
	  MSC_BOT_SendCSW(USBD_CSW_CMD_FAILED);
  }
  else
  {
    return;
  }
}


#endif //USB_CLASS_MSC

