#ifndef USB_H
#define USB_H

#include "STM32X.h"

#ifdef USB_ENABLE

#ifdef USB_CLASS_MSC
#include "usb/msc/USB_Storage.h"
#endif

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
void USB_WriteStr(const char * str);
#endif

#ifdef USB_CLASS_MSC
void USB_Mount(const USB_Storage_t * storage);
#endif

/*
 * EXTERN DECLARATIONS
 */

#endif //USB_ENABLE
#endif //USB_H
