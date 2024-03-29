#ifndef MTP_H
#define MTP_H

#include "STM32X.h"
#include "MTP_Defs.h"
#include "MTP_FS.h"

/*
 * PUBLIC DEFINITIONS
 */

typedef enum
{
	MTP_State_RxOperation,
	MTP_State_RxData,
	MTP_State_TxData,
	MTP_State_TxDataLast,
	MTP_State_TxResponse,
} MTP_State_t;

/*
 * PUBLIC TYPES
 */

MTP_State_t MTP_Reset(MTP_t * mtp);
MTP_State_t MTP_HandleOperation(MTP_t * mtp, MTP_Operation_t * op, MTP_Container_t * container);
MTP_State_t MTP_HandleData(MTP_t * mtp, MTP_Operation_t * op, MTP_Container_t * container);
MTP_State_t MTP_NextData(MTP_t * mtp, MTP_Operation_t * op, MTP_Container_t * container);

uint16_t MTP_GetState(MTP_t * mtp);

// Extern function. TODO: Clean this up.
bool USB_MTP_SubmitEvent(MTP_Event_t * event);
void MTP_UpdateFileEvent(MTP_t * mtp, MTP_File_t * file, uint16_t code);

/*
 * PUBLIC FUNCTIONS
 */

/*
 * EXTERN DECLARATIONS
 */

#endif //MTP_H
