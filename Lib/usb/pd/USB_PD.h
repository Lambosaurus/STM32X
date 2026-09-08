#ifndef USB_PD_H
#define USB_PD_H

#include "STM32X.h"

#ifdef USB_PD

/*
 * FUNCTIONAL TESTING
 * STM32L0: N/A
 * STM32F0: N/A
 * STM32G0: Y
 * STM32WL: N/A
 */

/*
 * PUBLIC DEFINITIONS
 */

/*
 * PUBLIC TYPES
 */

typedef enum {
	USB_PD_Flag_CC1 		= (1 << 0),
	USB_PD_Flag_CC2 		= (1 << 1),
	USB_PD_Flag_Negotiated 	= (1 << 2)
} USB_PD_Flag_t;

typedef void(*USB_PD_Callback_t)(USB_PD_Flag_t flag, uint32_t voltage, uint32_t current);

/*
 * PUBLIC FUNCTIONS
 */

// Initialisation
void USB_PD_Init(uint32_t voltage_limit);
void USB_PD_Deinit(void);

void USB_PD_OnChange(USB_PD_Callback_t cb);
USB_PD_Flag_t USB_PD_Read(uint32_t * voltage, uint32_t * current);
void USB_PD_Reset(void);

void USB_PD_IRQHandler(void);

/*
 * EXTERN DECLARATIONS
 */

#endif //USB_PD
#endif //USB_PD_H
