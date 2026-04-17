# FLASH
This module enables reading and writing to the chips FLASH. This can be used for reprogramming the processor, or storing data.

The header is available [here](../Lib/FLASH.h).

> [!TIP]
> The flash can only be erased in whole pages. First check if you could use [EEPROM](EEPROM.md) instead.

# Usage

The FLASH module operates on memory addresses. Helpers are provided to convert page numbers to addresses.

```C
// This gets the last page of flash - a practical place for storing settings.
uint32_t lastPage = FLASH_GetPageCount() - 1;
const uint32_t * address = FLASH_GetPage(lastPage);

// Erasing happens on pages. Note that the address must be a page start.
FLASH_Erase(address);

// Writes must align to words.
uint32_t data[] = { 0x00000001, 0x00000002, 0x00000003, 0x00000004 };
FLASH_Write(address, data, sizeof(data));

...

// Note that the address is valid reference to flash and can be read
for (int i = 0; i < 4; i++)
{
    // The stored data can be read back
    uint32_t value = address[i];
    ...
}
```

> [!IMPORTANT]
> Flash writes do not need to be entire pages, but **must not cross page boundaries**.

> [!WARNING]  
> Flash write/erase operations cause read operations to be stalled. This can result in your program and all interrupts being stalled for 100's of milliseconds.

> [!CAUTION]  
> Flash has write cycles limitations on the order of 10,000 write cycles. Continous flash writes will eventually result in flash failure.

# Board

The module is dependant on no definitions within `board.h`
