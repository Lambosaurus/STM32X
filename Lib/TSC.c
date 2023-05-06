
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
	//uint32_t div = CLK_SelectDivider(freq);
	uint32_t src_freq = CLK_GetHCLKFreq();
	uint32_t div = CLK_SelectPrescalar(src_freq, 1, 128, &freq);

	__HAL_RCC_TSC_CLK_ENABLE();

	TSC->CR = TSC_CR_TSCE;

	TSC->CR |= (TSC_CTPH_10CYCLES
			 | TSC_CTPL_10CYCLES
			 | (TSC_CTPL_1CYCLE << TSC_CR_SSD_Pos)
			 | TSC_SS_PRESC_DIV1
			 | (div * TSC_CR_PGPSC_0)
			 | TSC_MCV_4095
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
	GPIO_EnableAlternate(pin, TSC_AF, GPIO_Flag_None);

	uint32_t tsc_ch = (group << 2) | ch;

	TSC->IOHCR &= ~tsc_ch;
	TSC->IOCCR |= tsc_ch;
}

void TSC_EnableGroup(TSC_Group_t group, TSC_Channel_t sample_ch, GPIO_Pin_t sample_pin)
{
	GPIO_EnableAlternate(sample_pin, TSC_AF, GPIO_Flag_OpenDrain);

	uint32_t tsc_ch = (group << 2) | sample_ch;

	TSC->IOHCR &= ~tsc_ch;
	TSC->IOSCR |= tsc_ch;
	TSC->IOGCSR |= 1 << group;
}

void TSC_Sample(void)
{
	// Enable IO grounding (Required to discharge capacitor).
	//TSC->CR &= ~TSC_CR_IODEF;
	//CORE_Delay(100);
	// Disable IO grounding
	//TSC->CR |= TSC_CR_IODEF;

	// Clear end of acquisition flag
	TSC->ICR |= TSC_FLAG_EOA;

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
