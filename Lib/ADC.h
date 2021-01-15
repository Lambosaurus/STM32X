#ifndef ADC_H
#define ADC_H

#include "Board.h"

/*
 * PUBLIC DEFINITIONS
 */

#define ADC_VREF	3300
#define ADC_MAX		4095

/*
 * PUBLIC TYPES
 */

/*
 * PUBLIC FUNCTIONS
 */

void ADC_Init(void);
void ADC_Deinit(void);
uint32_t ADC_Read(uint32_t channel);

uint32_t AIN_AinToMv(uint32_t ain);
uint32_t AIN_AinToDivider(uint32_t ain, uint32_t rlow, uint32_t rhigh);

#endif //ADC_H
