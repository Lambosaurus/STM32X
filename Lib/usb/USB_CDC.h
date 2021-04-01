#ifndef USB_CDC_H
#define USB_CDC_H

#include "STM32X.h"

/*
 * FUNCTIONAL TESTING
 * STM32L0: N
 * STM32F0: N
 */

/*
 * PUBLIC DEFINITIONS
 */

#define USB_CDC_CLASSID				0x02
#define USB_CDC_SUBCLASSID			0x02
#define USB_CDC_PROTOCOLID			0x00

#define USB_CDC_CONFIG_DESC_SIZE	67
#define USB_CDC_CONFIG_DESC			cUSB_CDC_ConfigDescriptor

/*
 * PUBLIC TYPES
 */

/*
 * PUBLIC FUNCTIONS
 */

/*
 * EXTERN DECLARATIONS
 */

extern const uint8_t cUSB_CDC_ConfigDescriptor[USB_CDC_CONFIG_DESC_SIZE];

/*
 * DELETE LATER
 */

#include "usbd_def.h"

uint32_t USB_CDC_Read(uint8_t * data, uint32_t count);
void USB_CDC_Write(const uint8_t * data, uint32_t count);


#define CDC_IN_EP                                   0x81
#define CDC_OUT_EP                                  0x01
#define CDC_CMD_EP                                  0x82

#define CDC_BINTERVAL                          		0x10
#define CDC_PACKET_SIZE								USB_PACKET_SIZE
#define CDC_CMD_PACKET_SIZE                         8



#define CDC_SEND_ENCAPSULATED_COMMAND               0x00U
#define CDC_GET_ENCAPSULATED_RESPONSE               0x01U
#define CDC_SET_COMM_FEATURE                        0x02U
#define CDC_GET_COMM_FEATURE                        0x03U
#define CDC_CLEAR_COMM_FEATURE                      0x04U
#define CDC_SET_LINE_CODING                         0x20U
#define CDC_GET_LINE_CODING                         0x21U
#define CDC_SET_CONTROL_LINE_STATE                  0x22U
#define CDC_SEND_BREAK                              0x23U



extern USBD_ClassTypeDef  USBD_CDC;


#endif //USB_CDC_H
