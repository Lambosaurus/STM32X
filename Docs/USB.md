# USB
This provides the functionality of the USB peripheral.

The USB classes are provided via this module

The header is available [here](../Lib/USB.h).

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

# Classes
The following classes are supported:
* [Communications Device](usb_class/CDC.md)
* [Mass Storage](usb_class/MSC.md)
* [Composite Device](usb_class/Composite.md)

Refer to the class specific documentation for their usage.

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