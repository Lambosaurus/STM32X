# DAC
This module enables usage of DAC outputs.

The header is available [here](../Lib/DAC.h).

# Usage

Refer to the datasheet for how the DAC channels map to the pins.

> [!NOTE]  
> DAC pins should be left in analog mode. See [GPIO](GPIO.md) for more info.

Once initialised, the DAC channels become output. These are written with a 12 bit value, ie 0 to `DAC_MAX`.

```C
DAC_Init(DAC_Channel_1);
DAC_Write(DAC_Channel_1, 2048); // Set to half value.
```

# Board

The module is dependant on  definitions within `Board.h`
The following template can be used.
Commented out definitons are optional.

```C
// DAC configuration
#define DAC_ENABLE
```
