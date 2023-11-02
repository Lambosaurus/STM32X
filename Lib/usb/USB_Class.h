#ifndef USB_CLASS_H
#define USB_CLASS_H

#include "STM32X.h"

/*
 * PUBLIC DEFINITIONS
 */

#if defined(USB_CLASS_COMPOSITE)
#include "USB_Composite.h"

#define USB_CLASS_CLASSID				USB_COMPOSITE_CLASSID
#define USB_CLASS_SUBCLASSID			USB_COMPOSITE_SUBCLASSID
#define USB_CLASS_PROTOCOLID			USB_COMPOSITE_PROTOCOLID

#define USB_CLASS_INIT(config)			USB_Composite_Init(config)
#define USB_CLASS_DEINIT()				USB_Composite_Deinit()
#define USB_CLASS_SETUP(request) 		USB_Composite_Setup(request)

#define USB_CONFIG_DESCRIPTOR 			USB_COMPOSITE_CONFIG_DESC
#define USB_ENDPOINTS					USB_COMPOSITE_ENDPOINTS
#define USB_INTERFACES					USB_COMPOSITE_INTERFACES

#elif defined(USB_CLASS_CDC)
#include "cdc/USB_CDC.h"

#define USB_CLASS_CLASSID				USB_CDC_CLASSID
#define USB_CLASS_SUBCLASSID			USB_CDC_SUBCLASSID
#define USB_CLASS_PROTOCOLID			USB_CDC_PROTOCOLID

#define USB_CLASS_INIT(config)			USB_CDC_Init(config)
#define USB_CLASS_DEINIT()				USB_CDC_Deinit()
#define USB_CLASS_SETUP(request) 		USB_CDC_Setup(request)

#define USB_CONFIG_DESCRIPTOR 			USB_CDC_CONFIG_DESC
#define USB_ENDPOINTS					USB_CDC_ENDPOINTS
#define USB_INTERFACES					USB_CDC_INTERFACES

#elif defined(USB_CLASS_MSC)
#include "msc/USB_MSC.h"

#define USB_CLASS_INIT(config)			USB_MSC_Init(config)
#define USB_CLASS_DEINIT()				USB_MSC_Deinit()
#define USB_CLASS_SETUP(request) 		USB_MSC_Setup(request)

#define USB_CONFIG_DESCRIPTOR 			USB_MSC_CONFIG_DESC
#define USB_ENDPOINTS					USB_MSC_ENDPOINTS
#define USB_INTERFACES					USB_MSC_INTERFACES

#elif defined(USB_CLASS_HID)
#include "hid/USB_HID.h"

#define USB_CLASS_INIT(config)			USB_HID_Init(config)
#define USB_CLASS_DEINIT()				USB_HID_Deinit()
#define USB_CLASS_SETUP(request) 		USB_HID_Setup(request)

#define USB_CONFIG_DESCRIPTOR 			USB_HID_CONFIG_DESC
#define USB_ENDPOINTS					USB_HID_ENDPOINTS
#define USB_INTERFACES					USB_HID_INTERFACES

#elif defined(USB_CLASS_MTP)
#include "mtp/USB_MTP.h"

#define USB_CLASS_INIT(config)			USB_MTP_Init(config)
#define USB_CLASS_DEINIT()				USB_MTP_Deinit()
#define USB_CLASS_SETUP(request) 		USB_MTP_Setup(request)

#define USB_CONFIG_DESCRIPTOR 			USB_MTP_CONFIG_DESC
#define USB_ENDPOINTS					USB_MTP_ENDPOINTS
#define USB_INTERFACES					USB_MTP_INTERFACES

#else
#error "No USB Class defined"
#endif

/*
 * PUBLIC TYPES
 */

/*
 * PUBLIC FUNCTIONS
 */

/*
 * EXTERN DECLARATIONS
 */

#endif //USB_CLASS_H
