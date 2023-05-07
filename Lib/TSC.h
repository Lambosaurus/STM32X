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

/*
 * PUBLIC TYPES
 */

typedef enum {
	TSC_Channel_1 = (1 << 0),
	TSC_Channel_2 = (1 << 1),
	TSC_Channel_3 = (1 << 2),
	TSC_Channel_4 = (1 << 3),
} TSC_Channel_t;

typedef enum {
	TSC_Group_1 = 0,
	TSC_Group_2,
	TSC_Group_3,
	TSC_Group_4,
	TSC_Group_5,
	TSC_Group_6,
	TSC_Group_7,
	TSC_Group_8,
} TSC_Group_t;

/*
 * PUBLIC FUNCTIONS
 */

void TSC_Init(void);
void TSC_Deinit(void);

void TSC_EnableChannel(TSC_Group_t group, TSC_Channel_t ch, GPIO_Pin_t pin);
void TSC_EnableGroup(TSC_Group_t group, TSC_Channel_t sample_ch, GPIO_Pin_t sample_pin);

void TSC_Sample(void);
uint32_t TSC_Read(TSC_Group_t group);

#endif //TSC_ENABLE
#endif //TSC_H

