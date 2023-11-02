# USB CDC
The Communications Device Class is a [USB](../USB.md) class.

This works 'out of the box' on windows using the standard drivers. The interface is intended to be similar to the UART interface, and operates in a non-blocking manner. The USB_Write function is partially blocking - it will block if another USB_Write is not yet complete.

The header is available [here](../../Lib/usb/cdc/USB_CDC.h).

# Usage

The following demonstrates an echoing program.

```c
USB_Init();

while (1)
{
    uint8_t bfr[32];
    uint32_t read = USB_Read(bfr, sizeof(bfr));
    if (read)
    {
        // Write the bytes back
        USB_Write(bfr, read);
    }
    CORE_Idle();
}
```

# Board

The module is dependant on definitions within `Board.h`. Commented out definitions are optional.

```c
// USB config
#define USB_ENABLE
// USB CDC
#define USB_CLASS_CDC
//#define USB_CDC_BFR_SIZE	512
```
