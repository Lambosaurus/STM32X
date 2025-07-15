#ifndef TIM_H
#define TIM_H

#include "STM32X.h"
#include "GPIO.h"

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

/*
 * PUBLIC TYPES
 */

typedef struct {
	TIM_TypeDef * Instance;
#ifdef TIM_USE_IRQS
	VoidFunction_t ReloadCallback;
	VoidFunction_t PulseCallback[4];
#endif //TIM_USE_IRQS
} TIM_t;

typedef enum {
	TIM_CH1 = 0,
	TIM_CH2 = 1,
	TIM_CH3 = 2,
	TIM_CH4 = 3,
} TIM_Channel_t;

/*
 * PUBLIC FUNCTIONS
 */

// Initialisation
void TIM_Init(TIM_t * tim, uint32_t freq, uint32_t reload);
void TIM_Deinit(TIM_t * tim);
void TIM_SetFreq(TIM_t * tim, uint32_t freq);
void TIM_SetReload(TIM_t * tim, uint32_t reload);

// Base counter features
void TIM_Start(TIM_t * tim);
void TIM_Stop(TIM_t * tim);
static inline uint32_t TIM_Read(TIM_t * tim);

// Channel features
void TIM_SetPulse(TIM_t * tim, TIM_Channel_t ch, uint32_t pulse);

#ifdef TIM_USE_IRQS
void TIM_OnReload(TIM_t * tim, VoidFunction_t callback);
void TIM_OnPulse(TIM_t * tim, TIM_Channel_t ch, VoidFunction_t callback);
void TIM_IRQHandler(TIM_t * tim);
#endif //TIM_USE_IRQS

void TIM_EnablePwm(TIM_t * tim, TIM_Channel_t ch, GPIO_Pin_t pins, uint32_t af);

/*
 * EXTERN DECLARATIONS
 */

#ifdef TIM1_ENABLE
extern TIM_t * const TIM_1;
#endif
#ifdef TIM2_ENABLE
extern TIM_t * const TIM_2;
#endif
#ifdef TIM3_ENABLE
extern TIM_t * const TIM_3;
#endif
#ifdef TIM5_ENABLE
extern TIM_t * const TIM_5;
#endif
#ifdef TIM6_ENABLE
extern TIM_t * const TIM_6;
#endif
#ifdef TIM14_ENABLE
extern TIM_t * const TIM_14;
#endif
#ifdef TIM15_ENABLE
extern TIM_t * const TIM_15;
#endif
#ifdef TIM16_ENABLE
extern TIM_t * const TIM_16;
#endif
#ifdef TIM17_ENABLE
extern TIM_t * const TIM_17;
#endif
#ifdef TIM21_ENABLE
extern TIM_t * const TIM_21;
#endif
#ifdef TIM22_ENABLE
extern TIM_t * const TIM_22;
#endif

#include "TIM.inl.h"

#endif //TIM_H
