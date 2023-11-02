# WDG
This provides control of the watchdog.

This will reset the processor if the watchdog is not checked periodically. This can be used to protect your firmware from an unexpected firmware lockup.

The header is available [here](../Lib/WDG.h).

# Usage

The used watchdog is the Independant Watchdog (IWDG). This is based off the LSI - even when an LSE is supplied. This means it cannot even be interrupted by clock disturbance.

The watchdog has a maximum period of ~14s that can be used.

```c
// Create a watchdog with a period of 50ms
WDG_Init(50);

while (1)
{
    ...
    // The watchdog must be kicked periodically or the MCU will reset.
    WDG_Kick();
    CORE_Idle();
}
```

*Once started, the WDG cannot be stopped*. The exception to this when compiled in debug, the WDG will be stopped when the debugger halts the core.

Note that the LSI is extremely innaccurate - so be sure that the WDG has at least a 20% margin of error.

# Board

The module is dependant on no definitions within `Board.h`