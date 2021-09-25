# Module name
Describes the basic function, and the primary use case.
Some future changes may be mentioned too.

## Usage

Detailed notes on how the module should be used. An example block should always be below.
Multiple example blocks are encourages to show distinct use cases.

```C
Module_Init();

while (1)
{
    uint8_t example[] = { 0x01, 0x02, 0x03 };
    Module_Write(example, sizeof(example));
    ..
    CORE_Idle();
}
```

Caveats and warnings about edge cases are important.

## Board

The module is dependant on  definitions within `Board.h`
The following template can be used.

```C
// Module configuration
#define MODULE_GPIO		GPIOA
#define MODULE_PIN		GPIO_PIN_1
```