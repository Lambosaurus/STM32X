#include "SCSI.h"
#include "USB_MSC.h"
#include "../USB_EP.h"

#include <string.h>

/*
 * PRIVATE DEFINITIONS
 */

#define MODE_SENSE6_LEN                    8U
#define MODE_SENSE10_LEN                   8U
#define LENGTH_INQUIRY_PAGE00              7U
#define LENGTH_FORMAT_CAPACITIES           20U

#define SCSI_BLOCK_SIZE						USB_MSC_BLOCK_SIZE
#define SCSI_LUN							0

/*
 * PRIVATE TYPES
 */

/*
 * PRIVATE PROTOTYPES
 */

static int8_t SCSI_TestUnitReady(SCSI_t * scsi, uint8_t *params);
static int8_t SCSI_Inquiry(SCSI_t * scsi, uint8_t *params);
static int8_t SCSI_ReadFormatCapacity(SCSI_t * scsi, uint8_t *params);
static int8_t SCSI_ReadCapacity10(SCSI_t * scsi, uint8_t *params);
static int8_t SCSI_RequestSense(SCSI_t * scsi, uint8_t *params);
static int8_t SCSI_StartStopUnit(SCSI_t * scsi, uint8_t *params);
static int8_t SCSI_ModeSense6(SCSI_t * scsi, uint8_t *params);
static int8_t SCSI_ModeSense10(SCSI_t * scsi, uint8_t *params);
static int8_t SCSI_Write10(SCSI_t * scsi, uint8_t *params);
static int8_t SCSI_Read10(SCSI_t * scsi, uint8_t *params);
static int8_t SCSI_Verify10(SCSI_t * scsi, uint8_t *params);
static int8_t SCSI_CheckAddressRange(SCSI_t * scsi, uint32_t blk_offset, uint32_t blk_nbr);

static int8_t SCSI_ProcessRead(SCSI_t * scsi);
static int8_t SCSI_ProcessWrite(SCSI_t * scsi);

/*
 * PRIVATE VARIABLES
 */

const uint8_t  MSC_Page00_Inquiry_Data[] =
{
	0x00,
	0x00,
	0x00,
	(LENGTH_INQUIRY_PAGE00 - 4U),
	0x00,
	0x80,
	0x83
};

const uint8_t cSCSI_InquiryPage[] = {/* 36 */
	0x00,
	0x80,
	0x02,
	0x02,
	(0x24 - 5),
	0x00,
	0x00,
	0x00,
	'S', 'T', 'M', ' ', ' ', ' ', ' ', ' ', /* Manufacturer : 8 bytes */
	'P', 'r', 'o', 'd', 'u', 'c', 't', ' ', /* Product      : 16 Bytes */
	' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
	'0', '.', '0' ,'1'                      /* Version      : 4 Bytes */
};

/*
 * PUBLIC FUNCTIONS
 */

bool SCSI_Init(SCSI_t * scsi, USB_MSC_Storage_t * storage)
{
	scsi->sense.head = 0;
	scsi->sense.tail = 0;
	scsi->storage = NULL; // NULL storage indicates no disk.
	if (storage != NULL && storage->open(&scsi->block_count))
	{
		scsi->storage = storage;
		return true;
	}
	return false;
}

int8_t SCSI_ProcessCmd(SCSI_t * scsi, uint8_t * cmd)
{
	switch (cmd[0])
	{
	case SCSI_TEST_UNIT_READY:
		SCSI_TestUnitReady(scsi, cmd);
		break;
	case SCSI_REQUEST_SENSE:
		SCSI_RequestSense(scsi, cmd);
		break;
	case SCSI_INQUIRY:
		SCSI_Inquiry(scsi, cmd);
		break;
	case SCSI_START_STOP_UNIT:
		SCSI_StartStopUnit(scsi, cmd);
		break;
	case SCSI_ALLOW_MEDIUM_REMOVAL:
		SCSI_StartStopUnit(scsi, cmd);
		break;
	case SCSI_MODE_SENSE6:
		SCSI_ModeSense6(scsi, cmd);
		break;
	case SCSI_MODE_SENSE10:
		SCSI_ModeSense10(scsi, cmd);
		break;
	case SCSI_READ_FORMAT_CAPACITIES:
		SCSI_ReadFormatCapacity(scsi, cmd);
		break;
	case SCSI_READ_CAPACITY10:
		SCSI_ReadCapacity10(scsi, cmd);
		break;
	case SCSI_READ10:
		SCSI_Read10(scsi, cmd);
		break;
	case SCSI_WRITE10:
		SCSI_Write10(scsi, cmd);
		break;
	case SCSI_VERIFY10:
		SCSI_Verify10(scsi, cmd);
		break;
	default:
		SCSI_SenseCode(scsi, SCSI_SKEY_ILLEGAL_REQUEST, SCSI_ASQ_INVALID_CDB);
		return -1;
	}
	return 0;
}

/*
 * PRIVATE FUNCTIONS
 */

static int8_t SCSI_TestUnitReady(SCSI_t * scsi, uint8_t *params)
{
	USBD_MSC_BOT_HandleTypeDef  *hmsc = (USBD_MSC_BOT_HandleTypeDef *)scsi->pClassData;

	/* case 9 : Hi > D0 */
	if (hmsc->cbw.dDataLength != 0U)
	{
		SCSI_SenseCode(scsi, SCSI_SKEY_ILLEGAL_REQUEST, SCSI_ASQ_INVALID_CDB);
		return -1;
	}

	if (scsi->storage == NULL)
	{
		SCSI_SenseCode(scsi, SCSI_SKEY_NOT_READY, SCSI_ASQ_MEDIUM_NOT_PRESENT);
		hmsc->bot_state = USBD_BOT_NO_DATA;
		return -1;
	}
	scsi->data_len = 0;
	return 0;
}

static int8_t  SCSI_Inquiry(SCSI_t * scsi, uint8_t *params)
{
	uint16_t len;
	const uint8_t * page;

	if (params[1] & 0x01U) // Evpd is set
	{
		page = MSC_Page00_Inquiry_Data;
		len = LENGTH_INQUIRY_PAGE00;
	}
	else
	{
		page = cSCSI_InquiryPage;
		len = (uint16_t)page[4] + 5U;

		if (params[4] <= len)
		{
			len = params[4];
		}
	}

	scsi->data_len = len;
	memcpy(scsi->bfr, page, len);
	return 0;
}

static int8_t SCSI_ReadCapacity10(SCSI_t * scsi, uint8_t *params)
{
	if (scsi->storage == NULL)
	{
		SCSI_SenseCode(scsi, SCSI_SKEY_NOT_READY, SCSI_ASQ_MEDIUM_NOT_PRESENT);
		return -1;
	}
	else
	{
		uint32_t blk_nbr = scsi->block_count - 1;

		scsi->bfr[0] = (uint8_t)(blk_nbr >> 24);
		scsi->bfr[1] = (uint8_t)(blk_nbr >> 16);
		scsi->bfr[2] = (uint8_t)(blk_nbr >>  8);
		scsi->bfr[3] = (uint8_t)(blk_nbr);

		scsi->bfr[4] = (uint8_t)(SCSI_BLOCK_SIZE >>  24);
		scsi->bfr[5] = (uint8_t)(SCSI_BLOCK_SIZE >>  16);
		scsi->bfr[6] = (uint8_t)(SCSI_BLOCK_SIZE >>  8);
		scsi->bfr[7] = (uint8_t)(SCSI_BLOCK_SIZE);

		scsi->data_len = 8U;
		return 0;
	}
}

static int8_t SCSI_ReadFormatCapacity(SCSI_t * scsi, uint8_t *params)
{
	if (scsi->storage == NULL)
	{
		SCSI_SenseCode(scsi, SCSI_SKEY_NOT_READY, SCSI_ASQ_MEDIUM_NOT_PRESENT);
		return -1;
	}
	else
	{
		uint32_t blk_nbr = scsi->block_count - 1;

		scsi->bfr[0] = 0x00;
		scsi->bfr[1] = 0x00;
		scsi->bfr[2] = 0x00;
	    scsi->bfr[3] = 0x08U;
	    scsi->bfr[4] = (uint8_t)(blk_nbr >> 24);
	    scsi->bfr[5] = (uint8_t)(blk_nbr >> 16);
		scsi->bfr[6] = (uint8_t)(blk_nbr >>  8);
		scsi->bfr[7] = (uint8_t)blk_nbr;

		scsi->bfr[8] = 0x02U;
		scsi->bfr[9] = (uint8_t)(SCSI_BLOCK_SIZE >>  16);
		scsi->bfr[10] = (uint8_t)(SCSI_BLOCK_SIZE >>  8);
		scsi->bfr[11] = (uint8_t)(SCSI_BLOCK_SIZE);

		scsi->data_len = 12U;
		return 0;
	}
}

static int8_t SCSI_ModeSense6(SCSI_t * scsi, uint8_t *params)
{
	uint16_t len = MODE_SENSE6_LEN;
	scsi->data_len = len;
	memset(scsi->bfr, 0, len);
	return 0;
}

static int8_t SCSI_ModeSense10(SCSI_t * scsi, uint8_t *params)
{
	uint16_t len = MODE_SENSE10_LEN;
	scsi->data_len = len;
	memset(scsi->bfr, 0x00, len);
	// Byte 2 is constant 0x06. I dont know how these commands are formatted.
	scsi->bfr[2] = 0x06;
	return 0;
}

static int8_t SCSI_RequestSense(SCSI_t * scsi, uint8_t *params)
{
	memset(scsi->bfr, 0x00, REQUEST_SENSE_DATA_LEN);

	scsi->bfr[0] = 0x70U;
	scsi->bfr[7] = REQUEST_SENSE_DATA_LEN - 6U;

	if ((scsi->sense.head != scsi->sense.tail))
	{
		scsi->bfr[2]     = scsi->sense.stack[scsi->sense.head].Skey;
		//scsi->bfr[12]   // Leave ASCQ zero
		scsi->bfr[13]    = scsi->sense.stack[scsi->sense.head].ASC;
		scsi->sense.head++;

		if (scsi->sense.head == SCSI_SENSE_DEPTH)
		{
			scsi->sense.head = 0U;
		}
	}
	scsi->data_len = REQUEST_SENSE_DATA_LEN;

	if (params[4] <= REQUEST_SENSE_DATA_LEN)
	{
		scsi->data_len = params[4];
	}
	return 0;
}

void SCSI_SenseCode(SCSI_t  * scsi, uint8_t sKey, uint8_t ASC)
{
	scsi->sense.stack[scsi->sense.tail].Skey  = sKey;
	scsi->sense.stack[scsi->sense.tail].ASC   = ASC;
	scsi->sense.tail++;
	if (scsi->sense.tail == SCSI_SENSE_DEPTH)
	{
		scsi->sense.tail = 0U;
	}
}

static int8_t SCSI_StartStopUnit(SCSI_t  * scsi, uint8_t *params)
{
	scsi->data_len = 0U;
	return 0;
}

static int8_t SCSI_Read10(SCSI_t * scsi, uint8_t *params)
{
	USBD_MSC_BOT_HandleTypeDef  *hmsc = scsi->pClassData;

	if (hmsc->bot_state == USBD_BOT_IDLE) /* Idle */
	{
		/* case 10 : Ho <> Di */
		if ((hmsc->cbw.bmFlags & 0x80U) != 0x80U)
		{
			SCSI_SenseCode(scsi, SCSI_SKEY_ILLEGAL_REQUEST, SCSI_ASQ_INVALID_CDB);
			return -1;
		}

		if (scsi->storage == NULL)
		{
			SCSI_SenseCode(scsi, SCSI_SKEY_NOT_READY, SCSI_ASQ_MEDIUM_NOT_PRESENT);
			return -1;
		}

		scsi->block_addr = ((uint32_t)params[2] << 24)
						 | ((uint32_t)params[3] << 16)
						 | ((uint32_t)params[4] <<  8)
						 | (uint32_t)params[5];

		scsi->block_len = ((uint32_t)params[7] <<  8) | (uint32_t)params[8];

		if (SCSI_CheckAddressRange(scsi, scsi->block_addr, scsi->block_len) < 0)
		{
			return -1; /* error */
		}

		hmsc->bot_state = USBD_BOT_DATA_IN;

		/* cases 4,5 : Hi <> Dn */
		if (hmsc->cbw.dDataLength != (scsi->block_len * SCSI_BLOCK_SIZE))
		{
			SCSI_SenseCode(scsi, SCSI_SKEY_ILLEGAL_REQUEST, SCSI_ASQ_INVALID_CDB);
			return -1;
		}
	}
	scsi->data_len = SCSI_MEDIA_PACKET;

	return SCSI_ProcessRead(scsi);
}

static int8_t SCSI_Write10(SCSI_t * scsi, uint8_t *params)
{
	USBD_MSC_BOT_HandleTypeDef  *hmsc = scsi->pClassData;
	uint32_t len;

	if (hmsc->bot_state == USBD_BOT_IDLE) /* Idle */
	{
		// case 8 : Hi <> Do
		if ((hmsc->cbw.bmFlags & 0x80U) == 0x80U)
		{
			SCSI_SenseCode(scsi, SCSI_SKEY_ILLEGAL_REQUEST, SCSI_ASQ_INVALID_CDB);
			return -1;
		}

		if (scsi->storage == NULL)
		{
			SCSI_SenseCode(scsi, SCSI_SKEY_NOT_READY, SCSI_ASQ_MEDIUM_NOT_PRESENT);
			return -1;
		}

		if (scsi->storage->write == NULL)
		{
			SCSI_SenseCode(scsi, SCSI_SKEY_NOT_READY, SCSI_ASQ_WRITE_PROTECTED);
			return -1;
		}

		scsi->block_addr = ((uint32_t)params[2] << 24) |
							  ((uint32_t)params[3] << 16) |
							  ((uint32_t)params[4] << 8) |
							  (uint32_t)params[5];

		scsi->block_len = ((uint32_t)params[7] << 8) |
							 (uint32_t)params[8];

		// check if LBA address is in the right range
		if (SCSI_CheckAddressRange(scsi, scsi->block_addr, scsi->block_len) < 0)
		{
			return -1;
		}

		len = scsi->block_len * SCSI_BLOCK_SIZE;

		// cases 3,11,13 : Hn,Ho <> D0
		if (hmsc->cbw.dDataLength != len)
		{
			SCSI_SenseCode(scsi, SCSI_SKEY_ILLEGAL_REQUEST, SCSI_ASQ_INVALID_CDB);
			return -1;
		}

		len = MIN(len, SCSI_MEDIA_PACKET);

		// Prepare EP to receive first data packet
		hmsc->bot_state = USBD_BOT_DATA_OUT;
		USB_EP_Read(MSC_OUT_EP, scsi->bfr, len);
	}
	else // Write Process ongoing
	{
		return SCSI_ProcessWrite(scsi);
	}
	return 0;
}

static int8_t SCSI_Verify10(SCSI_t * scsi, uint8_t *params)
{
	if ((params[1] & 0x02U) == 0x02U)
	{
		SCSI_SenseCode(scsi, SCSI_SKEY_ILLEGAL_REQUEST, SCSI_ASQ_INVALID_FIELD_IN_COMMAND);
		return -1; /* Error, Verify Mode Not supported*/
	}

	if (SCSI_CheckAddressRange(scsi, scsi->block_addr, scsi->block_len) < 0)
	{
		return -1; /* error */
	}
	scsi->data_len = 0U;
	return 0;
}

static int8_t SCSI_CheckAddressRange(SCSI_t * scsi, uint32_t blk_offset, uint32_t blk_nbr)
{
	if ((blk_offset + blk_nbr) > scsi->block_count)
	{
		SCSI_SenseCode(scsi, SCSI_SKEY_ILLEGAL_REQUEST, SCSI_ASQ_ADDRESS_OUT_OF_RANGE);
		return -1;
	}
	return 0;
}

static int8_t SCSI_ProcessRead(SCSI_t * scsi)
{
	USBD_MSC_BOT_HandleTypeDef * hmsc = scsi->pClassData;
	uint32_t len = scsi->block_len * SCSI_BLOCK_SIZE;
	len = MIN(len, SCSI_MEDIA_PACKET);
	uint32_t blk_len = len / SCSI_BLOCK_SIZE;

	if (!scsi->storage->read(scsi->bfr, scsi->block_addr, blk_len))
	{
		SCSI_SenseCode(scsi, SCSI_SKEY_HARDWARE_ERROR, SCSI_ASQ_UNRECOVERED_READ_ERROR);
		return -1;
	}

	USB_EP_Write(MSC_IN_EP, scsi->bfr, len);

	scsi->block_addr += blk_len;
	scsi->block_len -= blk_len;

	// case 6 : Hi = Di
	hmsc->csw.dDataResidue -= len;

	if (scsi->block_len == 0U)
	{
		hmsc->bot_state = USBD_BOT_LAST_DATA_IN;
	}
	return 0;
}

static int8_t SCSI_ProcessWrite(SCSI_t  * scsi)
{
	USBD_MSC_BOT_HandleTypeDef * hmsc = scsi->pClassData;
	uint32_t len = scsi->block_len * SCSI_BLOCK_SIZE;
	len = MIN(len, SCSI_MEDIA_PACKET);
	uint32_t blk_len = len / SCSI_BLOCK_SIZE;

	if (!scsi->storage->write(scsi->bfr, scsi->block_addr, blk_len))
	{
		SCSI_SenseCode(scsi, SCSI_SKEY_HARDWARE_ERROR, SCSI_ASQ_WRITE_FAULT);
		return -1;
	}

	scsi->block_addr += blk_len;
	scsi->block_len -= blk_len;

	// case 12 : Ho = Do */
	hmsc->csw.dDataResidue -= len;

	if (scsi->block_len == 0U)
	{
		MSC_BOT_SendCSW(USBD_CSW_CMD_PASSED);
	}
	else
	{
		len = MIN((scsi->block_len * SCSI_BLOCK_SIZE), SCSI_MEDIA_PACKET);
		// Prepare EP to Receive next packet
		USB_EP_Read(MSC_OUT_EP, scsi->bfr, len);
	}

	return 0;
}

