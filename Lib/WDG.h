#ifndef WDG_H
#define WDG_H

#include "STM32X.h"

/*
 * FUNCTIONAL TESTING
 * STM32L0: Y
 * STM32F0: N
 */

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
// Note, enabling the IWDG will force the LSI on.
void WDG_Init(uint32_t period);

void WDG_Kick(void);


/*
 * EXTERN DECLARATIONS
 */

#endif //WDG_H
