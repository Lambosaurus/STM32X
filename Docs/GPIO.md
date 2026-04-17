# GPIO
This module allows basic IO functionality of the GPIO blocks.

The header is available [here](../Lib/GPIO.h).

# Usage

`GPIO_Init()` may be used directly, but using the alternative initialisers is reccommended.

> [!TIP]  
> `GPIO_State_t` is defined as `bool`. `GPIO_PIN_SET` and `true` can be used as readability requires.


## Output:
Below is a basic blinky example, demonstrating 1Hz blink on PA0.

```C
// Note that the initial state of the pin is set here
GPIO_EnableOutput(PA0, true);

while (1)
{
    GPIO_Write(PA0, true);
    CORE_Delay(500);
    GPIO_Write(PA0, false);
    CORE_Delay(500);
}
```

Inline functions for set/reset are available.

```c
GPIO_Set(PA0);
GPIO_Reset(PA0);

// Above an below are equivalent.

GPIO_Write(PA0, true);
GPIO_Write(PA0, false);
```

## Input:

The state of pins can be read once configured as an input.
```c
GPIO_EnableInput(PA0, GPIO_Pull_None);
bool state = GPIO_Read(PA0);
```

## Analog / Deinit:

Deinitialising a pin puts it back into High-Z (Analog) mode.

```c
GPIO_EnableOutput(PA0, true);
...
// Return GPIO to high-z once operation is completed.
GPIO_Deinit(PA0);
```

> [!TIP]  
> Its reccommended to deinitialize pins when they are not active. This minimizes noise and power consumption.

> [!TIP]  
> Some analog peripherals will require their affected pins to be in this mode, such as [ADC](ADC.md) and [COMP](COMP.md).

> [!NOTE]
> All GPIO are defaulted to Analog mode during [CORE_Init()](CORE.md)


## Combining pins:

Pins can be combined to affect multiple pins on the same GPIO block. This is reccommended where reasonable, as it takes no additional time or memory to affect pins in paralell.

```c
// configure 3 pins at once
GPIO_EnableInput(PA0 | PA1 | PA2, GPIO_Pull_None);
```

> [!WARNING]  
> Combining pins on different GPIO blocks will not work as intended. Ie, `PA0 | PB0`.

## Interrupts:

Interrupts occurr on a per channel basis. Each pin is on a channel corresponding to its pin number, ie, `GPIO_Pin_3` is on channel 3, and so on. These channels must be enabled within the `Board.h` file with the `#define GPIO_IRQx_ENABLE` definition.

```c
// Note that the pin must still be configured as an input before enabling the interrupt.
GPIO_EnableInput(PA0, GPIO_Pull_Up);
GPIO_OnChange(PA0, GPIO_IT_Falling, User_Callback);
```

> [!IMPORTANT]  
> Pins on the same channel will conflict, and cannot both be used for interrupts. For example `PA7` and `PB7` both use channel 7. Take care to consider this at the PCB level.

> [!WARNING]  
> `GPIO_OnChange` does not support combining pins.

> [!TIP]  
> To deinitialise a pin interrupt, you must call `GPIO_OnChange(PA0, GPIO_IT_None, NULL)` to remove the IRQ handler.

## Other configs:
Unorthodox IO configurations are possible using the raw GPIO_Init function. GPIO config flags are designed to be combined.

```c
// This configures PA0 an open drain output, with a pull up
GPIO_Init(PA0, GPIO_Mode_Output | GPIO_Speed_Medium | GPIO_Flag_OpenDrain | GPIO_Pull_Up);
GPIO_Write(PA0, false);
```

# Board


The module is dependant on  definitions within `Board.h`
The following template can be used. Commented out definitions are optional.

```C
// GPIO configuration
//#define GPIO_USE_IRQS
//#define GPIO_IRQ0_ENABLE
```
