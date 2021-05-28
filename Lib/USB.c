
#include "USB.h"
#ifdef USB_ENABLE

#include "CLK.h"
#include "usb/USB_PCD.h"
#include "usb/USB_CTL.h"

#ifdef USB_CLASS_CDC
#include "usb/cdc/USB_CDC.h"
#include <string.h>
#endif

/*
 * PRIVATE DEFINITIONS
 */

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

#ifdef USB_CLASS_CDC
void USB_Write(const uint8_t * data, uint32_t count)
{
	USB_CDC_Write(data, count);
}

uint32_t USB_Read(uint8_t * data, uint32_t size)
{
	return USB_CDC_Read(data, size);
}

void USB_WriteStr(const char * str)
{
	USB_CDC_Write((uint8_t *)str, strlen(str));
}
#endif //USB_CLASS_CDC

/*
 * PRIVATE FUNCTIONS
 */

static void USBx_Init(void)
{
	__HAL_RCC_USB_CLK_ENABLE();
	HAL_NVIC_SetPriority(USB_IRQn, 1, 1);
	HAL_NVIC_EnableIRQ(USB_IRQn);
}

static void USBx_Deinit(void)
{
	__HAL_RCC_USB_CLK_DISABLE();
	HAL_NVIC_DisableIRQ(USB_IRQn);
}

/*
 * INTERRUPT ROUTINES
 */

#endif //USB_ENABLE
