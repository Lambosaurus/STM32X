
#include "USB_PCD.h"

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

void USB_PCD_Start(void)
{
	// Enable interrupt sources
	USB->CNTR = (uint16_t)(
			USB_CNTR_CTRM  | USB_CNTR_WKUPM |
            USB_CNTR_SUSPM | USB_CNTR_ERRM |
            USB_CNTR_SOFM | USB_CNTR_ESOFM |
            USB_CNTR_RESETM | USB_CNTR_L1REQM
			);
	// Enable DP/DM pullups
	USB->BCDR |= (uint16_t)USB_BCDR_DPPU;
}

void USB_PCD_Stop(void)
{
	USB->CNTR = 0;

	// Disable DP/DM pullups
	USB->BCDR &= (uint16_t)(~(USB_BCDR_DPPU));
}


/*
 * PRIVATE FUNCTIONS
 */

/*
 * INTERRUPT ROUTINES
 */
