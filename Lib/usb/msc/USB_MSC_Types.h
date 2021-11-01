#ifndef USB_MSC_TYPES_H
#define USB_MSC_TYPES_H

#include "../USB_Defs.h"

/*
 * FUNCTIONAL TESTING
 * STM32L0: Y
 * STM32F0: N
 */

/*
 * PUBLIC DEFINITIONS
 */

#define USB_MSC_CONFIG_DESC_SIZE	32
#define USB_MSC_CONFIG_DESC			cUSB_MSC_ConfigDescriptor

#define SENSE_LIST_DEEPTH			4
#define MSC_MEDIA_PACKET			512

/*
 * PUBLIC TYPES
 */

typedef struct _USBD_STORAGE
{
  int8_t (* Init)(uint8_t lun);
  int8_t (* GetCapacity)(uint8_t lun, uint32_t *block_num, uint16_t *block_size);
  int8_t (* IsReady)(uint8_t lun);
  int8_t (* IsWriteProtected)(uint8_t lun);
  int8_t (* Read)(uint8_t lun, uint8_t *buf, uint32_t blk_addr, uint16_t blk_len);
  int8_t (* Write)(uint8_t lun, uint8_t *buf, uint32_t blk_addr, uint16_t blk_len);
  int8_t (* GetMaxLun)(void);
  int8_t *pInquiry;
} USBD_StorageTypeDef;

typedef struct
{
  uint32_t dSignature;
  uint32_t dTag;
  uint32_t dDataLength;
  uint8_t  bmFlags;
  uint8_t  bLUN;
  uint8_t  bCBLength;
  uint8_t  CB[16];
  uint8_t  ReservedForAlign;
} USBD_MSC_BOT_CBWTypeDef;

typedef struct
{
  uint32_t dSignature;
  uint32_t dTag;
  uint32_t dDataResidue;
  uint8_t  bStatus;
  uint8_t  ReservedForAlign[3];
} USBD_MSC_BOT_CSWTypeDef;

typedef struct _SENSE_ITEM
{
  char Skey;
  union
  {
    struct _ASCs
    {
      char ASC;
      char ASCQ;
    } b;
    uint8_t ASC;
    char *pData;
  } w;
} USBD_SCSI_SenseTypeDef;

typedef struct
{
  uint32_t                 max_lun;
  uint32_t                 interface;
  uint8_t                  bot_state;
  uint8_t                  bot_status;
  uint16_t                 bot_data_length;
  uint8_t                  bot_data[MSC_MEDIA_PACKET];
  USBD_MSC_BOT_CBWTypeDef  cbw;
  USBD_MSC_BOT_CSWTypeDef  csw;

  USBD_SCSI_SenseTypeDef   scsi_sense [SENSE_LIST_DEEPTH];
  uint8_t                  scsi_sense_head;
  uint8_t                  scsi_sense_tail;

  uint16_t                 scsi_blk_size;
  uint32_t                 scsi_blk_nbr;

  uint32_t                 scsi_blk_addr;
  uint32_t                 scsi_blk_len;
} USBD_MSC_BOT_HandleTypeDef;

typedef struct {
	USBD_MSC_BOT_HandleTypeDef * pClassData;
	USBD_StorageTypeDef * pUserData;
} SCSI_t;


#endif //USB_MSC_TYPES_H
