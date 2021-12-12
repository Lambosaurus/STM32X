#ifndef USB_HID_H
#define USB_HID_H

#include "STM32X.h"
#include "../USB_Defs.h"

/*
 * FUNCTIONAL TESTING
 * STM32L0: N
 * STM32F0: N
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

typedef struct {
	uint8_t buttons;
	int8_t x;
	int8_t y;
} HID_Report_t;

/*
 * PUBLIC FUNCTIONS
 */

// Callbacks for USB_CTL.
// These should be referenced in USB_Class.h
void USB_HID_Init(uint8_t config);
void USB_HID_Deinit(void);
void USB_HID_Setup(USB_SetupRequest_t * req);

void USB_HID_Report(HID_Report_t * report);

/*
 * EXTERN DECLARATIONS
 */

extern const uint8_t cUSB_HID_ConfigDescriptor[USB_HID_CONFIG_DESC_SIZE];


#endif //USB_MSC_H
