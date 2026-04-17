
#include "USB.h"
#ifdef USB_ENABLE

#include "CLK.h"
#include "IRQ.h"
#include "usb/USB_PCD.h"
#include "usb/USB_CTL.h"


/*
 * PRIVATE DEFINITIONS
 */

#ifndef USB_IRQ_PRIO
#define USB_IRQ_PRIO	2
#endif

/*
 * PRIVATE TYPES
 */

/*
 * PRIVATE PROTOTYPES
 */

static void USBx_Init(void);
static void USBx_Deinit(void);

/*
 * PRIVATE VARIABLES
 */

/*
 * PUBLIC FUNCTIONS
 */

void USB_Init(void)
{
	CLK_EnableUSBCLK();
	USBx_Init();
	USB_PCD_Init();
	USB_CTL_Init();
	USB_PCD_Start();
}

void USB_Deinit(void)
{
	USB_PCD_Stop();
	USB_CTL_Deinit();
	USB_PCD_Deinit();
	USBx_Deinit();
	CLK_DisableUSBCLK();
}

bool USB_IsEnumerated(void)
{
	return USB_CTL_IsEnumerated();
}

/*
 * PRIVATE FUNCTIONS
 */

static void USBx_Init(void)
{
	__HAL_RCC_USB_CLK_ENABLE();
	IRQ_Enable(IRQ_No_USB, USB_IRQ_PRIO);
}

static void USBx_Deinit(void)
{
	IRQ_Disable(IRQ_No_USB);
	__HAL_RCC_USB_CLK_DISABLE();
}

/*
 * INTERRUPT ROUTINES
 */

#endif //USB_ENABLE
