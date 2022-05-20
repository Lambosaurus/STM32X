#ifndef US_H
#define US_H

#include "STM32X.h"

/*
 * FUNCTIONAL TESTING
 * STM32L0: Y
 * STM32F0: N
 * STM32G0: N
 */

/*
 * PUBLIC DEFINITIONS
 */

#ifdef US_TIM
#define US_ENABLE
// Micros functionality is greatly extended when a timer can be dedicated to it.

#define US_TIM_MAX				0xFFFF

// Its assumed that only a 16 bit timer can be dedicated.
// Longer intervals can be measured by increasing the steps.
#ifndef US_RES
#define US_RES					1
#endif

#define US_MAX					(US_TIM_MAX * US_RES)

// Helper function for taking differences
#define US_Subtract(now, last)	((now - last) & US_MAX)
#endif

/*
 * PUBLIC TYPES
 */

/*
 * PUBLIC FUNCTIONS
 */

#ifdef US_ENABLE

// Do not call the US_Init. This will be done in CORE_Init();
void US_Init(void);
uint32_t US_Read(void);

#endif

// Even if the timer is not used, we still provide a way to delay in microseconds.
// Its just not guaranteed to be as accurate.
void US_Delay(uint32_t us);

/*
 * EXTERN DECLARATIONS
 */

#endif //US_H
