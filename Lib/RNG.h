#ifndef RNG_H
#define RNG_H

#include "STM32X.h"

#ifdef RNG_ENABLE

/*
 * FUNCTIONAL TESTING
 * STM32L0: N
 * STM32F0: N
 * STM32G0: N
 * STM32WL: Y
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

uint32_t RNG_Read(void);
void RNG_ReadBytes(uint8_t * bfr, uint32_t size);

/*
 * EXTERN DECLARATIONS
 */

#endif //RNG_ENABLE
#endif //RNG_H
