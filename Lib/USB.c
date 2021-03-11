
#include "USB.h"
#include "Core.h"

#include "usb/USB_PCD.h"

#include "usb_device.h"
#include "usbd_core.h"
#include "usbd_desc.h"
#include "usbd_cdc.h"
#include "usbd_cdc_if.h"


/*
 * PRIVATE DEFINITIONS
 */

#if defined(STM32L0)

#elif defined(STM32F0)

#endif


/*
 * PRIVATE TYPES
 */

/*
 * PRIVATE PROTOTYPES
 */

/*
 * PRIVATE VARIABLES
 */

/*
 * PUBLIC FUNCTIONS
 */

void USB_Init(void)
{
	CORE_EnableUSBClock();

	if (USBD_Init(&hUsbDeviceFS, &FS_Desc, DEVICE_FS) != USBD_OK)
	{
		Error_Handler();
	}
	if (USBD_RegisterClass(&hUsbDeviceFS, &USBD_CDC) != USBD_OK)
	{
		Error_Handler();
	}
	if (USBD_CDC_RegisterInterface(&hUsbDeviceFS, &USBD_Interface_fops_FS) != USBD_OK)
	{
		Error_Handler();
	}
	USB_PCD_Start();

}

void USB_Deinit(void)
{
	USB_PCD_Stop();
}

void USB_Tx(const uint8_t * data, uint32_t count)
{
	CDC_Transmit_FS(data, count);
}

uint32_t USB_Rx(uint8_t * data, uint32_t size)
{
	return CDC_Read(data, size);
}

/*
 * PRIVATE FUNCTIONS
 */

/*
 * INTERRUPT ROUTINES
 */

void USB_IRQHandler(void)
{
	HAL_PCD_IRQHandler(&hpcd_USB_FS);
}
