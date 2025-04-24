# LPTIM
This module enables low-power timers.

It can be used as a counter or interrupt source from stop mode.

The header is available [here](../Lib/LPTIM.h).

# Usage

## Clock configuration

The LPTIM only has a basic prescalar, so the base frequency must be the LSO frequency divided by some power of two.
See [CLK](./CLK.md#low-speed-oscillators) for more info on the LSO source. If this condition is not met, a lower base frequency will be selected.

Currently this module is dependent on the LSO being already initialised. Please init the [RTC](./RTC.md) before starting the LPTIM.

## Basic usage:
```c
// Set up a 1.024KHz timer that reloads every 1024 ticks (1Hz)
LPTIM_Init(1024, 1023);
LPTIM_Start();

...

// Get the current value of the timer
uint32_t t = LPTIM_Read();
```

## Interrupts:
An interrupt can be triggered when the timer reloads, or on a specific compare value.

```c
LPTIM_Init(1024, 1023);
LPTIM_OnReload(User_OnReload); // Occurrs on the timer reload (1Hz)
LPTIM_OnCompare(511, User_OnCompare); // Occurrs 0.5s after the reload event.
LPTIM_Start();

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
#define LPTIM_ENABLE
```
