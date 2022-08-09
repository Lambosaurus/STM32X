#ifndef CRC_H
#define CRC_H

#include "STM32X.h"

/*
 * FUNCTIONAL TESTING
 * STM32L0: N
 * STM32F0: Y
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

uint32_t CRC32(uint32_t init, const uint32_t * words, uint32_t size);

#endif //CRC_H
