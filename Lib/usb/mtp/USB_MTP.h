#ifndef USB_MTP_H
#define USB_MTP_H

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

#define USB_MTP_INTERFACES				1
#define USB_MTP_ENDPOINTS				2

#define USB_MTP_CONFIG_DESC_SIZE		41
#define USB_MTP_CONFIG_DESC				cUSB_MTP_ConfigDescriptor

/*
 * PUBLIC TYPES
 */

/*
 * PUBLIC FUNCTIONS
 */

// Callbacks for USB_CTL.
// These should be referenced in USB_Class.h
void USB_MTP_Init(uint8_t config);
void USB_MTP_Deinit(void);
void USB_MTP_Setup(USB_SetupRequest_t * req);

/*
 * EXTERN DECLARATIONS
 */

extern const uint8_t cUSB_MTP_ConfigDescriptor[USB_MTP_CONFIG_DESC_SIZE];


#endif //USB_MTP_H
