#ifndef USB_CLASS_H
#define USB_CLASS_H

#include "STM32X.h"

/*
 * FUNCTIONAL TESTING
 * STM32L0: N
 * STM32F0: N
 */

#if defined(USB_CLASS_CDC)
#include "USB_CDC.h"

#define USB_CLASS_CLASSID				USB_CDC_CLASSID
#define USB_CLASS_SUBCLASSID			USB_CDC_SUBCLASSID
#define USB_CLASS_PROTOCOLID			USB_CDC_PROTOCOLID

#define USB_CLASS_DEVICE_DESCRIPTOR 	USB_CDC_CONFIG_DESC

#else
#error "No USB Class defined"
#endif

/*
 * PUBLIC DEFINITIONS
 */

#define USB_CLASS_INIT(config)		(hUsbDeviceFS.pClass->Init(&hUsbDeviceFS, config))
#define USB_CLASS_DEINIT()			(hUsbDeviceFS.pClass->DeInit(&hUsbDeviceFS, 0))
#define USB_CLASS_SETUP(request) 	(hUsbDeviceFS.pClass->Setup(&hUsbDeviceFS, request))

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
