# IRQ
This module provides helpers for managing the interrupt request handlers.

The user should not interract with this module directly.

The header is available [here](../Lib/IRQ.h).

# Usage

Usage of the module functions are **NOT** expected of the user in normal circumstances. It intended for internal use by other libraries.

## IRQ numbers

This module defines the IRQ request numbers and handler callbacks. Note that an IRQ may be shared by multiple peripherals, so disabling an IRQ after enabling it is discouraged. 0 is the highest IRQ priority.

```c
// Enable the UART1 Interrupt handler with priority 0.
IRQ_Enable(IRQ_No_UART1, 0);
```

Note that priority control for shared interrupts will result in the last enabled peripheral setting the priority. This is poor behavior, but probably rare enough not to be a concern.

# Maintenance

The interrupt numbers and routines are setup in this module to be agnostic to the specific capabilites of the MCU series - however this work is not complete. This larger parts have more interrupt handlers and peripherals may not be fully supported.

To add support for these, please do the following:
1. Add the new `IRQ_No_t` handlers to the enumeration in [IRQ.h](../Lib/IRQ.h).
2. Modify the appropriate [irq source file](../Lib/irq/).
    * Add new the IRQ handlers
    * Use #define directives to ensure the name is correct when not all peripherals are present.


For example, the following IRQ handler may service both TIM3 and TIM4.

```c
#if defined(TIM_USE_IRQS) && (defined(TIM3_ENABLE) || defined(TIM4_ENABLE))
void TIM3_TIM4_IRQHandler(void)
{
#ifdef TIM3_ENABLE
	TIM_IRQHandler(TIM_3);
#endif
#ifdef TIM4_ENABLE
	TIM_IRQHandler(TIM_4);
#endif
}
#endif //defined(TIM_USE_IRQS) && (defined(TIM3_ENABLE) || defined(TIM4_ENABLE))
```

But on some series, TIM4 is not present, and so the handler name in the IRQ vector is different.
This is handled with the following:

```c
#ifndef TIM4
#define TIM3_TIM4_IRQHandler			TIM3_IRQHandler
#endif
```

For some peripherals, the name of the peripheral IRQ callback coincides with the name IRQ name. For example, `USB_IRQHandler` on the L0 series. In this case, we just allow this to be the IRQ handler, as this only occurrs when the IRQ is non-shared.
