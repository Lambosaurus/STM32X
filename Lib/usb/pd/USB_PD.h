#ifndef USB_PD_H
#define USB_PD_H

#include "STM32X.h"

#ifdef USB_PD

/*
 * FUNCTIONAL TESTING
 * STM32L0: N/A
 * STM32F0: N/A
 * STM32G0: N
 * STM32WL: N/A
 */

/*
 * PUBLIC DEFINITIONS
 */

/*
 * PUBLIC TYPES
 */

typedef enum {
	USB_PD_Flag_CC1 	= (1 << 0),
	USB_PD_Flag_CC2 	= (1 << 1),
	USB_PD_Flag_500mA 	= (0x1 << 2),
	USB_PD_Flag_1500mA 	= (0x2 << 2),
	USB_PD_Flag_3000mA 	= (0x3 << 2),
} USB_PD_Flag_t;

/*
 * PUBLIC FUNCTIONS
 */

// Initialisation
void USB_PD_Init(void);
void USB_PD_Deinit(void);

USB_PD_Flag_t USB_PD_Read(void);

/*
 * EXTERN DECLARATIONS
 */

#endif //USB_PD
#endif //USB_PD_H
