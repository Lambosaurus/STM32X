
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

static uint32_t TSC_GetGroup(TSC_Channel_t ch);
static void TSC_Sample(void);

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

void TSC_EnableCapacitor(TSC_Channel_t ch, GPIO_Pin_t pin)
{
	GPIO_EnableAlternate(pin, GPIO_Flag_OpenDrain, TSC_AF);
	TSC->IOHCR &= ~ch;

	TSC->IOSCR |= ch; // Select this as a sampling capacitor

}

void TSC_EnableInput(TSC_Channel_t ch, GPIO_Pin_t pin)
{
	// This function can enable multiple channels at once, so long as:
	//  - They are on the same GPIO bank
	GPIO_EnableAlternate(pin, GPIO_Flag_None, TSC_AF);
	TSC->IOHCR &= ~ch;
}

uint32_t TSC_Read(TSC_Channel_t ch)
{
	// This read function reads a single channel.
	// Reading multiple channels using multiple groups is possible, but not here.

	uint32_t group = TSC_GetGroup(ch);

	TSC->IOCCR = ch; // Select only this channel
	TSC->IOGCSR = 1 << group; // select only this group

	TSC_Sample();

	return TSC->IOGXCR[group];
}

/*
 * PRIVATE FUNCTIONS
 */

static uint32_t TSC_GetGroup(TSC_Channel_t ch)
{
	return FIRST_BIT_INDEX(ch) >> 2;
}

static void TSC_Sample(void)
{
	// Clear end of acquisition flag
	TSC->ICR |= TSC_FLAG_EOA | TSC_FLAG_MCE;

	// Start acquisition and wait for end
	TSC->CR |= TSC_CR_START;
	while (!(TSC->ISR & TSC_FLAG_EOA));
}

/*
 * INTERRUPT ROUTINES
 */

#endif //TSC_ENABLE
