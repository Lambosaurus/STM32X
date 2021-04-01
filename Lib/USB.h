#ifndef USB_H
#define USB_H

#include "STM32X.h"

#ifdef USB_ENABLE

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

/*
 * PUBLIC FUNCTIONS
 */

// Initialisation
void USB_Init(void);
void USB_Deinit(void);

#ifdef USB_CLASS_CDC
void USB_Write(const uint8_t * data, uint32_t count);
uint32_t USB_Read(uint8_t * data, uint32_t size);
#endif

/*
 * EXTERN DECLARATIONS
 */

#endif //USB_ENABLE
#endif //USB_H
