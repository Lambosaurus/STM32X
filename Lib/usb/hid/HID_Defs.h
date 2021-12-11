#ifndef HID_DEFS_H
#define HID_DEFS_H



/*
 *
 * https://www.usb.org/sites/default/files/hid1_11.pdf
 * https://www.usb.org/sites/default/files/hut1_22.pdf
 */


/*
 * ITEM DEFINITIONS
 */

#define ITEM(tag)					(tag)
#define ITEM8(tag, x)				(tag | 0x01), (x)
#define ITEM16(tag, x)				(tag | 0x02), (((uint8_t)(x))), (((uint8_t)(x >> 8)))
#define ITEM32(tag, x)				(tag | 0x03), (((uint8_t)(x))), (((uint8_t)(x >> 8))), (((uint8_t)(x >> 16))), (((uint8_t)(x >> 24)))

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



#endif //HID_DEFS_H
