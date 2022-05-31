#ifndef ADC_H
#define ADC_H

#include "STM32X.h"

/*
 * FUNCTIONAL TESTING
 * STM32L0: Y
 * STM32F0: Y
 * STM32G0: N
 * STM32WL: N
 */

/*
 * PUBLIC DEFINITIONS
 */

#ifndef ADC_VREF
#define ADC_VREF			3300
#endif
#define ADC_MAX				4095


/*
 * PUBLIC TYPES
 */

typedef void (*ADC_Callback_t)(uint16_t * samples, uint32_t size);

/*
 * PUBLIC FUNCTIONS
 */

void ADC_Init(void);
void ADC_Deinit(void);

// Configuration
uint32_t ADC_SetFreq(uint32_t target);

#ifdef STM32L0
void ADC_SetOversampling(uint32_t ratio);
#endif

// Reading
uint32_t ADC_Read(uint32_t channel);

#ifdef ADC_DMA_CH
// Reading via DMA
void ADC_Start(uint32_t channel, uint16_t * buffer, uint32_t count, bool circular, ADC_Callback_t callback);
void ADC_Stop(void);
#endif // ADC_DMA_CH

// Special channels
int32_t ADC_ReadDieTemp(void);
uint32_t ADC_ReadVRef(void);

// Utility
uint32_t AIN_AinToMv(uint32_t ain);
uint32_t AIN_AinToDivider(uint32_t ain, uint32_t rlow, uint32_t rhigh);

/*
 * EXTERN DECLARATIONS
 */

#endif //ADC_H
