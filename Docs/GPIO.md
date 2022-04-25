# GPIO
This module controls the processors GPIO blocks to control the pins

The header is available [here](../Lib/GPIO.h).

# Usage

`GPIO_Init()` may be used directly, but using the alternative initialisers is reccommended.

Note that `GPIO_State_t` is defined as `bool`. `GPIO_PIN_SET` and `true` can be used as readability requires.


## Output:
Below is a basic blinky example, demonstrating 1Hz blink on PA0.

```C
// Note that the initial state of the pin is set here
GPIO_EnableOutput(GPIOA, GPIO_PIN_0, true);

while (1)
{
    GPIO_Write(GPIOA, GPIO_PIN_0, true);
    CORE_Delay(500);
    GPIO_Write(GPIOA, GPIO_PIN_0, false);
    CORE_Delay(500);
}
```

Note that inline functions for set/reset are available.

```c
GPIO_Set(GPIOA, GPIO_PIN_0);
GPIO_Reset(GPIOA, GPIO_PIN_0);

// Above an below are equivalent.

GPIO_Write(GPIOA, GPIO_PIN_0, true);
GPIO_Write(GPIOA, GPIO_PIN_0, false);
```

## Input:

The state of pins can be read once configured as an input.
```c
GPIO_EnableInput(GPIOA, GPIO_PIN_0, GPIO_Pull_None);
bool state = GPIO_Read(GPIOA, GPIO_PIN_0);
```

## Analog / Deinit:

Note that all GPIO are defaulted to Analog mode during [CORE_Init()](CORE.md)

```c
GPIO_EnableOutput(GPIOA, GPIO_PIN_0, true);
...
// Return GPIO to high-z once operation is completed.
GPIO_Deinit(GPIOA, GPIO_PIN_0);
```

It is reccommended to deinit no-longer used pins:
* To prevent power leaking into unpowered devices
* To minimise noise and power consumption caused by floating pins

Some analog peripherals will require their affected pins to be in this mode, such as [ADC](ADC.md) and [COMP](COMP.md).

## Combining pins:

Note that pins can be combined to affect multiple pins on the same GPIO block. This is significantly faster.

```c
// configure 3 pins at once
GPIO_EnableInput(GPIOA, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2, GPIO_Pull_None);
```

This is reccommended where reasonable on all functions, with the exception of `GPIO_OnChange()`.

## Interrupts:

Interrupts occurr on a per channel basis. Each pin is on a channel corresponding to its pin number, ie, `GPIO_PIN_3` is on channel 3, and so on. Take care to enable each channel in `Board.h`, using `#define GPIO_IRQx_ENABLE`.

```c
// Note that the pin must still be configured as an input before enabling the interrupt.
GPIO_EnableInput(GPIOA, GPIO_PIN_0, GPIO_Pull_Up);
GPIO_OnChange(GPIOA, GPIO_PIN_0, GPIO_IT_Falling, User_Callback);
```

Pins on the same channel will conflict, and cannot both be used for interrupts. For example `PA7` and `PB7` both use channel 7, and will not funciton as intended. Take care to consider this at the PCB level.

## Other configs:
Unorthodox IO configurations are possible using the raw GPIO_Init function. GPIO config flags are designed to be combined.

```c
// This configures PA0 an open drain output, with a pull up
GPIO_Init(GPIOA, GPIO_PIN_0, GPIO_Mode_Output | GPIO_Speed_Medium | GPIO_Flag_OpenDrain | GPIO_Pull_Up);
GPIO_Write(GPIOA, GPIO_PIN_0, false);
```

# Board


The module is dependant on  definitions within `Board.h`
The following template can be used. Commented out definitions are optional.

```C
// GPIO configuration
//#define GPIO_USE_IRQS
//#define GPIO_IRQ0_ENABLE
```
