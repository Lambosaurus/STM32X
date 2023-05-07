
#include "TSC.h"

#ifdef TSC_ENABLE
#include "GPIO.h"
#include "CLK.h"
#include "Core.h"

/*
 * PRIVATE DEFINITIONS
 */

#define TSC_FREQ_TARGET		1000000
#define TSC_CHARGE_CYCLES	4
#define TSC_CYCLES_MAX		TSC_MCV_8191

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

void TSC_Init(void)
{
	uint32_t src_freq = CLK_GetHCLKFreq();
	uint32_t freq = TSC_FREQ_TARGET;
	uint32_t div = CLK_SelectPrescalar(src_freq, 1, 128, &freq);

	__HAL_RCC_TSC_CLK_ENABLE();

	TSC->CR = TSC_CR_TSCE;

	TSC->CR |= (
			   (TSC_CTPH_2CYCLES * (TSC_CHARGE_CYCLES - 1))
			 | (TSC_CTPL_2CYCLES * (TSC_CHARGE_CYCLES - 1))
			 | (TSC_CTPL_1CYCLE << TSC_CR_SSD_Pos)
			 | TSC_SS_PRESC_DIV1
			 | (div * TSC_CR_PGPSC_0)
			 | TSC_CYCLES_MAX
			 | TSC_SYNC_POLARITY_FALLING
			 | TSC_ACQ_MODE_NORMAL
			 // | TSC_CR_SSE
			 );

	// Disable interrupts
	TSC->IER &= (~(TSC_IT_EOA | TSC_IT_MCE));
}

void TSC_Deinit(void)
{
	TSC->IOHCR = 0xFFFFFFFF;
	TSC->IOCCR = 0;
	TSC->IOSCR = 0;
	TSC->IOGCSR = 0;
	__HAL_RCC_TSC_CLK_DISABLE();
}

void TSC_EnableChannel(TSC_Group_t group, TSC_Channel_t ch, GPIO_Pin_t pin)
{
	// This function can enable multiple channels at once, so long as:
	//  - They are on the same group
	//  - They are on the same GPIO bank
	GPIO_EnableAlternate(pin, GPIO_Flag_None, TSC_AF);

	uint32_t tsc_ch = ch << (group << 2);

	TSC->IOHCR &= ~tsc_ch;
	TSC->IOCCR |= tsc_ch;
}

void TSC_EnableGroup(TSC_Group_t group, TSC_Channel_t sample_ch, GPIO_Pin_t sample_pin)
{
	GPIO_EnableAlternate(sample_pin, GPIO_Flag_OpenDrain, TSC_AF);

	uint32_t tsc_ch = sample_ch << (group << 2);

	TSC->IOHCR &= ~tsc_ch;
	TSC->IOSCR |= tsc_ch;
	TSC->IOGCSR |= 1 << group;
}

void TSC_Sample(void)
{
	// Clear end of acquisition flag
	TSC->ICR |= TSC_FLAG_EOA | TSC_FLAG_MCE;

	// Start acquisition and wait for end
	TSC->CR |= TSC_CR_START;
	while (!(TSC->ISR & TSC_FLAG_EOA));

}

uint32_t TSC_Read(TSC_Group_t group)
{
	return TSC->IOGXCR[group];
}

/*
 * PRIVATE FUNCTIONS
 */

/*
 * INTERRUPT ROUTINES
 */

#endif //TSC_ENABLE
