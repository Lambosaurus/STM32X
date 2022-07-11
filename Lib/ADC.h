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

typedef enum {
	ADC_Channel_0 	= (1 << 0),
	ADC_Channel_1 	= (1 << 1),
	ADC_Channel_2 	= (1 << 2),
	ADC_Channel_3 	= (1 << 3),
	ADC_Channel_4 	= (1 << 4),
	ADC_Channel_5 	= (1 << 5),
	ADC_Channel_6 	= (1 << 6),
	ADC_Channel_7 	= (1 << 7),
	ADC_Channel_8 	= (1 << 8),
	ADC_Channel_9	= (1 << 9),
	ADC_Channel_10 	= (1 << 10),
	ADC_Channel_11 	= (1 << 11),
	ADC_Channel_12 	= (1 << 12),
	ADC_Channel_13 	= (1 << 13),
	ADC_Channel_14 	= (1 << 14),
	ADC_Channel_15 	= (1 << 15),
	ADC_Channel_16 	= (1 << 16),
	ADC_Channel_17 	= (1 << 17),
	ADC_Channel_18 	= (1 << 18),
} ADC_Channel_t;

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
uint32_t ADC_Read(ADC_Channel_t channel);

#ifdef ADC_DMA_CH
// Reading via DMA
void ADC_Start(ADC_Channel_t channel, uint16_t * buffer, uint32_t count, bool circular, ADC_Callback_t callback);
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
