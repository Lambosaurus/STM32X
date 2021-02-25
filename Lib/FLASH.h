#ifndef FLASH_H
#define FLASH_H

#include "STM32X.h"

/*
 * FUNCTIONAL TESTING
 * STM32L0: N
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
void FLASH_WritePage(uint32_t page, const uint32_t * data);
void FLASH_ReadPage(uint32_t page, uint32_t * data);

/*
 * EXTERN DECLARATIONS
 */

#endif //FLASH_H
