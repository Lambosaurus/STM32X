
# USB MTP

The Media Transport Protocol is a [USB](../USB.md) class.

This works 'out of the box' on windows using the standard drivers.

This provides access to an file based storage interface. The file operations need to be implemented by the user.
Files are handled in a transactional manner.

The header is available [here](../../Lib/usb/mtp/USB_MTP.h).
See [MTP_FS.h](../../Lib/usb/mtp/MTP_FS.h) for information on the MTP filesystem

# Usage

The USB MTP device is run as below.

```c
// Initialise the MTP handler
MTP_t mtp;
MTP_Init(&mtp);

// The storage may be mounted before USB_Init is called
// Once USB_Init is called, the USB Host may request access to the files at any time.
USB_MTP_Mount(&mtp);
USB_Init();

while (1)
{
    CORE_Idle();
}
```

## Declaring files

File need to be initialised with the following information:

```c
// File_Read:
//    Read the requested data into the suppled buffer.
//    Return true on success.
static bool File_Read(uint8_t * bfr, uint32_t pos, uint32_t count);

// File_Write:
//    Write the supplied data to the file
//    Return true on success.
//    This function may be NULL for a read only device.
//    EOF will be signalled with a zero length write.
static bool File_Write(const uint8_t * bfr, uint32_t pos, uint32_t count);


MTP_File_t file = {
    .name = "Filename.txt",
    .size = 1024,           // The read size of the file
    .read = File_Read,
    .write = File_Write,
}

// Files can be added dynamically at runtime.
// Take care not to exceed USB_MTP_MAX_OBJECTS
MTP_AddFile(&mtp, &file);
```

The read and write calls will be called transactionally - The read will start at 0, and be called sequentially until completion.

## Updating files

A file can be updated after it is declared. `MTP_UpdateFile` should be called to inform the host that the file and contents have changed.

```c
// Edit the file.
file.size = 2048;

// Files can be added dynamically at runtime.
// Take care not to exceed USB_MTP_MAX_OBJECTS
MTP_UpdateFile(&mtp, &file);
```

## Handling new files

Files may be created by the host. You can register a handler for this using `MTP_OnNewFile`.

An example handler is below:

```c
// Memory for the new objects must be provided by the user.
static char gNewFileName[32];
static MTP_File_t gNewFile;

MTP_File_t * USER_OnNewFile(const char * name, uint32_t size)
{
    if (size < 4096 && strlen(name) < sizeof(gNewFileName))
    {
        // Copy the supplied parameters
        strcpy(gNewFileName, name);
        gNewFile.name = gNewFileName;
        gNewFile.size = size;

        // Supply read and write callbacks (write will be immediately invoked)
        gNewFile.write = File_Write,
        gNewFile.read = File_Read,

        // Return the object.
        return &gNewFile;
    }
    // Return null to reject a file
    return NULL;
}
```

This file will now be tracked by the MTP file system. Take care to clean these files up using `MTP_RemoveFile` for long running applications to not exceed `USB_MTP_MAX_OBJECTS`.


# Board

The module is dependant on definitions within `Board.h`.

```c
// USB config
#define USB_ENABLE
// USB MTP
#define USB_CLASS_MTP
// Configure the maximum number of file objects in a MTP_t object.
#define USB_MTP_MAX_OBJECTS	        8
// This is required for windows to load the correct drivers.
#define USB_INTERFACE_STRING		"MTP"
```

