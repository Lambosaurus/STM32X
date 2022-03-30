# SPI
This module enables the SPI module in master mode.

Slave mode is not yet supported.

# Usage

Arbitrary SPI bitrates are not supported by the hardware, and usually have to be powers of two of PCLK. This module will select the highest bitrate that does not exceed your specified bitrate.

```c
SPI_Init(SPI_1, 16000000, SPI_Mode_0);

// Write 3 bytes and read 3 bytes over the SPI.
uint8_t tx[3] = { 0x01, 0x02, 0x03 };
uint8_t rx[3];
SPI_Transfer(SPI_1, &tx, &rx, sizeof(tx));
```

Read and write functions are also separately available.
```c
// read data is discarded
uint8_t tx[3] = { 0x01, 0x02, 0x03 };
SPI_Write(SPI_1, &rx, sizeof(rx));

// This writes 0xFF as dummy tx
uint8_t rx[3];
SPI_Read(SPI_1, &rx, sizeof(rx));
```

# Board

The module is dependant on definitions within `Board.h`
The following template can be used.

```C
// SPI configuration
#define SPI1_GPIO		    GPIOB
#define SPI1_PINS		    (GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5)
#define SPI1_AF			    GPIO_AF0_SPI1
```
