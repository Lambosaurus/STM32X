# CLK
This module provides helpers for managing the processors clocks.

The users primarially interaction with this module will be configuring the clocks via the `Board.h` file.

This module is in need of optimisation - and to allow SYSCLKs other than 32MHz

## Usage

Usage of the module functions are **NOT** expected of the user in normal circumstances. It is primarially for internal use of other libraries. `CLK_Init()` will be called within the CORE module, and should not be called by the user.

The clock frequencies can be queried via the following functions.
```C
// Alternately known as SYSCLK
uint32_t hclk = CLK_GetHCLKFreq();

// Peripheral clocks
uint32_t pclk = CLK_GetPCLKFreq();

// Low speed oscillator - May be LSI or LSE
uint32_t lso = CLK_GetLSOFreq();
```

## Board

The module is dependant on definitions within `Board.h`
The following template sections are both optional depending on your clocks.

The following is used to enable the HSE.
Note that `HSE_VALUE` must still be set within `stm32*0xx_hal_conf.h`
```C
// HSE configuration
#define CLK_USE_HSE
```

The following is used to enable the LSE
```C
// LSE configuration
#define CLK_USE_LSE
#define CLK_LSE_FREQ    32768

// Bypass the resonator if the LSE is an oscillator instead of a crystal.
//#define CLK_LSE_BYPASS
```