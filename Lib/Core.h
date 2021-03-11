#ifndef CORE_H
#define CORE_H

#include "STM32X.h"

/*
 * FUNCTIONAL TESTING
 * STM32L0: Y
 * STM32F0: Y
 */

//TODO:
// Implement HSE support options.

/*
 * PUBLIC DEFINITIONS
 */

/*
 * PUBLIC TYPES
 */

/*
 * PUBLIC FUNCTIONS
 */

// Initialisation
void CORE_Init(void);

// Ticks & power control
void CORE_Idle(void);
void CORE_Delay(uint32_t ms);
uint32_t CORE_GetTick(void);

#ifdef USE_SYSTICK_IRQ
void CORE_OnTick(VoidFunction_t callback);
#endif

void CORE_EnableUSBClock(void);

/*
 * EXTERN DECLARATIONS
 */

#endif //CORE_H
