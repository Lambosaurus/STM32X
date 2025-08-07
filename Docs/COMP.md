# COMP
This module enables usage of the integrated analog comparators.

The header is available [here](../Lib/COMP.h).

# Usage

## Basic:
When initialised, the comparator inputs must be specified.
Refer to the datasheet for how the comparator IO map to the pins.

> [!NOTE]  
> COMP pins should be left in analog mode. See [GPIO](GPIO.md) for more info.

```C
COMP_Init(COMP_1, COMP_Pos_IO1 | COMP_Neg_IO1);

if (COMP_Read(COMP_1))
{
    ...
}
```

## Interrupts:
The normal use case involves interrupts.

```C
COMP_Init(COMP_1, COMP_Pos_IO1 | COMP_Neg_IO1);
COMP_OnChange(COMP_1, GPIO_IT_Rising, User_Callback);
while(1)
{
    CORE_Idle();
}
```

> [!TIP]  
> The polarity of interrupts can be changed both by altering the polarity of the interrupt (changing `GPIO_IT_Rising` to `GPIO_IT_Falling`) and by inverting the comparator (including `COMP_Input_Inverted` in COMP_Init)

# Board

The module is dependant on  definitions within `Board.h`
The following template can be used.

```C
// COMP configuration
#define COMP1_ENABLE
```