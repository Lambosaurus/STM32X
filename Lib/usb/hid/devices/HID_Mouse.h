#ifndef HID_MOUSE_H
#define HID_MOUSE_H

#include "STM32X.h"

/*
 * PUBLIC DEFINITIONS
 */

#define USB_HID_REPORT_SIZE					3

#define USB_HID_REPORT_DESC_SIZE			50

#define USB_HID_REPORT_DESC_BODY					\
	USAGE_PAGE(PAGE_GENERIC_DESKTOP),				\
	USAGE(USAGE_MOUSE),								\
	COLLECTION(COLLECTION_APPLICATION),				\
		USAGE(USAGE_POINTER),						\
		COLLECTION(COLLECTION_PHYSICAL),			\
													\
			/* 3 buttons */							\
			USAGE_PAGE(PAGE_BUTTON),				\
			USAGE_MINIMUM(1),						\
			USAGE_MAXIMUM(3),						\
			LOGICAL_MINIMUM(0),						\
			LOGICAL_MAXIMUM(1),						\
			REPORT_COUNT(3),						\
			REPORT_SIZE(1),							\
			INPUT(IO_VARIABLE),						\
													\
			/* 5 pad bits */						\
			REPORT_COUNT(1),						\
			REPORT_SIZE(5),							\
			INPUT(IO_CONST | IO_VARIABLE),			\
													\
			/* An X,Y relative position */			\
			USAGE_PAGE(PAGE_GENERIC_DESKTOP),		\
			USAGE(USAGE_X),							\
			USAGE(USAGE_Y),							\
			LOGICAL_MINIMUM(-127),					\
			LOGICAL_MAXIMUM(127),					\
			REPORT_SIZE(8),							\
			REPORT_COUNT(2),						\
			INPUT(IO_VARIABLE | IO_RELATIVE),		\
													\
		END_COLLECTION(),							\
	END_COLLECTION(),								\

/*
 * PUBLIC TYPES
 */

typedef struct __attribute((packed)) {
	uint8_t buttons;
	int8_t x;
	int8_t y;
} USB_HID_Report_t;

/*
 * PUBLIC FUNCTIONS
 */

/*
 * EXTERN DECLARATIONS
 */

#endif //HID_MOUSE_H
