#ifndef TIM_H
#define TIM_H

#include "STM32X.h"
#include "GPIO.h"


/*
 * PUBLIC DEFINITIONS
 */

/*
 * PUBLIC TYPES
 */

typedef void(*VoidFunction_t)(void);

typedef struct {
	TIM_TypeDef * Instance;
#ifdef USE_TIM_IRQS
	VoidFunction_t ReloadCallback;
	VoidFunction_t PulseCallback[4];
#endif //USE_TIM_IRQS
} TIM_t;


/*
 * PUBLIC FUNCTIONS
 */

// Initialisation
void TIM_Init(TIM_t * tim, uint32_t frequency, uint32_t reload);
void TIM_Deinit(TIM_t * tim);
void TIM_SetFreq(TIM_t * tim, uint32_t freq);
void TIM_SetReload(TIM_t * tim, uint32_t reload);

// Base counter features
void TIM_Start(TIM_t * tim);
void TIM_Stop(TIM_t * tim);
static inline uint32_t TIM_GetCounter(TIM_t * tim);

// Channel features
void TIM_SetPulse(TIM_t * tim, uint32_t ch, uint32_t pulse);

#ifdef USE_TIM_IRQS
void TIM_OnReload(TIM_t * tim, VoidFunction_t callback);
void TIM_OnPulse(TIM_t * tim, uint32_t ch, VoidFunction_t callback);
#endif //USE_TIM_IRQS

void TIM_EnablePwm(TIM_t * tim, uint32_t ch, GPIO_t * gpio, uint32_t pin, uint32_t af);


/*
 * EXTERN DECLARATIONS
 */

#ifdef USE_TIM1
extern TIM_t * TIM_1;
#endif
#ifdef USE_TIM2
extern TIM_t * TIM_2;
#endif
#ifdef USE_TIM3
extern TIM_t * TIM_3;
#endif
#ifdef USE_TIM14
extern TIM_t * TIM_14;
#endif
#ifdef USE_TIM16
extern TIM_t * TIM_16;
#endif
#ifdef USE_TIM17
extern TIM_t * TIM_17;
#endif
#ifdef USE_TIM21
extern TIM_t * TIM_21;
#endif
#ifdef USE_TIM22
extern TIM_t * TIM_22;
#endif

#include "TIM.inl"

#endif //TIM_H
