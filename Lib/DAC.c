
#include "DAC.h"

#ifdef DAC_ENABLE

/*
 * PRIVATE DEFINITIONS
 */

#define DAC_CR_POS(_ch, _cr)		((_cr) << ((_ch) * 16))

/*
 * PRIVATE TYPES
 */

/*
 * PRIVATE PROTOTYPES
 */

/*
 * PRIVATE VARIABLES
 */

static struct {
	DAC_Channel_t enabled;
} gDAC;

/*
 * PUBLIC FUNCTIONS
 */

void DAC_Init(DAC_Channel_t channel)
{
	if (gDAC.enabled == 0)
	{
		// Are we the first channel?
		__HAL_RCC_DAC1_CLK_ENABLE();
	}
	gDAC.enabled |= 1 << channel;

	DAC->CR |= DAC_CR_POS(channel, DAC_CR_EN1);
}

void DAC_Deinit(DAC_Channel_t channel)
{
	DAC->CR &= ~DAC_CR_POS(channel, DAC_CR_EN1);

	gDAC.enabled &= ~(1 << channel);
	if (gDAC.enabled == 0)
	{
		// Are we the last channel?
		__HAL_RCC_DAC1_CLK_DISABLE();
	}
}

void DAC_Write(DAC_Channel_t channel, uint32_t value)
{
	switch (channel)
	{
	case DAC_Channel_1:
		DAC->DHR12R1 = value;
		break;
	case DAC_Channel_2:
		DAC->DHR12R2 = value;
		break;
	}
}

/*
 * PRIVATE FUNCTIONS
 */

/*
 * INTERRUPT ROUTINES
 */

#endif //DAC_ENABLE

