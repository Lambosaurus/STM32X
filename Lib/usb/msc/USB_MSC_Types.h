#ifndef USB_MSC_TYPES_H
#define USB_MSC_TYPES_H

#include "../USB_Defs.h"
#include "USB_MSC_Storage.h"

/*
 * FUNCTIONAL TESTING
 * STM32L0: Y
 * STM32F0: N
 */

/*
 * PUBLIC DEFINITIONS
 */

/*
 * PUBLIC TYPES
 */


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
} SCSI_CBW_t;

typedef struct
{
  uint32_t dSignature;
  uint32_t dTag;
  uint32_t dDataResidue;
  uint8_t  bStatus;
  uint8_t  ReservedForAlign[3];
} SCSI_CSW_t;

typedef struct
{
  uint8_t     bot_state;
  //uint8_t     bot_status;
  SCSI_CBW_t  cbw;
  SCSI_CSW_t  csw;
} USBD_MSC_BOT_HandleTypeDef;



#endif //USB_MSC_TYPES_H
