
# USB MSC

The Mass Storage Class is a [USB](../USB.md) class.

This works 'out of the box' on windows using the standard drivers.

This provides access to an abstract storage interface. This interface will have to be implemented by the user. An example of this would be an SD card or a virtual file system.

The header is available [here](../../Lib/usb/msc/USB_MSC.h).

# Usage

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
//    Write the supplied data to the file
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
USB_MSC_Mount(&gStorage);
USB_Init();

while (1)
{
    CORE_Idle();
}
```

# Board

The module is dependant on definitions within `Board.h`.

```c
// USB config
#define USB_ENABLE
// USB MSC
#define USB_CLASS_MSC
```

