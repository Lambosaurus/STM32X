#ifndef ADC_H
#define ADC_H

#include "Board.h"

/*
 * EXAMPLE BOARD DEFINITION
 */

/*
//#define ADC_VREF	3300
 */

/*
 * PUBLIC DEFINITIONS
 */

#ifndef ADC_VREF
#define ADC_VREF	3300
#endif
#define ADC_MAX		4095

/*
 * PUBLIC TYPES
 */

/*
 * PUBLIC FUNCTIONS
 */

// Initialisation
void ADC_Init(void);
void ADC_Deinit(void);

// Reading
uint32_t ADC_Read(uint32_t channel);

// Utility
uint32_t AIN_AinToMv(uint32_t ain);
uint32_t AIN_AinToDivider(uint32_t ain, uint32_t rlow, uint32_t rhigh);

/*
 * EXTERN DECLARATIONS
 */

#endif //ADC_H
