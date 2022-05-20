#ifndef CAN_H
#define CAN_H

#include "STM32X.h"

#ifdef CAN_GPIO

/*
 * FUNCTIONAL TESTING
 * STM32L0: N/A
 * STM32F0: Y
 * STM32G0: N/A
 */

/*
 * PUBLIC DEFINITIONS
 */

/*
 * PUBLIC TYPES
 */

typedef struct {
	uint32_t id;
	uint8_t data[8];
	uint8_t len;
} CANMsg_t;

/*
 * PUBLIC FUNCTIONS
 */

// Initialisation
void CAN_Init(uint32_t bitrate);
void CAN_EnableFilter(uint32_t bank, uint32_t id, uint32_t mask);
void CAN_Deinit(void);

// Transmit
bool CAN_Write(const CANMsg_t * msg);

// Receive
uint8_t CAN_ReadCount(void);
bool CAN_Read(CANMsg_t * msg);


#endif
#endif //CAN_H
