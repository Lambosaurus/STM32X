# TSC
This provides control of the touch sense controllers.

This enables monitoring of touch pins (or other capacitive based sensors)

The header is available [here](../Lib/TSC.h).

# Usage

Understanding how the groups and channels are laid out on your processor is critical to using this peripheral. Note that each pin has a specific group and channel.

`TSC_EnableGroup` is used to enable one of the TSC groups, and the channel + pin provided should include the reference capacitor.

`TSC_EnableChannel` is then used to enable the channels to be monitored by the group. While a group may support multiple channels, this is not currently supported.

```c
// Enable TSC_Group_1 - with sample capacitor on PA1, and the pad on PA2.
TSC_Init();
TSC_EnableGroup(TSC_Group_1, TSC_Channel_2, PA1);
TSC_EnableChannel(TSC_Group_1, TSC_Channel_3, PA2);

while(1)
{
    // Start the sampling process, then read the channel.
    TSC_Sample();
    uint32_t cycles = TSC_Read(TSC_Group_1);
    ...
}
```

# Board

The module is dependant on no definitions within `Board.h`