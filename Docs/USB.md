# USB
This provides the functionality of the USB peripheral.

The USB classes are provided as part of this module.

Currently USB CDC and MSC are implemented. HID and composite devices are planned.

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

## USB CDC:

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

## USB MSC:

This works 'out of the box' on windows using the standard drivers. This provides access to an abstract storage interface. This interface will have to be implemented by the user. An example of this would be an SD card or a virtual file system.

The storage interface should be declared as below. Note that blocks are 512 byte segments of data.

```c
// Storage_Open:
//    Prepare the storage device for read & write.
//    The number of blocks (512 byte segments) in the device must be stored in *blk_count
//    This function may be called multiple times if USB is reset by the Host.
//    Return true on success.
static bool Storage_Open(uint32_t * blk_count);

// Storage_Read:
//    Read the requested data into the suppled buffer.
//    Return true on success.
static bool Storage_Read(uint8_t * bfr, uint32_t blk_addr, uint16_t blk_count);

// Storage_Write:
//    Read the requested data into the suppled buffer.
//    Return true on success.
//    This function may be NULL for a read only device.
static bool Storage_Write(const uint8_t * bfr, uint32_t blk_addr, uint16_t blk_count);

static const USB_Storage_t gStorage = {
    .open = Storage_Open,
    .read = Storage_Read,
    .write = Storage_Write,
}
```

The USB MSC device is run as below.

```c
// The storage may be mounted before USB_Init is called
// Once USB_Init is called, the USB Host may request access to the storage at any time.
// Mount NULL to declare no storage media.
USB_Mount(&gStorage);
USB_Init();

while (1)
{
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

```c
// USB MSC
#define USB_CLASS_MSC
```
