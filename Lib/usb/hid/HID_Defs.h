#ifndef HID_DEFS_H
#define HID_DEFS_H



/*
 *
 * https://www.usb.org/sites/default/files/hid1_11.pdf
 * https://www.usb.org/sites/default/files/hut1_22.pdf
 *
 * https://eleccelerator.com/tutorial-about-usb-hid-report-descriptors/
 *
 * https://wiki.osdev.org/USB_Human_Interface_Devices
 *
 */


/*
 * ITEM DEFINITIONS
 */

#define ITEM(tag)					(tag)
#define ITEM8(tag, x)				(tag | 0x01), (x)
#define ITEM16(tag, x)				(tag | 0x02), (((uint8_t)(x))), (((uint8_t)((x) >> 8)))
#define ITEM32(tag, x)				(tag | 0x03), (((uint8_t)(x))), (((uint8_t)((x) >> 8))), (((uint8_t)((x) >> 16))), (((uint8_t)((x) >> 24)))

/*
 * ITEM TAGS
 */


// Main items
#define	TAG_INPUT					0x80 // Argument are a bitmap of IO Options
#define	TAG_OUTPUT					0x90 // Argument are a bitmap of IO Options
#define	TAG_FEATURE					0xB0 // Argument are a bitmap of IO Options
#define	TAG_COLLECTION				0xA0 // Argument is a Collection
#define TAG_END_COLLECTION			0xC0

// Global items
#define	TAG_USAGE_PAGE				0x04 // Argument is a Usage Page
#define	TAG_USAGE					0x08 // Argument is a Usage
#define TAG_USAGE_MINIMUM			0x18 // Argument is a Usage
#define TAG_USAGE_MAXIMUM			0x28 // Argument is a Usage
#define	TAG_LOGICAL_MINIMUM			0x14
#define	TAG_LOGICAL_MAXIMUM			0x24
#define	TAG_PHYSICAL_MINIMUM		0x34
#define	TAG_PHYSICAL_MAXIMUM		0x44
#define TAG_UNIT_EXPONENT			0x54
#define TAG_UNIT					0x64
#define TAG_REPORT_SIZE				0x74
#define TAG_REPORT_ID				0x84
#define TAG_REPORT_COUNT			0x94
#define TAG_PUSH					0xA4
#define TAG_POP						0xB4


// Simplified macros for the above
#define INPUT(io)					ITEM8(TAG_INPUT, io)
#define OUTPUT(io)					ITEM8(TAG_OUTPUT, io)
#define FEATURE(io)					ITEM8(TAG_FEATURE, io)
#define COLLECTION(collection)		ITEM8(TAG_COLLECTION, collection)
#define END_COLLECTION()			ITEM(TAG_END_COLLECTION)

#define USAGE_PAGE(page)			ITEM8(TAG_USAGE_PAGE, page)
#define USAGE(usage)				ITEM8(TAG_USAGE, usage)
#define USAGE_MINIMUM(usage)		ITEM8(TAG_USAGE_MINIMUM, usage)
#define USAGE_MAXIMUM(usage)		ITEM8(TAG_USAGE_MAXIMUM, usage)
#define LOGICAL_MINIMUM(value)		ITEM8(TAG_LOGICAL_MINIMUM, value)
#define LOGICAL_MAXIMUM(value)		ITEM8(TAG_LOGICAL_MAXIMUM, value)
#define PHYSICAL_MINIMUM(value)		ITEM8(TAG_PHYSICAL_MINIMUM, value)
#define PHYSICAL_MAXIMUM(value)		ITEM8(TAG_PHYSICAL_MAXIMUM, value)
#define REPORT_COUNT(count)			ITEM8(TAG_REPORT_COUNT, count)
#define REPORT_SIZE(size)			ITEM8(TAG_REPORT_SIZE, size)
#define REPORT_ID(id)				ITEM8(TAG_REPORT_ID, id)

/*
 * IO OPTIONS
 */

#define IO_CONST					0x01 // Data vs Constant
#define IO_VARIABLE					0x02 // Arrray vs Variable
#define IO_RELATIVE					0x04 // Absolute vs Relative
#define IO_WRAP						0x08 // Non wrapped vs wrapped
#define IO_NON_LINEAR				0x10 // Linear vs Non linear
#define IO_NO_PREFERRED_STATE		0x20 // Preferred state vs non preferred
#define IO_NULL_STATE				0x40 // No null vs nullable
#define IO_VOLATILE					0x80 // Non volatile vs volatile
#define IO_BUFFERED_BYTES			0x100 // Bit field vs buffered bytes

/*
 * COLLECTION TYPES
 */

#define COLLECTION_PHYSICAL			0x00 // Physical grouped buttons or axes
#define COLLECTION_APPLICATION		0x01 // Mouse, keyboard
#define COLLECTION_LOGICAL			0x02 // Interrelated data
#define COLLECTION_REPORT			0x03 // A group of data in a single report
#define COLLECTION_NAMED_ARRAY		0x04
#define COLLECTION_USAGE_SWITCH		0x05
#define COLLECTION_USAGE_MODIFIER	0x06

/*
 * GENERIC PAGE USAGES
 */

#define PAGE_GENERIC_DESKTOP	0x01

#define USAGE_POINTER			0x01
#define USAGE_MOUSE				0x02
#define USAGE_JOYSTICK			0x04
#define USAGE_GAMEPAD			0x05
#define USAGE_KEYBOARD			0x06
#define USAGE_KEYPAD			0x07

#define USAGE_X					0x30
#define USAGE_Y					0x31
#define USAGE_Z					0x32
#define USAGE_RX				0x33
#define USAGE_RY				0x34
#define USAGE_RZ				0x35
#define USAGE_SLIDER			0x36
#define USAGE_DIAL				0x37
#define USAGE_WHEEL				0x38
#define USAGE_HAT_SWITCH		0x39


/*
 * KEYBOARD PAGE USAGES
 */

#define PAGE_KEYBOARD			0x07

/*
 * LED PAGE USAGES
 */

#define PAGE_LED				0x08

/*
 * BUTTON PAGE USAGES
 */

#define PAGE_BUTTON				0x09

// Individual buttons are simply enumerated.
// 0x00 is reserved for null button
// 0x01 is Left click / Primary button
// 0x02 is Right click / Secondary button
// 0x03 is Middle click


#endif //HID_DEFS_H
