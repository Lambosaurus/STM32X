# DMA
This module provides control for the DMA controllers.

The header is available [here](../Lib/DMA.h).

> [!TIP]
> This module is primarially for internal use by other modules. The user is only expected to configure this module via the `Board.h` file.

# Usage
Any used DMA channels must be configured to use the correct DMA request signal. For some MCU's this is fixed (L0's), and for others it can be configured (G0's)

## Fixed DMA Requests

For some processors, specific DMA channels route to specific peripherals. You must check your datasheet to confirm the correct DMA channel is used.

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
