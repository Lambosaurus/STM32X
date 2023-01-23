#ifndef SUBGHZ_H
#define SUBGHZ_H

#include "STM32X.h"

#if defined(STM32WL)
#define SUBGHZ_ENABLED
#endif

/*
 * FUNCTIONAL TESTING
 * STM32L0: N/A
 * STM32F0: N/A
 * STM32G0: N/A
 * STM32WL: N
 */

#ifdef SUBGHZ_ENABLED

/*
 * PUBLIC DEFINITIONS
 */

/*
 * PUBLIC TYPES
 */

typedef enum {
	SUBGHZ_Event_TxComplete 			= 0x0001,
	SUBGHZ_Event_RxComplete 			= 0x0002,
	SUBGHZ_Event_PreambleDetected 		= 0x0004,
	SUBGHZ_Event_SyncwordValid			= 0x0008,
	SUBGHZ_Event_HeaderValid			= 0x0010,
	SUBGHZ_Event_HeaderError			= 0x0020,
	SUBGHZ_Event_CRCError				= 0x0040,
	SUBGHZ_Event_CADDetected			= 0x0080,
	SUBGHZ_Event_CADClear				= 0x0100,
	SUBGHZ_Event_RxTxTimeout			= 0x0200,
} SUBGHZ_Event_t;

typedef void (*SUBGHZ_Callback_t)(SUBGHZ_Event_t event);

/*
 * PUBLIC FUNCTIONS
 */

void SUBGHZ_Init(void);
void SUBGHZ_Deinit(void);

void SUBGHZ_OnEvent(SUBGHZ_Callback_t callback);

void SUBGHZ_ExecuteSet(SUBGHZ_RadioSetCmd_t cmd, const uint8_t * bfr, uint32_t size);
void SUBGHZ_ExecuteGet(SUBGHZ_RadioGetCmd_t cmd, uint8_t * bfr, uint32_t size);

void SUBGHZ_ReadBuffer(uint8_t offset, uint8_t * bfr, uint32_t size);
void SUBGHZ_WriteBuffer(uint8_t offset, const uint8_t * bfr, uint32_t size);

void SUBGHZ_ReadRegister(uint16_t address, uint8_t * value);
void SUBGHZ_WriteRegister(uint16_t address, uint8_t value);
void SUBGHZ_ReadRegisters(uint16_t address, uint8_t * value, uint32_t count);
void SUBGHZ_WriteRegisters(uint16_t address, const uint8_t * value, uint32_t count);

/*
 * EXTERN DECLARATIONS
 */

#endif //SUBGHZ_ENABLED
#endif //SUBGHZ_H
