# US
This provides a standardised microsecond timer.

The intent of this is so that other modules can rely on a standard interface for microseconds, as this is a common requirement. This does not reflect a physical peripheral.

The capabilities of this module are dependant on whether you are willing to dedicate a timer to it.

# Usage

`US_Init()` should not be called by the user. It will be initialised within `CORE_Init()`, see [CORE](CORE.md) for more info.

## Minimal mode:

When no timer is provided, only `US_Delay()` is available. This is provided using a calibrated loop, and so it not highly accurate. It will typically run 4us longer than requested on a Cortex M0+ core @ 32MHz. This is still suitable for many uses.

## Timer mode:

When a timer is used, this module can be used for accurate time keeping.

The timer is defined as `US_TIM` within `Board.h` See [TIM](TIM.md) for more info on timers.

```c
uint32_t start = US_Read();
...
uint32_t end = US_Read();

// Note the use of the subtract to get the difference.
// This automatically handles wrapping.
uint32_t elapsed = US_Subtract(start, end);
```

`US_Delay()` will be based on the timer, and have the same accuracy.

Note that as the timer is expected to be 16 bit, this limits the maximum intervals that can be measured. This interval can be extended by decreasing the microsecond resolution. With `US_RES` defined as `1` an interval of `65535` us can be measured. Increasing this step size multiplies this as the cost of accuracy.

# Board

The module is dependant on definitions within `Board.h`
The following template can be used. Commented out definitions are optional.

```C
// US Config
//#define US_RES     1
//#define US_TIM     TIM_22
//#define TIM22_ENABLE
```