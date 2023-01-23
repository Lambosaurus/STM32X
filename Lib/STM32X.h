#ifndef STM32X_H
#define STM32X_H

// Common includes
#include <stdint.h>
#include <stdbool.h>
#include <string.h>


// Include user board file.
#include "Board.h"

// Include appropriate HAL layer
#if defined(STM32L0)
#include "stm32l0xx_hal.h"
#elif defined(STM32F0)
#include "stm32f0xx_hal.h"
#elif defined(STM32F4)
#include "stm32f4xx_hal.h"
#elif defined(STM32G0)
#include "stm32g0xx_hal.h"
#elif defined(STM32WL)
#include "stm32wlxx_hal.h"
#else
#error "STM family not defined"
#endif

/*
 * PUBLIC DEFINITIONS
 */

#define LENGTH(x)		(sizeof(x) / sizeof(*(x)))

#define CRITICAL_SECTION_BEGIN() uint32_t primask_bit= __get_PRIMASK();\
  __disable_irq()

#define CRITICAL_SECTION_END()  __set_PRIMASK(primask_bit)

#define FIRST_BIT_INDEX(x) 		(__builtin_ffs(x) - 1)

#define IS_POWER_OF_2(x)		(!((x - 1) & x))

/*
 * PUBLIC TYPES
 */

typedef void(*VoidFunction_t)(void);

/*
 * PUBLIC FUNCTIONS
 */

/*
 * EXTERN DECLARATIONS
 */

#endif //STM32X_H
