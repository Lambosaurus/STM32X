# I2C
This module enables master operation of an I2C bus.

Slave operation is not yet implemented.

# Usage

The I2C frequency can be arbitrary, but the `I2C_Mode_t` enum is provided for convenience.

Note that the device address should be specified as a 7 bit address. The R/W bit is automatically set.

```c
I2C_Init(I2C_1, I2C_Mode_Fast);

uint8_t tx[] = { 0x01, 0x02, 0x03 };
uint8_t rx[3];

// Write 3 bytes to device at address 0x44, then read 3 bytes
if (   I2C_Write(I2C_1, 0x44, tx, sizeof(tx))
    && I2C_Read(I2C_1, 0x44, rx, sizeof(rx)) )
{
    // Note that all I2C operations may fail
    ...
}
```

All I2C operations may fail - usually for the following reasons:
* The bus is not "Idle". One of the SCL or SDA lines are not in the idle (high) state. Check your pullups.
* The device did not acknowledge one of the written bytes, either because it was an invalid operation, or the device is not present on the bus.
* A device attempted to stall the transaction by clock stretching for an extended ammount of time.

Most realistic reads require a write to set up the read address. Because of this a write following by a repeated read is provided. The following code is functionally equivilent to the above - but faster.

```c
I2C_Init(I2C_1, I2C_Mode_Fast);

uint8_t tx[] = { 0x01, 0x02, 0x03 };
uint8_t rx[3];

// A write followed by a repeated read.
if ( I2C_Transfer(I2C_1, 0x44, tx, sizeof(tx), rx, sizeof(rx)) )
{
    ...
}
```

A helper function has also been provided for checking whether a device is present. The scan checks for an acknowledgement on a zero length write.

```c
I2C_Init(I2C_1, I2C_Mode_Fast);

for (uint8_t address = 0; address < 0x7F; address++)
{
    if (I2C_Scan(I2C_1, address))
    {
        // A device acknowledged this address
        ...
    }
}
```

## I2C Timeouts

Because I2C supports clock stretching, all I2C transfers have a timeout. This timeout is implemented per byte, and can be overidden using the `I2C_TIMEOUT` definiton.
Some devices may require extended clock stretching, and so this timeout may need to be increased.

# Board

The module is dependant on  definitions within `Board.h`
The following template can be used. Commented out definitions are optional.

```C
// I2C configuration
#define I2C1_GPIO			GPIOB
#define I2C1_PINS			(GPIO_PIN_6 | GPIO_PIN_7)
#define I2C1_AF			    GPIO_AF1_I2C1
//#define USE_I2C_FASTMODEPLUS
//#define I2C_TIMEOUT       10
```