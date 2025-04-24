# STM32X
This is a HAL layer targeting my favourite STM32 series processors.
Currently only the STM32L0's and STM32F0's are supported. Other series may be added in future, but I will prioritise the smaller processors.

## Features
* A clean API for all peripheral functions
* 'Opt in' design philosophy. Modules only need to be considered when required.
* Clock management and interrupts are internally handled within the modules.
* Expected features like UART circular buffers are built-in
* Much less bloat than a vanilla STM32Cube project. A project using many peripherals including USB-CDC can easily compile to under 16KB.
* Still compatible with the Cube if additional functionality needed (I still use some of it too)
* There is very little abstraction between the STM32X api and the hardware. This promotes efficiency and predictibilty.
* Low power is a key use case for this project: Low power modes are supported, and peripheral init/deinit is under user control.

## Creating a STM32X based project
1) Create a new project using the STM32CubeIDE. STM32Cube should be the targeted project type.
2) Ensure the complete STM32Cube Drivers are added to the project. This can be done by enabling all peripherals in the IOC editer. Alternately [download them from ST](https://www.st.com/en/ecosystems/stm32cube.html#products).
3) Remove junk source from the Core directory. Only `system_stm32*xx.c`, and `startup_stm32*.s` should remain.
4) Checkout the STM32X repo into your project. I recommend adding it as a git sub-module. Add `../STM32X/Lib` to your include paths. Change the resource configurations for the STM32X/Lib folder, so that it **not** excluded from the build.
5) Copy in the replacement `stm32*xx_hal_conf.h`, `main.c`, and `Board.h` file from the STM32X/Templates folder into your Core/ folder.
6) Edit `Board.h` to configure your peripherals. Ensure the correct processor is defined at the top.
7) Build to confirm that the environment is set up correctly.

## Modules

Documentation for each module can be found in [Docs](Docs/)

The [CORE](Docs/CORE.md) module provides a lot of basic functionality, and should be your starting point. Next, [GPIO](Docs/GPIO.md) and [UART](Docs/UART.md) make for good examples of what to expect from this project.


### Basic:
* [CORE](Docs/CORE.md)
* [CLK](Docs/CLK.md)
* [GPIO](Docs/GPIO.md)

### Connectivity:
* [UART](Docs/UART.md)
* [SPI](Docs/SPI.md)
* [I2C](Docs/I2C.md)
* [CAN](Docs/CAN.md)
* [USB](Docs/USB.md)

### Analog:
* [ADC](Docs/ADC.md)
* [COMP](Docs/COMP.md)
* [TSC](Docs/TSC.md)

### Timing:
* [TIM](Docs/TIM.md)
* [RTC](Docs/RTC.md)
* [WDG](Docs/WDG.md)
* [US](Docs/US.md)
* [LPTIM](Docs/LPTIM.md)

### Storage:
* [FLASH](Docs/FLASH.md)
* [EEPROM](Docs/EEPROM.md)

### Misc:
* [CRC](Docs/CRC.md)
