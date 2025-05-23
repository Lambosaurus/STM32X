#ifndef LPTIM_H
#define LPTIM_H

#include "STM32X.h"

#ifdef LPTIM_ENABLE

/*
 * FUNCTIONAL TESTING
 * STM32L0: Y
 * STM32F0: N
 * STM32G0: N
 * STM32WL: N
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
void LPTIM_Init(uint32_t freq, uint32_t reload);
void LPTIM_Deinit(void);
void LPTIM_SetFreq(uint32_t freq);
void LPTIM_SetReload(uint32_t reload);

// Base counter features
void LPTIM_Start(void);
uint32_t LPTIM_Read(void);

// IRQ features.
void LPTIM_OnReload(VoidFunction_t callback);
void LPTIM_OnCompare(uint32_t value, VoidFunction_t callback);
void LPTIM_StopCompare(void);

/*
 * EXTERN DECLARATIONS
 */

#endif //LPTIM_ENABLE
#endif //LPTIM_H
