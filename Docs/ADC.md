# ADC
This module enables usage of the processors ADC converter.

This module is not very flexible, and will be subject to review in the future.

# Usage

`ADC_Read()` can be used to do an immediate blocking read of a specified ADC channel.

Refer to the datasheet for how the ADC channels map to the pins. Note that these pins must be left in analog mode. See [GPIO](GPIO.md) for more info.

```C
ADC_Init();
// ain will be in the range of 0 to ADC_MAX
uint32_t ain = ADC_Read(ADC_CHANNEL_0);
```

Helper functions are available to turn adc values into millivolts
```C
ADC_Init();
uint32_t ain = ADC_Read(ADC_CHANNEL_0);

// The value in millivolts directly at the pin.
uint32_t mv = AIN_AinToMv(ain);

// The value is millivolts at the input of a 47K/10K divider, as below:
// Input -> 47K -> Pin -> 10K -> GND
// The resistances do not have to be literal - just in ratio.
uint32_t mvi = AIN_AinToDivider(ain, 47, 10);
```

The internal channels are automatically configured with the following helpers
```C
ADC_Init();

int32_t degrees = ADC_ReadDieTemp();

// This computes the ADC reference voltage. Ideally this will be equal to ADC_VREF
uint32_t vref = ADC_ReadVRef();
```

# Board

The module is dependant on  definitions within `Board.h`
The following template can be used.
Commented out definitons are optional.

```C
// Module configuration
//#define ADC_VREF	3300
```