
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

#define CTL_IN_EP		0x80
#define CTL_OUT_EP		0x00

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

	// Initialise the ctrl endpoints
	USB_PCD_EP_Open(CTL_IN_EP, USBD_EP_TYPE_CTRL, USB_PACKET_SIZE, false);
	USB_PCD_EP_Open(CTL_OUT_EP, USBD_EP_TYPE_CTRL, USB_PACKET_SIZE, false);

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
