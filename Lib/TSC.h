#ifndef TSC_H
#define TSC_H

#include "STM32X.h"

#ifdef TSC_ENABLE
#include "GPIO.h"

/*
 * FUNCTIONAL TESTING
 * STM32L0: N
 */

/*
 * PUBLIC DEFINITIONS
 */

#define TSC_CHANNEL(group, ch)		((1 << ch) << (group << 2))

/*
 * PUBLIC TYPES
 */

typedef enum {
	TSC_G1_Channel1 	= TSC_CHANNEL(0, 0),
	TSC_G1_Channel2		= TSC_CHANNEL(0, 1),
	TSC_G1_Channel3 	= TSC_CHANNEL(0, 2),
	TSC_G1_Channel4		= TSC_CHANNEL(0, 3),
	TSC_G2_Channel1 	= TSC_CHANNEL(1, 0),
	TSC_G2_Channel2		= TSC_CHANNEL(1, 1),
	TSC_G2_Channel3 	= TSC_CHANNEL(1, 2),
	TSC_G2_Channel4		= TSC_CHANNEL(1, 3),
	TSC_G3_Channel1 	= TSC_CHANNEL(2, 0),
	TSC_G3_Channel2		= TSC_CHANNEL(2, 1),
	TSC_G3_Channel3 	= TSC_CHANNEL(2, 2),
	TSC_G3_Channel4		= TSC_CHANNEL(2, 3),
	TSC_G4_Channel1 	= TSC_CHANNEL(3, 0),
	TSC_G4_Channel2		= TSC_CHANNEL(3, 1),
	TSC_G4_Channel3 	= TSC_CHANNEL(3, 2),
	TSC_G4_Channel4		= TSC_CHANNEL(3, 3),
	TSC_G5_Channel1 	= TSC_CHANNEL(4, 0),
	TSC_G5_Channel2		= TSC_CHANNEL(4, 1),
	TSC_G5_Channel3 	= TSC_CHANNEL(4, 2),
	TSC_G5_Channel4		= TSC_CHANNEL(4, 3),
	TSC_G6_Channel1 	= TSC_CHANNEL(5, 0),
	TSC_G6_Channel2		= TSC_CHANNEL(5, 1),
	TSC_G6_Channel3 	= TSC_CHANNEL(5, 2),
	TSC_G6_Channel4		= TSC_CHANNEL(5, 3),
	TSC_G7_Channel1 	= TSC_CHANNEL(6, 0),
	TSC_G7_Channel2		= TSC_CHANNEL(6, 1),
	TSC_G7_Channel3 	= TSC_CHANNEL(6, 2),
	TSC_G7_Channel4		= TSC_CHANNEL(6, 3),
	TSC_G8_Channel1 	= TSC_CHANNEL(7, 0),
	TSC_G8_Channel2		= TSC_CHANNEL(7, 1),
	TSC_G8_Channel3 	= TSC_CHANNEL(7, 2),
	TSC_G8_Channel4		= TSC_CHANNEL(7, 3),
} TSC_Channel_t;

/*
 * PUBLIC FUNCTIONS
 */

void TSC_Init(void);
void TSC_Deinit(void);

void TSC_EnableCapacitor(TSC_Channel_t ch, GPIO_Pin_t pin);
void TSC_EnableInput(TSC_Channel_t ch, GPIO_Pin_t pin);

uint32_t TSC_Read(TSC_Channel_t ch);

#endif //TSC_ENABLE
#endif //TSC_H

