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

/*
 * PUBLIC TYPES
 */

/*
 * PUBLIC FUNCTIONS
 */

void USB_PCD_Init(void);
void USB_PCD_Start(void);
void USB_PCD_Stop(void);
void USB_PCD_Deinit(void);

/*
 * EXTERN DECLARATIONS
 */

// DELETE THIS
#include  "usbd_def.h"
extern USBD_HandleTypeDef hUsbDeviceFS;
extern PCD_HandleTypeDef hpcd_USB_FS;

HAL_StatusTypeDef USB_PCD_EP_ISR_Handler(PCD_HandleTypeDef *hpcd);
void USB_PCD_SetAddress(uint8_t address);

#endif //USB_PCD_H
