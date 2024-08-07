# ADC
This module enables usage of the processors ADC converter.

The header is available [here](../Lib/ADC.h).

# Usage

`ADC_Read()` can be used to do an immediate blocking read of a specified ADC channel.
Note that the ST CUBE ADC channel definitons should no longer be used. Use the enumeration found in `ADC.h`.

Refer to the datasheet for how the ADC channels map to the pins. Note that these pins must be left in analog mode. See [GPIO](GPIO.md) for more info.

```C
ADC_Init();
// ain will be in the range of 0 to ADC_MAX
uint32_t ain = ADC_Read(ADC_Channel_0);
```

Helper functions are available to turn adc values into millivolts
```C
ADC_Init();
uint32_t ain = ADC_Read(ADC_Channel_0);

// The value in millivolts directly at the pin.
uint32_t mv = AIN_AinToMv(ain);

// The value is millivolts at the input of a 47K/10K divider, as below:
// Input -> 47K -> Pin -> 10K -> GND
// The resistances do not have to be literal - just in ratio.
uint32_t mvi = AIN_AinToDivider(ain, 10, 47);
```

The internal channels are automatically configured with the following helpers
```C
ADC_Init();

int32_t degrees = ADC_ReadDieTemp();

// This computes the ADC reference voltage. Ideally this will be equal to ADC_VREF
uint32_t vref = ADC_ReadVRef();
```

## Configuration

The ADC can be used 'as is', but more control is often required, especially when used in DMA mode.

```C
ADC_Init();

// The actual frequency will be returned
uint32_t frequency = ADC_SetFreq(23121);

// This sets a 2x oversampling ratio - effectively halving the sample rate.
// This automatically configured the output shift to keep the samples in the 12 bit range.
// This must be a power of 2.
ADC_SetOversampling(2);
```

When configuring the frequency, note that only a specific list of frequencies are available. In the STM32L0 these are:
```  
	285714 hz
	250000 hz
	200000 hz
	160000 hz
	125000 hz
	 76923 hz
	 43478 hz
	 23121 hz
```

These are dependant on the ADC clocking configuration, so its recommended to check the return value of ADC_SetFreq to confirm your frequency.

## DMA

The ADC may be run in continous mode if a DMA channel is allocated. This can be used in a one-shot or circular mode.

See [DMA](DMA.md) for notes on enabling DMA channels.

### **One-shot mode**:

In one-shot mode, the callback will be called when the buffer is filled. On completion, the ADC will be automatically stopped.

```C
void User_Callback(uint16_t * buffer, uint32_t count)
{
    // This will be called once with 100 samples once the sample is complete.
}

void main()
{
    CORE_Init();
    ADC_Init();
    ADC_SetFreq(23121);
    
    // The User_Callback will be executed when the sample is complete.
    uint16_t buffer[100];
    ADC_Start(ADC_Channel_0, buffer, LENGTH(buffer), false, User_Callback);

    while (1) { CORE_Idle(); }
}
```

### **Circular mode**:

In circular mode, the callback will be called recurringly until the `ADC_Stop()` is called. The callback will be called when the buffer is half full.

```C
void User_Callback(uint16_t * buffer, uint32_t count)
{
    // This will be a recurring call of 50 samples until ADC_Stop() is called.
}

void main()
{
    CORE_Init();
    ADC_Init();
    ADC_SetFreq(23121);
    
    // The User_Callback will be executed periodically
    uint16_t buffer[100];
    ADC_Start(ADC_Channel_0, buffer, LENGTH(buffer), true, User_Callback);
    
    CORE_Delay(1000);
    // A circular transfer must be manually stopped.
    ADC_Stop();
    
    while (1) { CORE_Idle(); }
}
```

# Board

The module is dependant on  definitions within `Board.h`
The following template can be used.
Commented out definitons are optional.

```C
// Module configuration
//#define ADC_VREF	3300

// Optionally enable DMA
//#define ADC_DMA_CH        DMA_CH_1
//#define DMA_CH1_ENABLE
//#define DMA_CH1_RESOURCE  0x0005
```
