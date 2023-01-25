
#include "WDG.h"

#ifdef IWDG_ENABLE

/*
 * PRIVATE DEFINITIONS
 */

// Note, if the prescalar is adjusted, so must the freq
#define IWDG_PRESCALAR		IWDG_PRESCALER_128
#define IWDG_FREQUENCY		(LSI_VALUE / 128)
#define IWDG_RELOAD_MAX		0xFFF

// Using div 128, this gives a max period of ~14s
// And a resolution of ~3ms

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

	IWDG->PR = IWDG_PRESCALAR;

	// +1 to prevent timer being shorter than expected.
	uint32_t reload = (IWDG_FREQUENCY * period / 1000) + 1;
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

#endif //IWDG_ENABLE
