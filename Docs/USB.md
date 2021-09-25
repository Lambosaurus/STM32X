# USB
This provides the functionality of the USB peripheral.

The USB classes are provided as part of this module. Note that currently no composite classes are supported.

Currently USB CDC is the only class implemented. MSC and HID are planned.

# Usage

The bulk of the USB is done under interrupt.

```c
USB_Init();

while (1)
{
    ...
    CORE_Idle();
}
```

The provided functionality and other interfaces are dependant on the selected class.

## USB CDC

This works 'out of the box' on windows using the standard drivers. The interface is intended to be similar to the UART interface, and operates in a non-blocking manner. The USB_Write function is partially blocking - it will block if another USB_Write is not yet complete.

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
//#define USB_VID				    0x0483
//#define USB_PID				    0x5740
//#define USB_PRODUCT_STRING		"STM32X"
//#define USB_MANUFACTURER_STRING	"Lambosaurus"
//#ifndef USB_MAX_POWER_MA          100
```

Note that there are additional defintion options for each class.
```c
// USB CDC
#define USB_CLASS_CDC
//#define USB_CDC_BFR_SIZE	512
```
