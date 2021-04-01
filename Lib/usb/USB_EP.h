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

/*
 * PUBLIC FUNCTIONS
 */

void USB_EP_Init(void);
void USB_EP_Deinit(void);

void USB_EP_Open(uint8_t endpoint, uint8_t type, uint16_t size);
void USB_EP_Close(uint8_t endpoint);
void USB_EP_Rx(uint8_t endpoint, uint8_t *data, uint32_t count);
void USB_EP_Tx(uint8_t endpoint, const uint8_t * data, uint32_t count);
void USB_EP_Stall(uint8_t endpoint);
void USB_EP_Destall(uint8_t endpoint);

void USB_EP_IRQHandler(void);

// TODO: Hide these when able
void USB_PMA_Write(uint16_t address, uint8_t * data, uint16_t count);
void USB_PMA_Read(uint16_t address, uint8_t * data, uint16_t count);

/*
 * EXTERN DECLARATIONS
 */

#endif //USB_EP_H
