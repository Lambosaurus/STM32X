
#include "WDG.h"
#include "CLK.h"

/*
 * PRIVATE DEFINITIONS
 */

#define IWDG_RELOAD_MAX		0xFFF

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

void WDG_Init(uint32_t period)
{
	IWDG->KR = IWDG_KEY_ENABLE;
	IWDG->KR = IWDG_KEY_WRITE_ACCESS_ENABLE;

	uint32_t freq = (IWDG_RELOAD_MAX * 1000) / period;
	IWDG->PR = CLK_SelectPrescalar( LSI_VALUE, 4, 256, &freq);

	// +1 to prevent timer being shorter than expected.
	uint32_t reload = (freq * period / 1000) + 1;
	if (reload > IWDG_RELOAD_MAX) { reload = IWDG_RELOAD_MAX; }

	IWDG->RLR = reload;

	while (IWDG->SR != 0x00); // Wait for the IWDG to start

	// Trigger a reload to kick things off.
	WDG_Kick();

#ifdef DEBUG
	// This prevents a watchdog timeout while halted under debug
	__HAL_DBGMCU_FREEZE_IWDG();
#endif
}

void WDG_Kick(void)
{
	IWDG->KR = IWDG_KEY_RELOAD;
}

/*
 * PRIVATE FUNCTIONS
 */

/*
 * INTERRUPT ROUTINES
 */
