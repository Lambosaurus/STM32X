
#include "DMA.h"

/*
 * PRIVATE DEFINITIONS
 */

#define DMAx	DMA1

/*
 * PRIVATE TYPES
 */

/*
 * PRIVATE PROTOTYPES
 */

static void DMAx_Init(DMA_t * dma);
static void DMAx_Deinit(DMA_t * dma);
static uint32_t DMA_GetMemSize(DMA_Flags_t flags);

/*
 * PRIVATE VARIABLES
 */

#ifdef DMA_CH1_ENABLE
static DMA_t gDMA_CH1 = {
	.Instance = DMA1_Channel1
};
DMA_t * DMA_CH1 = &gDMA_CH1;
#endif
#ifdef DMA_CH2_ENABLE
static DMA_t gDMA_CH2 = {
	.Instance = DMA1_Channel2
};
DMA_t * DMA_CH2 = &gDMA_CH2;
#endif
#ifdef DMA_CH3_ENABLE
static DMA_t gDMA_CH3 = {
	.Instance = DMA1_Channel3
};
DMA_t * DMA_CH3 = &gDMA_CH3;
#endif
#ifdef DMA_CH4_ENABLE
static DMA_t gDMA_CH4 = {
	.Instance = DMA1_Channel4
};
DMA_t * DMA_CH4 = &gDMA_CH4;
#endif
#ifdef DMA_CH5_ENABLE
static DMA_t gDMA_CH5 = {
	.Instance = DMA1_Channel5
};
DMA_t * DMA_CH5 = &gDMA_CH5;
#endif
#ifdef DMA_CH6_ENABLE
static DMA_t gDMA_CH6 = {
	.Instance = DMA1_Channel6
};
DMA_t * DMA_CH6 = &gDMA_CH6;
#endif
#ifdef DMA_CH7_ENABLE
static DMA_t gDMA_CH7 = {
	.Instance = DMA1_Channel7
};
DMA_t * DMA_CH7 = &gDMA_CH7;
#endif

/*
 * PUBLIC FUNCTIONS
 */


void DMA_Init(DMA_t * dma, void * peripheral, void * bfr, uint32_t length, DMA_Flags_t flags, DMA_Callback_t callback)
{
	DMAx_Init(dma);

	dma->callback = callback;
	dma->index = (((uint32_t)dma->Instance - (uint32_t)DMA1_Channel1) / ((uint32_t)DMA1_Channel2 - (uint32_t)DMA1_Channel1));

	MODIFY_REG( dma->Instance->CCR,
		  DMA_CCR_PL    | DMA_CCR_MSIZE  | DMA_CCR_PSIZE
		| DMA_CCR_MINC  | DMA_CCR_PINC   | DMA_CCR_CIRC
		| DMA_CCR_DIR   | DMA_CCR_MEM2MEM,

		DMA_PRIORITY_MEDIUM | DMA_MINC_ENABLE | DMA_PINC_DISABLE | flags
	);

	dma->Instance->CNDTR = length;
	dma->Instance->CPAR = (uint32_t)peripheral;
	dma->Instance->CMAR = (uint32_t)bfr;

	dma->data.size = DMA_GetMemSize(flags) * length;
	dma->data.length = length;
	dma->data.bfr = bfr;

	if (flags & DMA_Mode_Circular)
	{
		__HAL_DMA_ENABLE_IT(dma, (DMA_IT_TC | DMA_IT_HT));
	}
	else
	{
		// No half complete in normal mode.
		__HAL_DMA_DISABLE_IT(dma, DMA_IT_HT);
		__HAL_DMA_ENABLE_IT(dma, DMA_IT_TC);
	}

   __HAL_DMA_ENABLE(dma);
}

void DMA_Deinit(DMA_t * dma)
{
	__HAL_DMA_DISABLE(dma);
	DMAx_Deinit(dma);
}

/*
 * PRIVATE FUNCTIONS
 */

static uint32_t DMA_GetMemSize(DMA_Flags_t flags)
{
	if (flags & DMA_MemSize_Word)
	{
		return 4;
	}
	else if (flags & DMA_MemSize_HalfWord)
	{
		return 2;
	}
	else
	{
		return 1;
	}
}

static void DMAx_EnableIRQn(int n)
{
	if (n <= 1)
	{
		HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);
	}
	else if (n <= 3)
	{
		HAL_NVIC_EnableIRQ(DMA1_Channel2_3_IRQn);
	}
	else
	{
		HAL_NVIC_EnableIRQ(DMA1_Channel4_5_6_7_IRQn);
	}
}


static void DMAx_Init(DMA_t * dma)
{
	__DMA1_CLK_ENABLE();
	DMAx_EnableIRQn(dma->index);
}

static void DMAx_Deinit(DMA_t * dma)
{
	__DMA1_CLK_DISABLE();
}

static void DMA_IRQHandler(DMA_t * dma, uint32_t flag_it)
{
	uint32_t source_it = dma->Instance->CCR;
	uint32_t flag_pos = dma->index << 2;

	if ((flag_it & (DMA_FLAG_HT1 << flag_pos)) && (source_it & DMA_IT_HT))
	{
		// Clear half complete flag
		DMAx->IFCR = DMA_ISR_HTIF1 << flag_pos;

		if (dma->callback)
		{
			dma->callback(dma->data.bfr, dma->data.length / 2);
		}
	}
	else if ((flag_it & (DMA_FLAG_TC1 << flag_pos)) && (source_it & DMA_IT_TC))
	{
		bool circular = dma->Instance->CCR & DMA_CCR_CIRC;
		if(!circular)
		{
			// Disable the transfer complete and error interrupt
			__HAL_DMA_DISABLE_IT(dma, DMA_IT_TE | DMA_IT_HT | DMA_IT_TC);
		}
		// Clear the transfer complete flag
		DMAx->IFCR = DMA_ISR_TCIF1 << flag_pos;
		if (dma->callback)
		{
			if (circular)
			{
				dma->callback(dma->data.bfr + (dma->data.size/2), dma->data.length / 2);
			}
			else
			{
				dma->callback(dma->data.bfr, dma->data.length);

				// DMA should be re-initialised after a single shot.
				DMA_Deinit(dma);
			}
		}
	}
}

/*
 * INTERRUPT ROUTINES
 */
#if defined(DMA_CH1_ENABLE)
void DMA1_Channel1_IRQHandler(void)
{
	uint32_t flag_it = DMAx->ISR;
	DMA_IRQHandler(DMA_CH1, flag_it);
}
#endif //defined(DMA_CH1_ENABLE)

#if defined(DMA_CH2_ENABLE) || defined(DMA_CH3_ENABLE)
void DMA1_Channel2_3_IRQHandler(void)
{
	uint32_t flag_it = DMAx->ISR;
#ifdef DMA_CH2_ENABLE
	DMA_IRQHandler(DMA_CH2, flag_it);
#endif
#ifdef DMA_CH3_ENABLE
	DMA_IRQHandler(DMA_CH3, flag_it);
#endif
}
#endif //defined(DMA_CH1_ENABLE) || defined(DMA_CH2_ENABLE)

#if defined(DMA_CH4_ENABLE) || defined(DMA_CH5_ENABLE) || defined(DMA_CH6_ENABLE) || defined(DMA_CH7_ENABLE)
void DMA1_Channel4_5_6_7_IRQHandler(void)
{
	uint32_t flag_it = DMAx->ISR;
#ifdef DMA_CH4_ENABLE
	DMA_IRQHandler(DMA_CH4, flag_it);
#endif
#ifdef DMA_CH5_ENABLE
	DMA_IRQHandler(DMA_CH5, flag_it);
#endif
#ifdef DMA_CH6_ENABLE
	DMA_IRQHandler(DMA_CH6, flag_it);
#endif
#ifdef DMA_CH7_ENABLE
	DMA_IRQHandler(DMA_CH7, flag_it);
#endif
}
#endif

