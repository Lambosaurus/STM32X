#ifndef SCSI_H
#define SCSI_H

#include <stdint.h>
#include <stdbool.h>
#include "USB_MSC_Types.h"

/*
 * PUBLIC DEFINITIONS
 */

#define SCSI_BLOCK_SIZE								512

#ifndef SCSI_SENSE_DEPTH
#define SCSI_SENSE_DEPTH                           	4U
#endif

// SCSI Commands
#define SCSI_FORMAT_UNIT                            0x04U
#define SCSI_INQUIRY                                0x12U
#define SCSI_MODE_SELECT6                           0x15U
#define SCSI_MODE_SELECT10                          0x55U
#define SCSI_MODE_SENSE6                            0x1AU
#define SCSI_MODE_SENSE10                           0x5AU
#define SCSI_ALLOW_MEDIUM_REMOVAL                   0x1EU
#define SCSI_READ6                                  0x08U
#define SCSI_READ10                                 0x28U
#define SCSI_READ12                                 0xA8U
#define SCSI_READ16                                 0x88U

#define SCSI_READ_CAPACITY10                        0x25U
#define SCSI_READ_CAPACITY16                        0x9EU

#define SCSI_REQUEST_SENSE                          0x03U
#define SCSI_START_STOP_UNIT                        0x1BU
#define SCSI_TEST_UNIT_READY                        0x00U
#define SCSI_WRITE6                                 0x0AU
#define SCSI_WRITE10                                0x2AU
#define SCSI_WRITE12                                0xAAU
#define SCSI_WRITE16                                0x8AU

#define SCSI_VERIFY10                               0x2FU
#define SCSI_VERIFY12                               0xAFU
#define SCSI_VERIFY16                               0x8FU

#define SCSI_SEND_DIAGNOSTIC                        0x1DU
#define SCSI_READ_FORMAT_CAPACITIES                 0x23U

// SCSI errors
#define SCSI_SKEY_NO_SENSE                                    0U
#define SCSI_SKEY_RECOVERED_ERROR                             1U
#define SCSI_SKEY_NOT_READY                                   2U
#define SCSI_SKEY_MEDIUM_ERROR                                3U
#define SCSI_SKEY_HARDWARE_ERROR                              4U
#define SCSI_SKEY_ILLEGAL_REQUEST                             5U
#define SCSI_SKEY_UNIT_ATTENTION                              6U
#define SCSI_SKEY_DATA_PROTECT                                7U
#define SCSI_SKEY_BLANK_CHECK                                 8U
#define SCSI_SKEY_VENDOR_SPECIFIC                             9U
#define SCSI_SKEY_COPY_ABORTED                                10U
#define SCSI_SKEY_ABORTED_COMMAND                             11U
#define SCSI_SKEY_VOLUME_OVERFLOW                             13U
#define SCSI_SKEY_MISCOMPARE                                  14U


#define SCSI_ASQ_INVALID_CDB                                 0x20U
#define SCSI_ASQ_INVALID_FIELD_IN_COMMAND                    0x24U
#define SCSI_ASQ_PARAMETER_LIST_LENGTH_ERROR                 0x1AU
#define SCSI_ASQ_INVALID_FIELD_IN_PARAMETER_LIST             0x26U
#define SCSI_ASQ_ADDRESS_OUT_OF_RANGE                        0x21U
#define SCSI_ASQ_MEDIUM_NOT_PRESENT                          0x3AU
#define SCSI_ASQ_MEDIUM_HAVE_CHANGED                         0x28U
#define SCSI_ASQ_WRITE_PROTECTED                             0x27U
#define SCSI_ASQ_UNRECOVERED_READ_ERROR                      0x11U
#define SCSI_ASQ_WRITE_FAULT                                 0x03U

// SCSI Blocks sizes?
#define READ_FORMAT_CAPACITY_DATA_LEN               0x0CU
#define READ_CAPACITY10_DATA_LEN                    0x08U
#define MODE_SENSE10_DATA_LEN                       0x08U
#define MODE_SENSE6_DATA_LEN                        0x04U
#define REQUEST_SENSE_DATA_LEN                      0x12U
#define STANDARD_INQUIRY_DATA_LEN                   0x24U
#define BLKVFY                                      0x04U

/*
 * PUBLIC TYPES
 */

typedef struct
{
  char Skey;
  uint8_t ASC;
} SCSI_Sense_t;

typedef struct {
	USBD_MSC_BOT_HandleTypeDef * pClassData;
	USB_MSC_Storage_t * storage;

	uint32_t block_count;
	uint32_t block_addr;
	uint32_t block_len;

	struct {
		SCSI_Sense_t stack[SCSI_SENSE_DEPTH];
		uint8_t head;
		uint8_t tail;
	} sense;

	uint8_t bfr[SCSI_BLOCK_SIZE];
	uint16_t data_len;
} SCSI_t;

typedef enum {
	SCSI_State_Error = -1,
	SCSI_State_Ok = 0,
	SCSI_State_SendData,
	SCSI_State_DataOut,
	SCSI_State_DataIn,
	SCSI_State_LastDataIn,
} SCSI_State_t;

/*
 * PUBLIC FUNCTIONS
 */

SCSI_State_t SCSI_Init(SCSI_t * scsi, USB_MSC_Storage_t * storage);
SCSI_State_t SCSI_ProcessCmd(SCSI_t * scsi, uint8_t *cmd);
SCSI_State_t SCSI_ResumeCmd(SCSI_t * scsi, SCSI_State_t state);

// TODO: Make internal.
SCSI_State_t SCSI_SenseCode(SCSI_t * scsi, uint8_t sKey, uint8_t ASC);



#endif // SCSI_H

