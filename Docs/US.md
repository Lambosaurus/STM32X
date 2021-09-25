# US
This provides a standardised microsecond timer.

The intent of this is so that other modules can rely on a standard interface for microseconds, as this is a common requirement. This does not reflect a physical peripheral.

The capabilities of this module are dependant on whether you are willing to dedicate a timer to it. See [TIM](TIM.md) for more info on timers.

# Usage

`US_Init()` should not be called by the user. It will be initialised within `CORE_Init()`, see [CORE](CORE.md) for more info.

## Without a timer


## With a timer

When a timer is used, this module can be used for microsecond time keeping

```c
uint32_t start = US_Read();
...
uint32_t end = US_Read();

// Note the use of the subtract to get the difference.
// This automatically handles wrapping.
uint32_t elapsed = US_Subtract(start, end);
```


# Board

The module is dependant on definitions within `Board.h`
The following template can be used. Commented out definitions are optional.

```C
// US Config
//#define US_RES     1
//#define US_TIM     TIM_22
//#define TIM22_ENABLE
```