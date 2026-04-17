#ifndef DMA_H
#define DMA_H

#include "STM32X.h"

/*
 * FUNCTIONAL TESTING
 * STM32L0: Y
 * STM32F0: N
 * STM32G0: N
 * STM32WL: N
 */

/*
 * PUBLIC DEFINITIONS
 */

typedef void (*DMA_Callback_t)(void * bfr, uint32_t index);

typedef struct {
	DMA_Channel_TypeDef * Instance;
	DMA_Callback_t callback;
	uint8_t index;
	struct {
		uint32_t size;
		uint32_t length;
		void * bfr;
	} data;
} DMA_t;


typedef enum {
	DMA_Mode_Normal		= DMA_NORMAL,
	DMA_Mode_Circular 	= DMA_CIRCULAR,

	DMA_Dir_Memory		= DMA_MEMORY_TO_MEMORY,
	DMA_Dir_ToPeriph	= DMA_MEMORY_TO_PERIPH,
	DMA_Dir_FromPeriph  = DMA_PERIPH_TO_MEMORY,

	DMA_MemSize_Byte		= DMA_MDATAALIGN_BYTE,
	DMA_MemSize_HalfWord	= DMA_MDATAALIGN_HALFWORD,
	DMA_MemSize_Word		= DMA_MDATAALIGN_WORD,

	DMA_PeriphSize_Byte		= DMA_PDATAALIGN_BYTE,
	DMA_PeriphSize_HalfWord = DMA_PDATAALIGN_HALFWORD,
	DMA_PeriphSize_Word		= DMA_PDATAALIGN_WORD
} DMA_Flags_t;

/*
 * PUBLIC TYPES
 */

void DMA_Init(DMA_t * dma, void * peripheral, void * bfr, uint32_t length, DMA_Flags_t flags, DMA_Callback_t callback);
void DMA_Deinit(DMA_t * dma);

void DMA_IRQHandler(DMA_t * dma);

/*
 * PUBLIC FUNCTIONS
 */

/*
 * EXTERN DECLARATIONS
 */

#ifdef DMA_CH1_ENABLE
extern DMA_t * const DMA_CH1;
#endif
#ifdef DMA_CH2_ENABLE
extern DMA_t * const DMA_CH2;
#endif
#ifdef DMA_CH3_ENABLE
extern DMA_t * const DMA_CH3;
#endif
#ifdef DMA_CH4_ENABLE
extern DMA_t * const DMA_CH4;
#endif
#ifdef DMA_CH5_ENABLE
extern DMA_t * const DMA_CH5;
#endif
#ifdef DMA_CH6_ENABLE
extern DMA_t * const DMA_CH6;
#endif
#ifdef DMA_CH7_ENABLE
extern DMA_t * const DMA_CH7;
#endif

#endif //DMA_H
