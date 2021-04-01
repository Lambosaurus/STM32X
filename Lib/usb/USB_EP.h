#ifndef USB_EP_H
#define USB_EP_H

#include "STM32X.h"

/*
 * FUNCTIONAL TESTING
 * STM32L0: N
 * STM32F0: N
 */

/*
 * PUBLIC DEFINITIONS
 */


/*
 * PUBLIC TYPES
 */

#define USB_EP_TYPE_NONE		0
#define USB_EP_TYPE_CTRL		1
#define USB_EP_TYPE_BULK		2
#define USB_EP_TYPE_INTR		3
#define USB_EP_TYPE_ISOC		4


/*
 * PUBLIC FUNCTIONS
 */

void USB_EP_Init(void);
void USB_EP_Deinit(void);
void USB_EP_Reset(void);

void USB_EP_Open(uint8_t endpoint, uint8_t type, uint16_t size);
void USB_EP_Close(uint8_t endpoint);
bool USB_EP_IsOpen(uint8_t endpoint);
void USB_EP_Read(uint8_t endpoint, uint8_t *data, uint32_t count);
void USB_EP_Write(uint8_t endpoint, const uint8_t * data, uint32_t count);
void USB_EP_Stall(uint8_t endpoint);
void USB_EP_Destall(uint8_t endpoint);
bool USB_EP_IsStalled(uint8_t endpoint);

// TODO: Delete this when able.
uint32_t USB_EP_RxCount(uint8_t endpoint);

void USB_EP_IRQHandler(void);

// TODO: Hide these when able
void USB_EP_Activate(USB_EPTypeDef *ep);
void USB_EP_Deactivate(USB_EPTypeDef *ep);

/*
 * EXTERN DECLARATIONS
 */

#endif //USB_EP_H
