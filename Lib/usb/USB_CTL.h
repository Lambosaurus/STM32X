#ifndef USB_CTL_H
#define USB_CTL_H

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

void USB_CTL_Init(void);
void USB_CTL_Deinit(void);
void USB_CTL_Reset(void);

void USB_CTL_HandleSetup(uint8_t * data);
uint16_t USB_CTL_ToUnicode(uint8_t * unicode, char * str);
void USB_CTL_Error(void);

void USB_CTL_DataOutStage(uint8_t endpoint, uint8_t * data);
void USB_CTL_DataInStage(uint8_t endpoint, uint8_t * data);

void USB_CTL_Send(uint8_t * data, uint16_t size);
void USB_CTL_Receive(uint8_t * data, uint16_t size);

/*
 * PUBLIC FUNCTIONS
 */

/*
 * EXTERN DECLARATIONS
 */

#endif //USB_CTL_H