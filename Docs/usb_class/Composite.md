
# USB Composite

The Composite device allows multiple [USB](../USB.md) classes to be used.

# Python tools
The composite device must be generated using the supplied tools in [Tools/usb_composite](../../Tools/usb-composite/).
Python is required to run these. These are tested on python 3, but **may** be compatible with python 2.7.

`usb_composite.py` takes two arguments:
* `dest`: The destination folder for the generated class source
* `classes`: A list of classes to be used

An example usage is `python usb_composite.py --dest "../../../user" --classes CDC MSC`
This will generate a `USB_Composite.c` and `USB_Composite.h` that enables the [CDC](CDC.md) and [MSC](MSC.md) classes.

These files should be added to your project.

# Usage

Once built, the usage of this should be as per the included classes. See the specific documentation for each class.

# Board

The module is dependant on definitions within `Board.h`.

Note that the specific classes **must** also be enabled and configured as per their documentation.

```c
// USB config
#define USB_ENABLE
// USB Composite
#define USB_CLASS_COMPOSITE
```

For example: when using [CDC](CDC.md) and [MSC](MSC.md) classes, this segment will look like:

```c
// USB config
#define USB_ENABLE
// USB Composite
#define USB_CLASS_COMPOSITE
#define USB_CLASS_MSC
#define USB_CLASS_CDC
```

