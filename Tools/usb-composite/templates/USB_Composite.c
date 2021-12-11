
#include "USB_Composite.h"

#ifdef USB_CLASS_COMPOSITE

${class_includes}

/*
 * PRIVATE DEFINITIONS
 */

/*
 * PRIVATE TYPES
 */

/*
 * PRIVATE PROTOTYPES
 */

/*
 * PRIVATE VARIABLES
 */

__ALIGNED(4) const uint8_t cUSB_Composite_ConfigDescriptor[USB_COMPOSITE_CONFIG_DESC_SIZE] =
{
${class_descriptor}
};

/*
 * PUBLIC FUNCTIONS
 */

void USB_Composite_Init(uint8_t config)
{
${class_init}
}

void USB_Composite_Deinit(void)
{
${class_deinit}
}

void USB_Composite_Setup(USB_SetupRequest_t * req)
{
	uint8_t interface = LOBYTE(req->wIndex);
	
${class_setup}
}

/*
 * PRIVATE FUNCTIONS
 */

#endif //USB_CLASS_COMPOSITE

