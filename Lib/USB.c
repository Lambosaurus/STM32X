
#include "USB.h"
#include "Core.h"

#include "usb/USB_PCD.h"

#include "usbd_core.h"
#include "usbd_desc.h"
#include "usb/USB_CDC.h"


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
	CORE_EnableUSBClock();
	USBx_Init();

	hUsbDeviceFS.pDesc = &FS_Desc;
	hUsbDeviceFS.dev_state = USBD_STATE_DEFAULT;
	hUsbDeviceFS.id = DEVICE_FS;
	hUsbDeviceFS.pClass = &USBD_CDC;
	hUsbDeviceFS.pData = &hpcd_USB_FS;

	USB_PCD_Init();

	// Need to reserve 8 bytes per EP-pair for the BTABLE, hence 0x18. Remaining endpoints need ep->maxpacket reserved.
	HAL_PCDEx_PMAConfig(&hpcd_USB_FS, 0x00, PCD_SNG_BUF, 0x18);
	HAL_PCDEx_PMAConfig(&hpcd_USB_FS, 0x80, PCD_SNG_BUF, 0x58);
	HAL_PCDEx_PMAConfig(&hpcd_USB_FS, 0x81, PCD_SNG_BUF, 0xC0);
	HAL_PCDEx_PMAConfig(&hpcd_USB_FS, 0x01, PCD_SNG_BUF, 0x110);
	HAL_PCDEx_PMAConfig(&hpcd_USB_FS, 0x82, PCD_SNG_BUF, 0x100);

	USB_PCD_Start();

}

void USB_Deinit(void)
{
	USB_PCD_Stop();
	USBx_Deinit();
}

void USB_Tx(const uint8_t * data, uint32_t count)
{
	USB_CDC_Tx(data, count);
}

uint32_t USB_Rx(uint8_t * data, uint32_t size)
{
	return USB_CDC_Rx(data, size);
}

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

void USB_IRQHandler(void)
{
	HAL_PCD_IRQHandler(&hpcd_USB_FS);
}
