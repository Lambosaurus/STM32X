
#include "USB.h"
#include "Core.h"

#include "usb_device.h"
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
	MX_USB_DEVICE_Init();
}

void USB_Deinit(void)
{

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
