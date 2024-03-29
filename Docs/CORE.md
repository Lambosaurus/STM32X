# CORE
This module provides control of the processors power states, and sets up the system tick.

`CORE_Init()` should **always** be the first function run.

The header is available [here](../Lib/Core.h).

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

## Sleep modes:
`CORE_Idle()` is just a wrapper for the WFI instruction.
* This will return immediately when interrupts occurr, making it ideal for event driven systems
* Systick is an interrupt, so this will return at least every millisecond (assuming `CORE_SYSTICK_FREQ` is 1000), making it ideal for polled systems.
* This functions use is optional, but its encouraged: as it reduces the power consumption of compared to a busy wait.
* All clocks and peripherals are left running as expected.

`CORE_Stop()` is similar to CORE_Idle, but with the following adjustments:
* Systick is disabled. Note that this means CORE_GetTick() will not reflect the passed time.
* SYSCLK is disabled. This means that most clocked peripherals will stop functioning. See the datasheet on STOP mode for more information.
* Stop mode should ideally be used in conjuction with RTC module and its alarms. See [RTC](RTC.md) for more information.

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

The source of the last reset can also be fetched using `CORE_GetResetSource()`. If used, this should be called once on boot directly after `CORE_Init()`. Subsequent calls may be invalid.

# Board

The module is dependant on  definitions within `Board.h`
The following template can be used. Commented out definitions are optional.

```C
// CORE configuration
//#define CORE_USE_TICK_IRQ
//#define CORE_SYSTICK_FREQ   1000
```