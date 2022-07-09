#ifndef CAN_H
#define CAN_H

#include "STM32X.h"

#ifdef CAN_GPIO

/*
 * FUNCTIONAL TESTING
 * STM32L0: N/A
 * STM32F0: Y
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

typedef enum
{
	CAN_Mode_Default 			= 0,
	CAN_Mode_Silent 			= (1 << 0),
	CAN_Mode_MailboxFIFO		= (1 << 1),
} CAN_Mode_t;

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


#endif
#endif //CAN_H
