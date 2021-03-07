
#include "CRC.h"

/*
 * PRIVATE DEFINITIONS
 */

/*
 * PRIVATE TYPES
 */

/*
 * PRIVATE PROTOTYPES
 */

/*
 * PRIVATE VARIABLES
 */

/*
 * PUBLIC FUNCTIONS
 */

uint32_t CRC32(uint32_t crc, uint32_t * words, uint32_t size)
{
	__HAL_RCC_CRC_CLK_ENABLE();
	WRITE_REG(CRC->INIT, crc);
	MODIFY_REG(CRC->CR, CRC_CR_REV_IN, CRC_INPUTDATA_INVERSION_NONE);
	MODIFY_REG(CRC->CR, CRC_CR_REV_OUT, CRC_OUTPUTDATA_INVERSION_DISABLE);

	for (uint32_t i = 0; i < size/4; i++)
	{
		CRC->DR = words[i];
	}

	crc = CRC->DR;
	__HAL_RCC_CRC_CLK_DISABLE();

	return crc;
}

/*
 * PRIVATE FUNCTIONS
 */

/*
 * INTERRUPT ROUTINES
 */
