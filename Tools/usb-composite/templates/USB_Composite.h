#ifndef USB_COMPOSITE_H
#define USB_COMPOSITE_H

#include "STM32X.h"
#include "usb/USB_Defs.h"

/*
 * PUBLIC DEFINITIONS
 */

#define USB_COMPOSITE_INTERFACES			${interfaces}
#define USB_COMPOSITE_ENDPOINTS				${endpoints}

#define USB_COMPOSITE_CONFIG_DESC_SIZE		${descriptor_size}
#define USB_COMPOSITE_CONFIG_DESC			cUSB_Composite_ConfigDescriptor

#define USB_COMPOSITE_CLASSID				${class_id}
#define USB_COMPOSITE_SUBCLASSID			${subclass_id}
#define USB_COMPOSITE_PROTOCOLID			${protocol_id}

${definitions}

/*
 * PUBLIC TYPES
 */

/*
 * PUBLIC FUNCTIONS
 */

// Callbacks for USB_CTL.
// These should be referenced in USB_Class.h
void USB_Composite_Init(uint8_t config);
void USB_Composite_Deinit(void);
void USB_Composite_Setup(USB_SetupRequest_t * req);

/*
 * EXTERN DECLARATIONS
 */

extern const uint8_t cUSB_Composite_ConfigDescriptor[USB_COMPOSITE_CONFIG_DESC_SIZE];


#endif // USB_COMPOSITE_H
