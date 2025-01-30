#ifndef USB_H
#define USB_H

#include "STM32X.h"

#ifdef USB_ENABLE
#include "usb/USB_Class.h"

/*
 * FUNCTIONAL TESTING
 * STM32L0: Y
 * STM32F0: Y
 * STM32G0: Y
 * STM32WL: N/A
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
bool USB_IsEnumerated(void);

/*
 * EXTERN DECLARATIONS
 */

#endif //USB_ENABLE
#endif //USB_H
