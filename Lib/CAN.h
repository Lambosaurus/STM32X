#ifndef CAN_H
#define CAN_H

#include "STM32X.h"

#ifdef CAN_PINS

/*
 * FUNCTIONAL TESTING
 * STM32L0: N/A
 * STM32F0: Y
 * STM32G0: N
 * STM32WL: N
 */

/*
 * PUBLIC DEFINITIONS
 */

#define CAN_MAILBOX_COUNT	3

/*
 * PUBLIC TYPES
 */

typedef struct {
	uint32_t id;
	uint8_t data[8];
	uint8_t len;
	bool ext;
} CAN_Msg_t;

typedef enum {
	CAN_Mode_Default 			= 0,
	CAN_Mode_Silent 			= (1 << 0),
	CAN_Mode_TransmitFIFO		= (1 << 1),
} CAN_Mode_t;

typedef enum {
	CAN_Error_None 				= 0,
	CAN_Error_Stuff				= 1,
	CAN_Error_Form				= 2,
	CAN_Error_Acknowledgement	= 3,
	CAN_Error_RecessiveBit		= 4,
	CAN_Error_DominantBit		= 5,
	CAN_Error_CRC				= 6,
	CAN_Error_Software			= 7,
	CAN_Error_RxOverrun			= 8,
} CAN_Error_t;

typedef void (*CAN_ErrorCallback_t)(CAN_Error_t error);

/*
 * PUBLIC FUNCTIONS
 */

// Initialisation
void CAN_Init(uint32_t bitrate, CAN_Mode_t mode);
void CAN_EnableFilter(uint32_t bank, uint32_t id, uint32_t mask);
void CAN_Deinit(void);

// Transmit
bool CAN_Write(const CAN_Msg_t * msg);
uint32_t CAN_WriteFree(void);

// Receive
uint32_t CAN_ReadCount(void);
bool CAN_Read(CAN_Msg_t * msg);

#ifdef CAN_USE_IRQS
void CAN_OnError(CAN_ErrorCallback_t callback);
void CAN_IRQHandler(void);
#endif //CAN_USE_IRQS


#endif //CAN_PINS
#endif //CAN_H
