#ifndef CORE_H
#define CORE_H

#include "STM32X.h"

/*
 * FUNCTIONAL TESTING
 * STM32L0: Y
 * STM32F0: Y
 * STM32G0: Y
 */

/*
 * PUBLIC DEFINITIONS
 */

/*
 * PUBLIC TYPES
 */

typedef enum {
	CORE_ResetSource_Standby,
	CORE_ResetSource_Watchdog,
	CORE_ResetSource_Software,
	CORE_ResetSource_PowerOn,
	CORE_ResetSource_Pin,
	CORE_ResetSource_UNKNOWN,
} CORE_ResetSource_t;

/*
 * PUBLIC FUNCTIONS
 */

// Initialisation
void CORE_Init(void);

// Ticks & power control
void CORE_Idle(void);
void CORE_Stop(void);
void CORE_Delay(uint32_t ms);
static inline uint32_t CORE_GetTick(void);

// Core reset
void CORE_Reset(void);
CORE_ResetSource_t CORE_GetResetSource(void);

#ifdef CORE_USE_TICK_IRQ
void CORE_OnTick(VoidFunction_t callback);
#endif

/*
 * EXTERN DECLARATIONS
 */

#include "Core.inl.h"

#endif //CORE_H
