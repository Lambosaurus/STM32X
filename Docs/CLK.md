# CLK
This module provides helpers for managing the processors clocks.

The users primarially interaction with this module will be configuring the clocks via the `Board.h` file.

# Usage

## Functions

Usage of the module functions are **NOT** expected of the user in normal circumstances. It is primarially for internal use of other libraries. `CLK_Init()` will be called within the [CORE](CORE.md) module, and should not be called by the user.

The clock frequencies can be queried via the following functions.
```C
// Alternately known as SYSCLK
uint32_t hclk = CLK_GetHCLKFreq();

// Peripheral clocks
uint32_t pclk = CLK_GetPCLKFreq();

// Low speed oscillator - May be LSI or LSE
uint32_t lso = CLK_GetLSOFreq();
```

The clock configuraion will be detailed below in the Board section.

# Board

The module is dependant on definitions within `Board.h`
The following template sections are optional depending on your clock requirements.

## Low speed oscillators:

A low speed oscillator is required for the use of the [RTC](RTC.md).
The presense of an external low speed oscillator (LSE) can be specifed using the following.

```C
// LSE configuration
#define CLK_USE_LSE
#define CLK_LSE_FREQ    32768

// Bypass the resonator if the LSE is a digital oscillator instead of a crystal.
//#define CLK_LSE_BYPASS
```

In the absence of the LSE, the internal LSI will be automatically used.

## High speed oscillators:

A high speed oscillator is required for almost all peripherals and the generation of the system clock.
The presense of an external high speed oscillator (HSE) can be specified using the following.

```C
// HSE configuration
#define CLK_USE_HSE
#define CLK_HSE_FREQ    8000000
```

In the absence of the HSE, the internal HSI  will be automatically used.

## SYSCLK and PLL configuration

The PLL will be automatically configued to achieve the required system clock frequency. The PLL will use the HSI or HSE if available, or will be disabled if the system clock already matches the HSI/HSE.

The PLL multiplication and division factors will be automatically computed - but may need user assistance for some combinations. Specifiy the multiplier only if required.

```C
// PLL configuration
#define CLK_SYSCLK_FREQ   32000000
//#define CLK_PLL_MUL     4
```