#ifndef USB_MSC_H
#define USB_MSC_H

#include "STM32X.h"
#include "../USB_Defs.h"
#include "USB_MSC_Types.h"

/*
 * FUNCTIONAL TESTING
 * STM32L0: Y
 * STM32F0: N
 */

/*
 * PUBLIC DEFINITIONS
 */

#define USB_MSC_CLASSID				0x00
#define USB_MSC_SUBCLASSID			0x00
#define USB_MSC_PROTOCOLID			0x00

#define USB_MSC_CONFIG_DESC_SIZE	32
#define USB_MSC_CONFIG_DESC			cUSB_MSC_ConfigDescriptor



// DELETE THIS
#define USBD_BOT_IDLE                      0U       /* Idle state */
#define USBD_BOT_DATA_OUT                  1U       /* Data Out state */
#define USBD_BOT_DATA_IN                   2U       /* Data In state */
#define USBD_BOT_LAST_DATA_IN              3U       /* Last Data In Last */
#define USBD_BOT_SEND_DATA                 4U       /* Send Immediate data */
#define USBD_BOT_NO_DATA                   5U       /* No data Stage */

#define MSC_IN_EP                        0x81
#define MSC_OUT_EP                       0x01

#define USBD_CSW_CMD_PASSED                0x00U
#define USBD_CSW_CMD_FAILED                0x01U

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

void USB_MSC_SetStorage(USB_MSC_Storage_t * storage);

// Delete this
void MSC_BOT_SendCSW(uint8_t CSW_Status);

/*
 * EXTERN DECLARATIONS
 */

extern const uint8_t cUSB_MSC_ConfigDescriptor[USB_MSC_CONFIG_DESC_SIZE];


#endif //USB_CDC_H
