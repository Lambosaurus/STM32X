#ifndef USB_MSC_H
#define USB_MSC_H

#include "STM32X.h"
#include "../USB_Defs.h"
#include "USB_MSC_Storage.h"

/*
 * FUNCTIONAL TESTING
 * STM32L0: Y
 * STM32F0: N
 */

/*
 * PUBLIC DEFINITIONS
 */

#define USB_MSC_CONFIG_DESC_SIZE		32
#define USB_MSC_CONFIG_DESC				cUSB_MSC_ConfigDescriptor

#define MSC_IN_EP                        0x81
#define MSC_OUT_EP                       0x01

#define USBD_CSW_CMD_PASSED              0x00U
#define USBD_CSW_CMD_FAILED              0x01U

/*
 * PUBLIC TYPES
 */

/*
 * PUBLIC FUNCTIONS
 */

// Callbacks for USB_CTL.
// These should be referenced in USB_Class.h
void USB_MSC_Init(uint8_t config);
void USB_MSC_Deinit(void);
void USB_MSC_Setup(USB_SetupRequest_t * req);

// Interface to user
void USB_MSC_Mount(USB_MSC_Storage_t * storage);

/*
 * EXTERN DECLARATIONS
 */

extern const uint8_t cUSB_MSC_ConfigDescriptor[USB_MSC_CONFIG_DESC_SIZE];


#endif //USB_MSC_H
