# CORE
This module provides control of the processors power states, and sets up the system tick.

The header is available [here](../Lib/Core.h).

> [!IMPORTANT]
> `CORE_Init()` should always be the first function run.

# Usage

## Init:
`CORE_Init()` performs a variety of housekeeping tasks
* Clock initialisation. See [CLK](CLK.md) for more info.
* Systick initialised
* All GPIO defaulted to analog mode. See [GPIO](GPIO.md) for more info.
* If enabled, the microseconds module will be initialised. See [US](US.md) for more info.

The main function should follow the following general template

```C
void main(void)
{
    CORE_Init();
    // System initialisation here
    ...

    while(1)
    {
        // Other responsibilities here
        ...
        CORE_Idle();
    }
}
```

## Sleep mode:
`CORE_Idle()` is just a wrapper for the WFI instruction. This puts the CPU into a low power state. This should be favored over busy waiting.

> [!TIP]  
> This function will return immediately when interrupts occurr, making it ideal for event driven systems.

> [!TIP]  
> SYSTICK is an interrupt source, so CORE_Idle will run for at most one millisecond (assuming `CORE_SYSTICK_FREQ` is 1000).

## Stop mode:

`CORE_Stop()` puts the CPU int an ultra-low power state, and suspends all high speed peripheral clocks. This should be used for low power applications.

> [!IMPORTANT]  
> The system tick is stalled during stop mode, so `CORE_GetTick()` cannot be used as a timebase when stopped. This also means stop mode may run indefinitely until an event occurrs.

> [!TIP]  
> Stop mode is normally used in conjunction with the [RTC](RTC.md) or a [LPTIM](LPTIM.md) to serve as a wakeup source.

> [!TIP]  
> Because SYSTICK is disabled, most peripherals will be stalled while in stop mode. Best practice is to deinitialise these peripherals before entering stop mode.

## System tick:

The system tick should be used for basic timekeeping

```C
CORE_Init();

uint32_t start = CORE_GetTick();
...
uint32_t elapsed = CORE_GetTick() - start;
// The elapsed time will be in milliseconds
```

Blocking delays can be based on the systick as below:
```c
CORE_Delay(100);
```

Events can be optionally piggybacked on the system tick. This can be a way to do small tasks at a reliable time. In this example `User_Callback` is a user defined function.

```C
CORE_Init();
CORE_OnTick( User_Callback );

while(1)
{
    CORE_Idle();
}
```

## Reset
`CORE_Reset()` triggers a NVIC_SystemReset(). This completely restarts the processor and all peripherals. The sole exception is the [RTC](RTC.md).

The source of the last reset can also be fetched using `CORE_GetResetSource()`.

> [!WARNING]  
> Subsequent calls to `CORE_GetResetSource` are not guaranteed to be valid. This should ideally be read once on boot.

# Board

The module is dependant on  definitions within `Board.h`
The following template can be used. Commented out definitions are optional.

```C
// CORE configuration
//#define CORE_USE_TICK_IRQ
//#define CORE_SYSTICK_FREQ   1000
```