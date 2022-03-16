# USB HID
The Human Interface Device is a [USB](../USB.md) class.

This works 'out of the box' on windows using the standard drivers. A variety of interface devices are supported.

Multiple reports are not supported.

# Usage

The following example enumerates as a mouse with a constant movement:

```c
USB_Init();

while (1)
{
    // Create a report
    USB_HID_Report_t report = {
        .x = 0,
        .y = 1,
        .buttons = 0,
    };

    // Send the report to the host
    USB_HID_Report(&report);

    // Do not emit too many reports.
    CORE_Delay(2);
}
```

Some attempt should be should be made not to generate an unnessicary volume of reports.
The reccommended reporting strategy depends on your specific device type.

## Mouse

The HID device enumerates as a mouse when `USB_HID_MOUSE` is defined.
The report has the following structure:

```c
typedef struct {
    uint8_t buttons, // Bit packed structure of button states
    int8_t x, // Current motion on the X axis.
    int8_t y, // Current motion on the Y axis
} USB_HID_Report_t;
```

The mouse buttons are enumerated in order: mouse button 1 (left click) is bit 0, mouse button 2 (right click) is bit 1, and so on.

# Board

The module is dependant on definitions within `Board.h`. Commented out definitions are optional.

One of the HID types must be selected however.

```c
// USB config
#define USB_ENABLE
// USB HID
#define USB_CLASS_HID
//#define USB_HID_MOUSE
```
