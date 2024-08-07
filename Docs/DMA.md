# DMA
This module provides control for the DMA controllers.

This module is primarially designed for use in other modules, such as the [ADC](ADC.md) module.
Modules requiring DMA will only need the appropriate DMA channel defined in the Board file, and the rest is done internally.

The header is available [here](../Lib/DMA.h).

# Usage
Usage of the module functions are **NOT** expected of the user in normal circumstances. It is primarially for internal use of other libraries.

The DMA channel must be configured to use the correct DMA request signal. For some MCU's this is fixed (L0's), and for others it can be configured (G0's)

## Fixed DMA Requests

For some processors, you must make sure that the used DMA channel can recieve requests from the desired peripheral. Check your datasheet.

## Routable DMA Requests

For some processors, the DMA request routing is handled by the DMA-MUX.

The correct request ID must be configured in the `Board.h` file for each used channel. Check your datasheet for the appropriate request ID.

For example, `#define DMA_CH1_RESOURCE 0x0005` associates the DMA_CH1 with request #5, which is usually the ADC.

# Board

The module is dependant on definitions within `Board.h`

```C
// Enable DMA channels.
#define DMA_CH1_ENABLE
//#define DMA_CH1_RESOURCE  0x0005
```
