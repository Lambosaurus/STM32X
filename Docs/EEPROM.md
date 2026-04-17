# EEPROM
This module enables reading and writing to the chips EEPROM. This can be used for non-volatile data storage.

The header is available [here](../Lib/EEPROM.h).

> [!TIP]  
> EEPROM will not be available on all STM32 variants. The last flash page can be used as an alternative. See [FLASH](FLASH.md) for more info.

# Usage

The offsets are relative to the EEPROM base address.
Ie, an offset of `0` addresses the first byte of EEPROM.

```C
uint8_t data[4] = { 0x01, 0x02, 0x03, 0x04 };
EEPROM_Write(0, data, sizeof(data));
```

The data will now be available to read at any time, such as after a power cycle.

```C
uint8_t data[4];
EEPROM_Read(0, data, sizeof(data));
```

> [!TIP]  
> Take care not to exceed the EEPROM size. Refer to your datasheet for the available EEPROM size.

> [!CAUTION]
> EEPROM has write cycle limitations in the order of 100,000 write cycles. Continous EEPROM writes will eventually result in EEPROM failure.

# Board

The module is dependant on no definitions within `board.h`