# DMA
This module provides control for the DMA controllers.

This module is primarially designed for use in other modules, such as the [ADC](ADC.md) module.
Modules requiring DMA will only need the appropriate DMA channel defined in the Board file, and the rest is done internally.

# Usage
Usage of the module functions are **NOT** expected of the user in normal circumstances. It is primarially for internal use of other libraries.

When assigning DMA channels to peripherals, ensure that the correct channel is used.
DMA channels are linked to specific peripherals. See the datasheet for more information.

# Board

The module is dependant on definitions within `Board.h`
The following template sections are optional depending on your clock requirements.

```C
// Enable DMA channels.
#define DMA_CH1_ENABLE
```
