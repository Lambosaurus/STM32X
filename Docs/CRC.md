# CRC
This module provides access to the CRC unit.
This can compute a 32 bit CRC in only 4 bus cycles

## Usage

No initialisation of this module is required.

Note that the size is specified in bytes, not words.
The default polynomial of 0x4C11DB7 is used.

```C
uint32_t page1[] = { 0x00000001, 0x00000002, 0x00000003, 0x00000004 };
uint32_t crc = CRC32(0xFFFFFFFF, page1, sizeof(page1));

// When required a CRC can be continued by using the previous CRC as the next init value
uint32_t page2[] = { 0x00000005, 0x00000006, 0x00000007, 0x00000008 };
crc = CRC32(crc, page2, sizeof(page2));
```

## Board

The module is dependant on no definitions within `board.h`