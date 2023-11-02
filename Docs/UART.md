# UART
This module enables the UART module.

The UART is operated in interrupt mode, and includes circular buffers for recieve and transmit.

The header is available [here](../Lib/UART.h).

# Usage

The following demonstrates basic usage of the UART.

```c
// Set up a standard uart with 115200 baud.
UART_Init(UART_1, 115200, UART_Mode_Default);

// Write 3 bytes to the uart
uint8_t tx[] = { 0x01, 0x02, 0x03 };
UART_Write(UART_1, tx, sizeof(tx));

...

// Read up to 3 bytes from the UART.
uint8_t rx[3];
uint32_t read = UART_Read(UART_1, rx, sizeof(rx));
// Note that this call does not block, and will return 0 immediately if the uart is empty.
```

## Writing:

`UART_Write()` is appropriate for writing binary data, and `UART_WriteStr()` is a helper for writing null terminated strings. These functions will not block if there is space in the transmit buffer. If the transmit buffer is full, these functions will block until the data is in the buffer.

Because the UART transmit is interrupt driven, you may need to flush the transmitter before deinitialisation, otherwise the transmit will be cut short.

```c
UART_WriteStr(UART_1, "Entering stop mode\r\n");

// This call will block untill the UART is finished transmitting
UART_WriteFlush(UART_1);

UART_Deinit(UART_1);
CORE_Stop();
```

## Reading:

The UART must be monitored in a polled manner. Alternative methods are provided for reading blocks of data, or reading in a byte-wise manner. Use either depending on your needs.

```c
uint8_t rx[3];
uint32_t count = UART_ReadCount(UART_1);
for (uint32_t i = 0; i < count && i < sizeof(rx); i++)
{
    // UART_Pop() should not be called if the rx buffer is empty
    rx[i] = UART_Pop(UART_1);
}

// Above and below are equivalent

uint8_t rx[3];
uint32_t count = UART_Read(UART_1, rx, sizeof(rx));
```

`UART_ReadFlush()` is discards any data currently in the rx buffer.

## Line reading:

No built in helper is available for line reading: because the specific needs of line reading are so varied. The following example demonstrates a basic blocking line reader. 

```c
uint32_t User_ReadLine(char * line, uint32_t maxSize, uint32_t timeout)
{
    uint32_t start = CORE_GetTick();
    uint32_t size = 0;
    while (CORE_GetTick() - start < timeout)
    {
        if (UART_ReadCount(UART_1))
        {
            char ch = UART_Pop(UART_1);
            if (ch == '\n')
            {
                // Line is complete. Null terminate and return.
                line[size] = 0;
                return size;
            }
            else if (size >= maxSize)
            {
                // Maximum size exceeded. panic.
                return 0;
            }
            else
            {
                line[size++] = ch;
            }
        }
    }
    // Timeout occurred
    return 0;
}
```

# Board

The module is dependant on definitions within `Board.h`
The following template can be used. Commented out definitions are optional.

```C
// UART Config
#define UART1_GPIO		GPIOA
#define UART1_PINS		(GPIO_PIN_9 | GPIO_PIN_10)
#define UART1_AF		    GPIO_AF4_USART1
//#define UART_BFR_SIZE     128
```
