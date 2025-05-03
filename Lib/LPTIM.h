#ifndef LPTIM_H
#define LPTIM_H

#include "STM32X.h"

/*
 * FUNCTIONAL TESTING
 * STM32L0: Y
 * STM32F0: N
 * STM32G0: Y
 * STM32WL: N
 */

/*
 * PUBLIC DEFINITIONS
 */

typedef struct {
	LPTIM_TypeDef * Instance;
#ifdef LPTIM_USE_IRQS
	VoidFunction_t ReloadCallback;
	VoidFunction_t PulseCallback;
#endif //TIM_USE_IRQS
} LPTIM_t;

/*
 * PUBLIC TYPES
 */

/*
 * PUBLIC FUNCTIONS
 */

// Initialisation
void LPTIM_Init(LPTIM_t * tim, uint32_t freq, uint32_t reload);
void LPTIM_Deinit(LPTIM_t * tim);
void LPTIM_SetFreq(LPTIM_t * tim, uint32_t freq);
void LPTIM_SetReload(LPTIM_t * tim, uint32_t reload);

// Base counter features
void LPTIM_Start(LPTIM_t * tim);
uint32_t LPTIM_Read(LPTIM_t * tim);

#ifdef LPTIM_USE_IRQS
void LPTIM_OnReload(LPTIM_t * tim, VoidFunction_t callback);
void LPTIM_OnPulse(LPTIM_t * tim, uint32_t value, VoidFunction_t callback);
void LPTIM_StopPulse(LPTIM_t * tim);
#endif //LPTIM_USE_IRQS

/*
 * EXTERN DECLARATIONS
 */

#ifdef LPTIM1_ENABLE
extern LPTIM_t * const LPTIM_1;
#endif
#ifdef LPTIM2_ENABLE
extern LPTIM_t * const LPTIM_2;
#endif

#endif //LPTIM_H
