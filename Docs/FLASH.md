# FLASH
This module enables reading and writing to the chips FLASH. This can be used for reprogramming the processor, or storing data.

This can only be erased in pages, so take care to understand how this will affect your application. If you are looking to store settings, first check if you could instead use the [EEPROM](EEPROM.md) module.

# Usage

Take care when writing to flash for the following reasons:
* Your application is executing out of flash. Overwriting this will end poorly.
* Flash has write cycle limitations - in the order of 100,000 write cycles. If you leave a program to continously write to flash, it will eventually fail.

It is helpful to think of flash in terms of pages - but using the actual memory address is usually the most practical way to read and write to it.
To this end, I have provided helper functions to convert from page numbers to addresses.

```C
// This gets the last page of flash - a practical place for storing settings.
uint32_t lastPage = FLASH_GetPageCount() - 1;
uint32_t * address = FLASH_GetPage(lastPage);

// Erasing happens on pages. Note that the address must be a page start.
FLASH_Erase(address);

// Writes must align to words.
uint32_t data[] = { 0x00000001, 0x00000002, 0x00000003, 0x00000004 };
FLASH_Write(address, data, sizeof(data));
```
When writing to flash you do not have to cover an entire page, but they must **NOT** cross page boundaries. In this way: data can be written to flash piecemeal.

# Board

The module is dependant on no definitions within `board.h`