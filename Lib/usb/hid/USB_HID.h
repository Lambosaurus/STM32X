#ifndef USB_HID_H
#define USB_HID_H

#include "STM32X.h"
#include "../USB_Defs.h"

#ifdef USB_CLASS_HID

#ifdef USB_HID_MOUSE
#include "devices/HID_Mouse.h"
#else
#error "No HID device type defined"
#endif

/*
 * FUNCTIONAL TESTING
 * STM32L0: Y
 * STM32F0: N
 */


/*
 * TODO:
 * 	Boot protocols are not yet implemented.
 *  Multiple reports are not yet implemented.
 *  Keyboard, Gamepad & Custom devices are not yet implemented.
 *  Mouse with scroll wheel would be nice too.
 */

/*
 * PUBLIC DEFINITIONS
 */

#define USB_HID_INTERFACES				1
#define USB_HID_ENDPOINTS				2

#define USB_HID_CONFIG_DESC_SIZE		41
#define USB_HID_CONFIG_DESC				cUSB_HID_ConfigDescriptor

/*
 * PUBLIC TYPES
 */

/*
 * PUBLIC FUNCTIONS
 */

// Callbacks for USB_CTL.
// These should be referenced in USB_Class.h
void USB_HID_Init(uint8_t config);
void USB_HID_Deinit(void);
void USB_HID_Setup(USB_SetupRequest_t * req);

void USB_HID_Report(USB_HID_Report_t * report);

/*
 * EXTERN DECLARATIONS
 */

extern const uint8_t cUSB_HID_ConfigDescriptor[USB_HID_CONFIG_DESC_SIZE];


#endif //USB_CLASS_HID
#endif //USB_MSC_H
