#ifndef USB_PCD_H
#define USB_PCD_H

#include "STM32X.h"


/*
 * FUNCTIONAL TESTING
 * STM32L0: N
 * STM32F0: N
 */

/*
 * PUBLIC DEFINITIONS
 */

#ifdef USB_SPEED_FULL
#define USB_PACKET_SIZE		512
#else
#define USB_PACKET_SIZE		64
#endif

/*
 * PUBLIC TYPES
 */

/*
 * PUBLIC FUNCTIONS
 */

void USB_PCD_Start(void);
void USB_PCD_Stop(void);

/*
 * EXTERN DECLARATIONS
 */

// DELETE THIS
#include  "usbd_ioreq.h"
extern USBD_HandleTypeDef hUsbDeviceFS;


#endif //USB_PCD_H
