#ifndef USB_CTL_H
#define USB_CTL_H

#include "STM32X.h"

/*
 * PUBLIC DEFINITIONS
 */

/*
 * PUBLIC TYPES
 */

/*
 * PUBLIC FUNCTIONS
 */

void USB_CTL_Init(void);
void USB_CTL_Deinit(void);

bool USB_CTL_IsEnumerated(void);

// Called by status requests on EP0
void USB_CTL_HandleSetup(uint8_t * data);

// Required by classes to reply to EP0 requests
void USB_CTL_Send(const uint8_t * data, uint16_t size);
void USB_CTL_Receive(uint8_t * data, uint16_t size, VoidFunction_t callback);
void USB_CTL_Error(void);

/*
 * EXTERN DECLARATIONS
 */

#endif //USB_CTL_H
