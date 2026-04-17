
#include "IRQ.h"

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

void IRQ_Enable(IRQ_No_t irq, uint32_t priority)
{
	NVIC_EnableIRQ(irq);
	NVIC_SetPriority(irq, priority);
}

void IRQ_Disable(IRQ_No_t irq)
{
	NVIC_DisableIRQ(irq);
}

/*
 * PRIVATE FUNCTIONS
 */

/*
 * INTERRUPT ROUTINES
 */

#if defined(STM32L0)
#include "irq/IRQL0.inl.h"
#elif defined(STM32F0)
#include "irq/IRQF0.inl.h"
#elif defined(STM32G0)
#include "irq/IRQG0.inl.h"
#else
#error "IRQ support not added for this series"
#endif


