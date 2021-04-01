#ifndef USB_H
#define USB_H

#include "STM32X.h"

/*
 * FUNCTIONAL TESTING
 * STM32L0: N
 * STM32F0: N
 */

/*
 * PUBLIC DEFINITIONS
 */

/*
 * PUBLIC TYPES
 */

/*
 * PUBLIC FUNCTIONS
 */

// Initialisation
void USB_Init(void);
void USB_Deinit(void);

void USB_Tx(const uint8_t * data, uint32_t count);
uint32_t USB_Rx(uint8_t * data, uint32_t size);

/*
 * EXTERN DECLARATIONS
 */

#endif //USB_H
