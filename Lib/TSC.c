
#include "TSC.h"

#ifdef TSC_ENABLE
#include "GPIO.h"
#include "CLK.h"
#include "Core.h"

/*
 * PRIVATE DEFINITIONS
 */

/*
 * PRIVATE TYPES
 */

/*
 * PRIVATE PROTOTYPES
 */

/*
 * PRIVATE VARIABLES
 */

/*
 * PUBLIC FUNCTIONS
 */

void TSC_Init(uint32_t freq)
{
	uint32_t div = CLK_SelectDivider(freq);

	__HAL_RCC_TSC_CLK_ENABLE();

	TSC->CR = TSC_CR_TSCE;

	TSC->CR |= (TSC_CTPH_1CYCLE |
			 TSC_CTPL_1CYCLE |
			 (TSC_CTPL_1CYCLE << TSC_CR_SSD_Pos) |
			 TSC_SS_PRESC_DIV1 |
			 TSC_PG_PRESC_DIV1 |
			 TSC_MCV_4095 |
			 TSC_SYNC_POLARITY_FALLING |
			 TSC_ACQ_MODE_NORMAL |
			 TSC_CR_SSE);

	// Disable interrupts
	TSC->IER &= (~(TSC_IT_EOA | TSC_IT_MCE));

	// Clear flags
	TSC->ICR = (TSC_FLAG_EOA | TSC_FLAG_MCE);
}

void TSC_Deinit(void)
{
	TSC->IOHCR =
	TSC->IOCCR = 0;
	TSC->IOSCR = 0;
	TSC->IOGCSR = 0;
	__HAL_RCC_TSC_CLK_DISABLE();
}

void TSC_EnableChannel(TSC_Group_t group, TSC_Channel_t ch, GPIO_Pin_t pin)
{
	GPIO_EnableAlternate(pin, TSC_AF, GPIO_Flag_OpenDrain);

	uint32_t tsc_pin = (group * 4) + ch;

	TSC->IOHCR &= ~tsc_pin;
	TSC->IOCCR |= tsc_pin;
}

void TSC_EnableGroup(TSC_Group_t group, TSC_Channel_t sample_ch, GPIO_Pin_t pin)
{
	GPIO_EnableAlternate(pin, TSC_AF, 0);

	uint32_t tsc_pin = (group * 4) + ch;

	TSC->IOHCR &= ~tsc_pin;
	TSC->IOSCR |= tsc_pin;
	TSC->IOGCSR |= 1 << group;
}

/*
 * PRIVATE FUNCTIONS
 */

/*
 * INTERRUPT ROUTINES
 */

#endif //TSC_ENABLE
