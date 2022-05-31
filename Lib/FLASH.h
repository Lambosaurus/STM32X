#ifndef FLASH_H
#define FLASH_H

#include "STM32X.h"

/*
 * FUNCTIONAL TESTING
 * STM32L0: N
 * STM32F0: Y
 * STM32G0: Y
 * STM32WL: N
 */

/*
 * PUBLIC DEFINITIONS
 */

// Refer to FLASH_PAGE_SIZE for page dimensions.

/*
 * PUBLIC TYPES
 */

/*
 * PUBLIC FUNCTIONS
 */

uint32_t FLASH_GetPageCount(void);
const uint32_t * FLASH_GetPage(uint32_t page);

// The erase is expected to target a page start address
void FLASH_Erase(const uint32_t * address);
// Take care with writes: if not aligned to the series flash word sizes, they may be lost.
void FLASH_Write(const uint32_t * address, const uint32_t * data, uint32_t size);

/*
 * EXTERN DECLARATIONS
 */

#endif //FLASH_H
