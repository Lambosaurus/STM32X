
#include "USB_MSC.h"

#ifdef USB_CLASS_MSC
#include "../USB_EP.h"
#include "../USB_CTL.h"
#include "Core.h"
#include <string.h>

#include "SCSI.h"

/*
 * PRIVATE DEFINITIONS
 */


#define MSC_PACKET_SIZE						USB_PACKET_SIZE

#define MSC_CBW_SIGNATURE             		0x43425355U
#define MSC_CSW_SIGNATURE             		0x53425355U
#define MSC_CBW_LENGTH                		31U
#define MSC_CSW_LENGTH                		13U
#define MSC_MAX_DATA                  		256U

#define MSC_STATUS_NORMAL             		0
#define MSC_STATUS_RECOVERY           		1
#define MSC_STATUS_ERROR              		2

#define MSC_LUN_COUNT						1
#define MSC_MAX_LUN							(MSC_LUN_COUNT-1)

#define MSC_REQ_GET_MAX_LUN              	0xFE
#define MSC_REQ_RESET                    	0xFF


/*
 * PRIVATE TYPES
 */

/*typedef struct
{

} MSC_t;*/

/*
 * PRIVATE PROTOTYPES
 */

static void USB_MSC_Reset(void);
void USB_MSC_TransmitDone(uint32_t size);
void USB_MSC_Receive(uint32_t size);
void MSC_BOT_CplClrFeature(uint8_t epnum);

static void MSC_BOT_CBW_Decode(uint32_t size);
static void MSC_BOT_SendData(uint8_t *pbuf, uint16_t len);
static void MSC_BOT_Abort(void);

static void USB_MSC_SCSI_Reply(SCSI_State_t state);

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


static struct {
	SCSI_t scsi;
	uint8_t status;
	int8_t state;
} gMSC =
{
	.scsi = {
		.pClassData = &gHmsc,
	},
};

/*
 * PUBLIC FUNCTIONS
 */

void USB_MSC_SetStorage(USB_MSC_Storage_t * storage)
{
	gMSC.scsi.storage = storage;
}

void USB_MSC_Init(uint8_t config)
{
	// Data endpoints
	USB_EP_Open(MSC_IN_EP, USB_EP_TYPE_BULK, MSC_PACKET_SIZE, USB_MSC_TransmitDone);
	USB_EP_Open(MSC_OUT_EP, USB_EP_TYPE_BULK, MSC_PACKET_SIZE, USB_MSC_Receive);

	gMSC.state = SCSI_Init(&gMSC.scsi, gMSC.scsi.storage);
	gMSC.status = MSC_STATUS_NORMAL;

	USB_EP_Read(MSC_OUT_EP, (uint8_t *)&hmsc->cbw, MSC_CBW_LENGTH);
}

void USB_MSC_Deinit(void)
{
	USB_EP_Close(MSC_IN_EP);
	USB_EP_Close(MSC_OUT_EP);
}

void USB_MSC_Setup(USB_SetupRequest_t * req)
{
	switch (req->bRequest)
	{
	case MSC_REQ_GET_MAX_LUN:
	  if (req->wValue == 0 && req->wLength == 1 && req->bmRequest & 0x80)
	  {
		  uint8_t max_lun = MSC_MAX_LUN;
		  USB_CTL_Send(&max_lun, sizeof(max_lun));
		  return;
	  }
	  break;
	case MSC_REQ_RESET:
	  if (req->wValue == 0U && req->wLength == 0 && !(req->bmRequest & 0x80U))
	  {
		  USB_MSC_Reset();
		  return;
	  }
	  break;
	}
	// Error occurred. Probably shouldn't ignore?
}

/*
 * PRIVATE FUNCTIONS
 */

static inline void Assert(bool statement)
{
	if (!statement)
	{
		__BKPT();
	}
}


static void USB_MSC_Reset(void)
{
	gMSC.status = MSC_STATUS_RECOVERY;
	USB_EP_Read(MSC_OUT_EP, (uint8_t *)&hmsc->cbw, MSC_CBW_LENGTH);
}

void USB_MSC_TransmitDone(uint32_t size)
{
	switch (gMSC.state)
	{
	case SCSI_State_DataIn:
	case SCSI_State_SendData:
	case SCSI_State_LastDataIn:
		gMSC.state = SCSI_ResumeCmd(&gMSC.scsi, gMSC.state);
		USB_MSC_SCSI_Reply(gMSC.state);
		break;
	}
}

void USB_MSC_Receive(uint32_t size)
{
	switch (gMSC.state)
	{
	case SCSI_State_DataOut:
		gMSC.state = SCSI_ResumeCmd(&gMSC.scsi, gMSC.state);
		USB_MSC_SCSI_Reply(gMSC.state);
		break;

	default:
		MSC_BOT_CBW_Decode(size);
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

	SCSI_t * scsi = &gMSC.scsi;

	if ((size != MSC_CBW_LENGTH) ||
	  (hmsc->cbw.dSignature != MSC_CBW_SIGNATURE) ||
	  (hmsc->cbw.bLUN > 1U) ||
	  (hmsc->cbw.bCBLength < 1U) || (hmsc->cbw.bCBLength > 16U))
	{
		SCSI_SenseCode(scsi, SCSI_SKEY_ILLEGAL_REQUEST, SCSI_ASQ_INVALID_CDB);

		gMSC.status = MSC_STATUS_ERROR;
		MSC_BOT_Abort();
	}
	else
	{
		gMSC.state = SCSI_ProcessCmd(scsi, &hmsc->cbw.CB[0]);
		USB_MSC_SCSI_Reply(gMSC.state);
	}
}

static void USB_MSC_SCSI_Reply(SCSI_State_t state)
{
	switch(state)
	{
	case SCSI_State_Error:
		MSC_BOT_SendCSW(USBD_CSW_CMD_FAILED);
		break;
	case SCSI_State_Ok:
		MSC_BOT_SendCSW(USBD_CSW_CMD_PASSED);
		break;
	case SCSI_State_SendData:
		MSC_BOT_SendData(gMSC.scsi.bfr, gMSC.scsi.data_len);
		break;
	case SCSI_State_DataOut:
		USB_EP_Read(MSC_OUT_EP, gMSC.scsi.bfr, gMSC.scsi.data_len);
		break;
	case SCSI_State_DataIn:
	case SCSI_State_LastDataIn:
		USB_EP_Write(MSC_IN_EP, gMSC.scsi.bfr, gMSC.scsi.data_len);
		break;
	default:
		break;
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
	hmsc->csw.dSignature = MSC_CSW_SIGNATURE;
	hmsc->csw.bStatus = CSW_Status;

	USB_EP_Write(MSC_IN_EP, (uint8_t *)&hmsc->csw, MSC_CSW_LENGTH);
	// Recieve next CBW
	USB_EP_Read(MSC_OUT_EP, (uint8_t *)&hmsc->cbw, MSC_CBW_LENGTH);
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
	  (gMSC.status == MSC_STATUS_NORMAL))
	{
		USB_EP_Stall(MSC_OUT_EP);
	}

	USB_EP_Stall(MSC_IN_EP);

	if (gMSC.status == MSC_STATUS_ERROR)
	{
		USB_EP_Read(MSC_OUT_EP, (uint8_t *)&hmsc->cbw, MSC_CBW_LENGTH);
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
	if (gMSC.status == MSC_STATUS_ERROR) /* Bad CBW Signature */
	{
		USB_EP_Stall(MSC_IN_EP);
		gMSC.status = MSC_STATUS_NORMAL;
	}
	else if (((epnum & 0x80U) == 0x80U) && (gMSC.status != MSC_STATUS_RECOVERY))
	{
		MSC_BOT_SendCSW(USBD_CSW_CMD_FAILED);
	}
}


#endif //USB_CLASS_MSC

