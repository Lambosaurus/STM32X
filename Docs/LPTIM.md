# LPTIM
This module enables low-power timers.

It can be used as a counter or interrupt source from stop mode.

The header is available [here](../Lib/LPTIM.h).

# Usage

Warning - the IRQn names for the LPTIM are probably shared with different peripherals for many STM parts.
This will be improved in future, but right now you'll need to manually confirm these are correct.

## Clock configuration

The LPTIM only has a basic prescalar, so the base frequency must be the LSO frequency divided by some power of two.
See [CLK](./CLK.md#low-speed-oscillators) for more info on the LSO source. If this condition is not met, a lower base frequency will be selected.

Currently this module is dependent on the LSO being already initialised. Please init the [RTC](./RTC.md) before starting the LPTIM.

## Basic usage:
```c
// Set up a 1.024KHz timer that reloads every 1024 ticks (1Hz)
LPTIM_Init(LPTIM_1, 1024, 1023);
LPTIM_Start(LPTIM_1);

...

// Get the current value of the timer
uint32_t t = LPTIM_Read(LPTIM_1);
```

## Interrupts:
An interrupt can be triggered when the timer reloads, or on a specific pulse value.

```c
LPTIM_Init(LPTIM_1, 1024, 1023);
LPTIM_OnReload(LPTIM_1,  ser_OnReload); // Occurrs on the timer reload (1Hz)
LPTIM_OnPulse(LPTIM_1, 511, User_OnPulse); // Occurrs 0.5s after the reload event.
LPTIM_Start(LPTIM_1);

while (1)
{
    CORE_Stop();
}
```

# Board
The module is dependant on definitions within `Board.h`
The following template can be used. Commented out definitions are optional.

```c
// LPTIM config
#define LPTIM1_ENABLE
//#define LPTIM_USE_IRQS
```
