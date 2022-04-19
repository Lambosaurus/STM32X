# CAN
This module enables a CAN bus peripheral.
The bus is always run with 29 bit addresses.

Note that the use of an external oscillator is reccommended for bus reliability.

The header is available [here](../Lib/CAN.h).

# Usage

The CAN bitrate will be automatically converted into time quanta.
Filters can be enabled to recieve the requested messages.

Once initialised, the recieved messages will be queued into the CAN FIFO's, and should be polled. Interrupts are currently not utilised.

```C
CAN_Init(250000);
CAN_EnableFilter(0, 0, 0); // All messages

while (1)
{
    // Poll for reception
    CANMsg_t rx;
    if (CAN_Read(&rx))
    {
        ...
        // Reply
        CANMsg_t tx = {
            .id = 0x00200000,
            .data = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08 },
            .len = 8,
        };
        CAN_Write(&tx);
    }
    ..
    CORE_Idle();
}
```

Up to `FILTER_BANK_COUNT` CAN filters can be enabled
```C
// Arguments are: index, id_mask, id_match 
CAN_EnableFilter(0, 0x00F00000, 0x00100000);
CAN_EnableFilter(1, 0x00F00000, 0x00200000);
```

# Board

The module is dependant on  definitions within `Board.h`
The following template can be used. Commented out settings are optional.

```C
// CAN configuration
#define CAN_GPIO		GPIOA
#define CAN_PINS		(GPIO_PIN_8 | GPIO_PIN_9)
#define CAN_AF          GPIO_AF4_CAN

// This will enable both FIFO's, to increase RX capacity.
// Odd numbered filters will be routed to the second FIFO.
//#define CAN_DUAL_FIFO
```

The CAN time quanta will be automatically computed, but may be manually specified in the Board.h file. This is not reccommended unless you have a specific requirement to do so.

```C
// Set the time segments
#define CAN_SEG1_TQ     8
#define CAN_SEG2_TQ     16
// Synchronisation jump width
#define CAN_SJW_TQ      2
```
