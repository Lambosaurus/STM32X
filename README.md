# STM32X
This is a HAL layer targeting my favourite STM32 series processors.
Currently only the STM32L0's and STM32F0's are supported. Other series may be added in future, but I will prioritise the smaller processors.

## Features
* A clean API for all peripheral functions
* Clock management handled
* Expected features like UART circular buffers are built-in
* Much less bloat than a vanilla STM32Cube project. A project using many peripherals including USB-CDC can easily compile to under 16KB.
* Still compatible with the Cube if additional functionality needed (I still use much of it too)

## Creating a STM32X based project
1) Create a new project using the STM32CubeIDE. STM32Cube should be the targeted project type.
2) Ensure the complete STM32Cube Drivers are added to the project. This can be done by enabling all peripherals in the IOC editer. Alternately [download them from ST](https://www.st.com/en/ecosystems/stm32cube.html#products).
3) Remove junk source from the Core directory. Only `system_stm32*xx.c`, and `startup_stm32*.s` should remain.
4) Checkout the STM32X repo into your project. I recommend adding it as a git sub-module. Add `../STM32X/Lib` to your include paths. Change the resource configurations for the STM32X/Lib folder, so that it **not** excluded from the build.
5) Copy in the replacement `stm32*xx_hal_conf.h`, `main.c`, and `Board.h` file from the STM32X/Templates folder into your Core/ folder.
6) Edit `Board.h` to configure your peripherals. Ensure the correct processor is defined at the top.
7) Build to confirm that the environment is set up correctly.

## Module highlights
### CORE
* Clock intialisation
* Idle mode (wait for interrupt)
* System tick

## RTC
* LSE/LSI configuration
* Periodic and alarm callbacks

### UART
* Interrupt driven reads & writes
* Inbuilt circular buffers

### SPI
* Master only
* Blocking read & write

### I2C
* Master only
* Blocking read & write
* Automatic timing config & fast mode plus

### GPIO
* Inlined reads & writes
* Interrupt callbacks

### ADC
* Blocking reads on input channels
* Handles VREF & Temperature channels

### TIM
* PWM configuration
* Interrupt callbacks on reload and compare channels

### CAN
* Polling read & writes
* Uses rx mailboxes as storage

### USB
* Interrupt based USB
* USB-CDC (VCOM port)
	* Works 'out of the box' using default windows driver
	* Inbuilt circular buffers

### WDG
* Uses the independant watchdog

### Others
* CRC
* EEPROM
* FLASH
