#include "SCSI.h"
#include "USB_MSC.h"
#include <string.h>

/*
 * PRIVATE DEFINITIONS
 */

#define MODE_SENSE6_LEN                    8U
#define MODE_SENSE10_LEN                   8U
#define LENGTH_INQUIRY_PAGE00              7U
#define LENGTH_FORMAT_CAPACITIES           20U

#define SCSI_LUN							0

/*
 * PRIVATE TYPES
 */

/*
 * PRIVATE PROTOTYPES
 */

static SCSI_State_t SCSI_TestUnitReady(SCSI_t * scsi, uint8_t *params);
static SCSI_State_t SCSI_Inquiry(SCSI_t * scsi, uint8_t *params);
static SCSI_State_t SCSI_ReadFormatCapacity(SCSI_t * scsi, uint8_t *params);
static SCSI_State_t SCSI_ReadCapacity10(SCSI_t * scsi, uint8_t *params);
static SCSI_State_t SCSI_RequestSense(SCSI_t * scsi, uint8_t *params);
static SCSI_State_t SCSI_StartStopUnit(SCSI_t * scsi, uint8_t *params);
static SCSI_State_t SCSI_ModeSense6(SCSI_t * scsi, uint8_t *params);
static SCSI_State_t SCSI_ModeSense10(SCSI_t * scsi, uint8_t *params);
static SCSI_State_t SCSI_Write10(SCSI_t * scsi, uint8_t *params);
static SCSI_State_t SCSI_Read10(SCSI_t * scsi, uint8_t *params);
static SCSI_State_t SCSI_Verify10(SCSI_t * scsi, uint8_t *params);
static SCSI_State_t SCSI_CheckAddressRange(SCSI_t * scsi, uint32_t blk_offset, uint32_t blk_nbr);

static SCSI_State_t SCSI_ProcessRead(SCSI_t * scsi);
static SCSI_State_t SCSI_ProcessWrite(SCSI_t * scsi);

/*
 * PRIVATE VARIABLES
 */

const uint8_t  cSCSI_InquiryPage00[] =
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

SCSI_State_t SCSI_Init(SCSI_t * scsi, USB_MSC_Storage_t * storage)
{
	scsi->sense.head = 0;
	scsi->sense.tail = 0;
	scsi->storage = NULL; // NULL storage indicates no disk.
	if (storage != NULL && storage->open(&scsi->block_count))
	{
		scsi->storage = storage;
	}
	return SCSI_State_Ok;
}

SCSI_State_t SCSI_ProcessCmd(SCSI_t * scsi, uint8_t * cmd)
{
	switch (cmd[0])
	{
	case SCSI_TEST_UNIT_READY:
		return SCSI_TestUnitReady(scsi, cmd);
	case SCSI_REQUEST_SENSE:
		return SCSI_RequestSense(scsi, cmd);
	case SCSI_INQUIRY:
		return SCSI_Inquiry(scsi, cmd);
	case SCSI_START_STOP_UNIT:
		return SCSI_StartStopUnit(scsi, cmd);
	case SCSI_ALLOW_MEDIUM_REMOVAL:
		return SCSI_StartStopUnit(scsi, cmd);
	case SCSI_MODE_SENSE6:
		return SCSI_ModeSense6(scsi, cmd);
	case SCSI_MODE_SENSE10:
		return SCSI_ModeSense10(scsi, cmd);
	case SCSI_READ_FORMAT_CAPACITIES:
		return SCSI_ReadFormatCapacity(scsi, cmd);
	case SCSI_READ_CAPACITY10:
		return SCSI_ReadCapacity10(scsi, cmd);
	case SCSI_READ10:
		return SCSI_Read10(scsi, cmd);
	case SCSI_WRITE10:
		return SCSI_Write10(scsi, cmd);
	case SCSI_VERIFY10:
		return SCSI_Verify10(scsi, cmd);
	default:
		return SCSI_SenseCode(scsi, SCSI_SKEY_ILLEGAL_REQUEST, SCSI_ASQ_INVALID_CDB);
	}
}

SCSI_State_t SCSI_ResumeCmd(SCSI_t * scsi, SCSI_State_t state)
{
	switch (state)
	{
	case SCSI_State_DataOut:
		return SCSI_ProcessWrite(scsi);

	case SCSI_State_DataIn:
		return SCSI_ProcessRead(scsi);

	case SCSI_State_SendData:
	case SCSI_State_LastDataIn:
		return SCSI_State_Ok;

	default:
		return SCSI_State_Error;
	}
}

/*
 * PRIVATE FUNCTIONS
 */

static SCSI_State_t SCSI_TestUnitReady(SCSI_t * scsi, uint8_t *params)
{
	USBD_MSC_BOT_HandleTypeDef  *hmsc = (USBD_MSC_BOT_HandleTypeDef *)scsi->pClassData;

	/* case 9 : Hi > D0 */
	if (hmsc->cbw.dDataLength != 0U)
	{
		return SCSI_SenseCode(scsi, SCSI_SKEY_ILLEGAL_REQUEST, SCSI_ASQ_INVALID_CDB);
	}

	if (scsi->storage == NULL)
	{
		return SCSI_SenseCode(scsi, SCSI_SKEY_NOT_READY, SCSI_ASQ_MEDIUM_NOT_PRESENT);
	}
	return SCSI_State_Ok;
}

static SCSI_State_t  SCSI_Inquiry(SCSI_t * scsi, uint8_t *params)
{
	uint16_t len;
	const uint8_t * page;

	if (params[1] & 0x01U) // Evpd is set
	{
		page = cSCSI_InquiryPage00;
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
	return SCSI_State_SendData;
}

static SCSI_State_t SCSI_ReadCapacity10(SCSI_t * scsi, uint8_t *params)
{
	if (scsi->storage == NULL)
	{
		return SCSI_SenseCode(scsi, SCSI_SKEY_NOT_READY, SCSI_ASQ_MEDIUM_NOT_PRESENT);
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
		return SCSI_State_SendData;
	}
}

static SCSI_State_t SCSI_ReadFormatCapacity(SCSI_t * scsi, uint8_t *params)
{
	if (scsi->storage == NULL)
	{
		return SCSI_SenseCode(scsi, SCSI_SKEY_NOT_READY, SCSI_ASQ_MEDIUM_NOT_PRESENT);
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
		return SCSI_State_SendData;
	}
}

static SCSI_State_t SCSI_ModeSense6(SCSI_t * scsi, uint8_t *params)
{
	uint16_t len = MODE_SENSE6_LEN;
	scsi->data_len = len;
	memset(scsi->bfr, 0, len);
	return SCSI_State_SendData;
}

static SCSI_State_t SCSI_ModeSense10(SCSI_t * scsi, uint8_t *params)
{
	uint16_t len = MODE_SENSE10_LEN;
	scsi->data_len = len;
	memset(scsi->bfr, 0x00, len);
	// Byte 2 is constant 0x06. I dont know how these commands are formatted.
	scsi->bfr[2] = 0x06;
	return SCSI_State_SendData;
}

static SCSI_State_t SCSI_RequestSense(SCSI_t * scsi, uint8_t *params)
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
	return SCSI_State_SendData;
}

SCSI_State_t SCSI_SenseCode(SCSI_t  * scsi, uint8_t sKey, uint8_t ASC)
{
	scsi->sense.stack[scsi->sense.tail].Skey  = sKey;
	scsi->sense.stack[scsi->sense.tail].ASC   = ASC;
	scsi->sense.tail++;
	if (scsi->sense.tail == SCSI_SENSE_DEPTH)
	{
		scsi->sense.tail = 0U;
	}
	return SCSI_State_Error;
}

static SCSI_State_t SCSI_StartStopUnit(SCSI_t  * scsi, uint8_t *params)
{
	return SCSI_State_Ok;
}

static SCSI_State_t SCSI_Read10(SCSI_t * scsi, uint8_t *params)
{
	USBD_MSC_BOT_HandleTypeDef  *hmsc = scsi->pClassData;

	/* case 10 : Ho <> Di */
	if ((hmsc->cbw.bmFlags & 0x80U) != 0x80U)
	{
		return SCSI_SenseCode(scsi, SCSI_SKEY_ILLEGAL_REQUEST, SCSI_ASQ_INVALID_CDB);
	}

	if (scsi->storage == NULL)
	{
		return SCSI_SenseCode(scsi, SCSI_SKEY_NOT_READY, SCSI_ASQ_MEDIUM_NOT_PRESENT);
	}

	scsi->block_addr = ((uint32_t)params[2] << 24)
					 | ((uint32_t)params[3] << 16)
					 | ((uint32_t)params[4] <<  8)
					 | (uint32_t)params[5];

	scsi->block_len = ((uint32_t)params[7] <<  8) | (uint32_t)params[8];

	if (SCSI_CheckAddressRange(scsi, scsi->block_addr, scsi->block_len) != SCSI_State_Ok)
	{
		return SCSI_State_Error;
	}

	/* cases 4,5 : Hi <> Dn */
	if (hmsc->cbw.dDataLength != (scsi->block_len * SCSI_BLOCK_SIZE))
	{
		return SCSI_SenseCode(scsi, SCSI_SKEY_ILLEGAL_REQUEST, SCSI_ASQ_INVALID_CDB);
	}

	return SCSI_ProcessRead(scsi);
}

static SCSI_State_t SCSI_Write10(SCSI_t * scsi, uint8_t *params)
{
	USBD_MSC_BOT_HandleTypeDef  *hmsc = scsi->pClassData;

	// case 8 : Hi <> Do
	if ((hmsc->cbw.bmFlags & 0x80U) == 0x80U)
	{
		return SCSI_SenseCode(scsi, SCSI_SKEY_ILLEGAL_REQUEST, SCSI_ASQ_INVALID_CDB);
	}

	if (scsi->storage == NULL)
	{
		return SCSI_SenseCode(scsi, SCSI_SKEY_NOT_READY, SCSI_ASQ_MEDIUM_NOT_PRESENT);
	}

	if (scsi->storage->write == NULL)
	{
		return SCSI_SenseCode(scsi, SCSI_SKEY_NOT_READY, SCSI_ASQ_WRITE_PROTECTED);
	}

	scsi->block_addr = ((uint32_t)params[2] << 24) |
						  ((uint32_t)params[3] << 16) |
						  ((uint32_t)params[4] << 8) |
						  (uint32_t)params[5];

	scsi->block_len = ((uint32_t)params[7] << 8) |
						 (uint32_t)params[8];

	// check if LBA address is in the right range
	if (SCSI_CheckAddressRange(scsi, scsi->block_addr, scsi->block_len) != SCSI_State_Ok)
	{
		return SCSI_State_Error;
	}

	uint32_t len = scsi->block_len * SCSI_BLOCK_SIZE;

	// cases 3,11,13 : Hn,Ho <> D0
	if (hmsc->cbw.dDataLength != len)
	{
		return SCSI_SenseCode(scsi, SCSI_SKEY_ILLEGAL_REQUEST, SCSI_ASQ_INVALID_CDB);
	}

	len = MIN(len, SCSI_BLOCK_SIZE);
	scsi->data_len = len;
	return SCSI_State_DataOut;
}

static SCSI_State_t SCSI_Verify10(SCSI_t * scsi, uint8_t *params)
{
	if ((params[1] & 0x02U) == 0x02U)
	{
		// Verify mode not supported
		return SCSI_SenseCode(scsi, SCSI_SKEY_ILLEGAL_REQUEST, SCSI_ASQ_INVALID_FIELD_IN_COMMAND);
	}

	if (SCSI_CheckAddressRange(scsi, scsi->block_addr, scsi->block_len) < 0)
	{
		return SCSI_State_Error;
	}
	return SCSI_State_Ok;
}

static SCSI_State_t SCSI_CheckAddressRange(SCSI_t * scsi, uint32_t blk_offset, uint32_t blk_nbr)
{
	if ((blk_offset + blk_nbr) > scsi->block_count)
	{
		return SCSI_SenseCode(scsi, SCSI_SKEY_ILLEGAL_REQUEST, SCSI_ASQ_ADDRESS_OUT_OF_RANGE);
	}
	return SCSI_State_Ok;
}

static SCSI_State_t SCSI_ProcessRead(SCSI_t * scsi)
{
	USBD_MSC_BOT_HandleTypeDef * hmsc = scsi->pClassData;
	uint32_t len = scsi->block_len * SCSI_BLOCK_SIZE;
	len = MIN(len, SCSI_BLOCK_SIZE);
	uint32_t blk_len = len / SCSI_BLOCK_SIZE;

	if (!scsi->storage->read(scsi->bfr, scsi->block_addr, blk_len))
	{
		return SCSI_SenseCode(scsi, SCSI_SKEY_HARDWARE_ERROR, SCSI_ASQ_UNRECOVERED_READ_ERROR);
	}

	scsi->block_addr += blk_len;
	scsi->block_len -= blk_len;
	scsi->data_len = SCSI_BLOCK_SIZE;

	// case 6 : Hi = Di
	hmsc->csw.dDataResidue -= len;

	if (scsi->block_len == 0U)
	{
		return SCSI_State_LastDataIn;
	}
	return SCSI_State_DataIn;
}

static SCSI_State_t SCSI_ProcessWrite(SCSI_t  * scsi)
{
	USBD_MSC_BOT_HandleTypeDef * hmsc = scsi->pClassData;

	uint32_t len = scsi->block_len * SCSI_BLOCK_SIZE;
	len = MIN(len, SCSI_BLOCK_SIZE);
	uint32_t blk_len = len / SCSI_BLOCK_SIZE;

	if (!scsi->storage->write(scsi->bfr, scsi->block_addr, blk_len))
	{
		return SCSI_SenseCode(scsi, SCSI_SKEY_HARDWARE_ERROR, SCSI_ASQ_WRITE_FAULT);
	}

	scsi->block_addr += blk_len;
	scsi->block_len -= blk_len;

	// case 12 : Ho = Do */
	hmsc->csw.dDataResidue -= len;

	if (scsi->block_len == 0U)
	{
		return SCSI_State_Ok;
	}
	else
	{
		scsi->data_len = SCSI_BLOCK_SIZE;
		return SCSI_State_DataOut;
	}
}

