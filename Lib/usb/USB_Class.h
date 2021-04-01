#ifndef USB_CLASS_H
#define USB_CLASS_H

#include "STM32X.h"

/*
 * FUNCTIONAL TESTING
 * STM32L0: N
 * STM32F0: N
 */

#ifdef USE_USB_CDC
#include "USB_CDC.h"
#endif

/*
 * PUBLIC DEFINITIONS
 */

#define USB_CLASS_INIT(cfg)			(hUsbDeviceFS.pClass->Init(&hUsbDeviceFS, cfg))

#define USB_CLASS_DEINIT()		\
	if (hUsbDeviceFS.pClassData)\
	{\
		hUsbDeviceFS.pClass->DeInit(&hUsbDeviceFS, hUsbDeviceFS.dev_config);\
	}\

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
