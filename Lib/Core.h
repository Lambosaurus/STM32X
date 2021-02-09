#ifndef CORE_H
#define CORE_H

#include "STM32X.h"


//TODO:
// Implement HSE support options.
// Extract GetTick from HAL.

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

/*
 * EXTERN DECLARATIONS
 */

#endif //CORE_H
