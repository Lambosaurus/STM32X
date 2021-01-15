
#include "EEPROM.h"
#include "string.h"

/*
 * PRIVATE DEFINITIONS
 */

/*
 * PRIVATE TYPES
 */

/*
 * PRIVATE PROTOTYPES
 */

static void EEPROM_Unlock(void);
static inline void EEPROM_Lock(void);
static void EEPROM_WaitForOperation(void);

/*
 * PRIVATE VARIABLES
 */

/*
 * PUBLIC FUNCTIONS
 */

void EEPROM_Write(uint32_t offset, const void * data, uint16_t size)
{
	uint8_t * bytes = (uint8_t *)data;
	uint8_t * eeprom = (uint8_t *)(DATA_EEPROM_BASE + offset);

	EEPROM_Unlock();
	for (uint16_t i = 0; i < size; i++)
	{
		if (bytes[i] != eeprom[i])
		{
			eeprom[i] = bytes[i]; // This triggers and 8 bit write operation.
			EEPROM_WaitForOperation();
		}
	}
	EEPROM_Lock();
}

void EEPROM_Read(uint32_t offset, void * data, uint16_t size)
{
	uint8_t * eeprom = (uint8_t *)(DATA_EEPROM_BASE + offset);
	memcpy(data, eeprom, size);
}

/*
 * PRIVATE FUNCTIONS
 */

static void EEPROM_Unlock(void)
{
	// This sequence must not be interrupted.
	uint32_t primask_bit = __get_PRIMASK();
	__disable_irq();

	FLASH->PEKEYR = FLASH_PEKEY1;
	FLASH->PEKEYR = FLASH_PEKEY2;

	__set_PRIMASK(primask_bit);
}

static inline void EEPROM_Lock(void)
{
	SET_BIT(FLASH->PECR, FLASH_PECR_PELOCK);
}

static void EEPROM_WaitForOperation(void)
{
	while(__HAL_FLASH_GET_FLAG(FLASH_FLAG_BSY));
	if (__HAL_FLASH_GET_FLAG(FLASH_FLAG_EOP))
	{
		__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP);
	}
}

/*
 * INTERRUPT ROUTINES
 */


