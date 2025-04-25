#ifndef DAC_H
#define DAC_H

#include "STM32X.h"

#ifdef DAC_ENABLE

/*
 * FUNCTIONAL TESTING
 * STM32L0: N
 * STM32F0: N
 * STM32G0: Y
 * STM32WL: N
 */

/*
 * PUBLIC DEFINITIONS
 */

#define DAC_MAX		4095

/*
 * PUBLIC TYPES
 */

typedef enum {
	DAC_Channel_1 	= 0,
	DAC_Channel_2 	= 1,
} DAC_Channel_t;

/*
 * PUBLIC FUNCTIONS
 */

void DAC_Init(DAC_Channel_t channel);
void DAC_Deinit(DAC_Channel_t channel);

// Output
void DAC_Write(DAC_Channel_t channel, uint32_t value);

/*
 * EXTERN DECLARATIONS
 */

#endif //DAC_ENABLE
#endif //DAC_H
