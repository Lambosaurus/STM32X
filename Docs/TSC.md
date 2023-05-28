# TSC
This provides control of the touch sense controllers.

This enables monitoring of touch pins (or other capacitive based sensors)

The header is available [here](../Lib/TSC.h).

# Usage

Understanding how the groups and channels are laid out on your processor is critical to using this peripheral. Note that each pin has a specific group and channel.

`TSC_EnableCapacitor` is used to enable the sampling capacitor. One sampling capacitor should be enabled for each group.

`TSC_EnableInput` is used to enable a channel as a sampled input. It will be compared to the sampling capacitor for its group.

```c
// Enable TSC_Group_1 - with sample capacitor on PA0, and the input element on PA1.
TSC_Init();
TSC_EnableCapacitor(TSC_G1_Channel1, PA0);
TSC_EnableInput(TSC_G1_Channel2, PA1);

while(1)
{
    // Read the input
    uint32_t cycles = TSC_Read(TSC_G1_Channel2);
    ...
}
```

## Multiple inputs
Multiple inputs can be enabled simultaniously as long as they are on the same port.

```c
TSC_Init();
TSC_EnableCapacitor(TSC_G1_Channel1, PA0);
TSC_EnableInput(TSC_G1_Channel2 | TSC_G1_Channel3 | TSC_G1_Channel3, PA1 | PA2 | PA3);

while(1)
{
    // Read the input
    uint32_t cycles_a = TSC_Read(TSC_G1_Channel2);
    uint32_t cycles_b = TSC_Read(TSC_G1_Channel3);
    uint32_t cycles_c = TSC_Read(TSC_G1_Channel4);
    ...
}
```

# Board

The module is dependant on no definitions within `Board.h`