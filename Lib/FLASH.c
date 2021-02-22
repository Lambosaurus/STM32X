
#include "FLASH.h"
#include <string.h>

/*
 * PRIVATE DEFINITIONS
 */

// Treat this like a function: It dereferences an address, so must be calculated.
#define FLASH_PAGE_COUNT()		(FLASH_SIZE / FLASH_PAGE_SIZE)
#define FLASH_PAGE_BASE(page)	((uint32_t *)(FLASH_BASE + (FLASH_PAGE_SIZE * page)))

// This must be a macro, as the __RAM_FUNC's should not call other functions.
#define FLASH_WAIT_FOR_OPERATION()					\
	while(__HAL_FLASH_GET_FLAG(FLASH_FLAG_BSY));	\
	if (__HAL_FLASH_GET_FLAG(FLASH_FLAG_EOP)) {		\
		__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP);		\
	}												\

/*
 * PRIVATE TYPES
 */

/*
 * PRIVATE PROTOTYPES
 */

static void FLASH_Unlock(void);
static inline void FLASH_Lock(void);
static void FLASH_Erase(uint32_t * address);
__RAM_FUNC void FLASH_WriteHalfPage(uint32_t * __restrict address, const uint32_t * __restrict data);

/*
 * PRIVATE VARIABLES
 */

/*
 * PUBLIC FUNCTIONS
 */

uint32_t FLASH_GetPageCount(void)
{
	return FLASH_PAGE_COUNT();
}

void FLASH_WritePage(uint32_t page, const uint32_t * data)
{
	FLASH_Unlock();

	uint32_t * address = FLASH_PAGE_BASE(page);
	FLASH_Erase(address);

	FLASH->PECR |= FLASH_PECR_PROG | FLASH_PECR_FPRG;
	FLASH_WriteHalfPage(address, data);
	FLASH_WriteHalfPage(address + (FLASH_PAGE_SIZE/2), data + (FLASH_PAGE_SIZE/2));
	FLASH->PECR &= ~(FLASH_PECR_PROG | FLASH_PECR_FPRG);

	FLASH_Lock();
}

void FLASH_ReadPage(uint32_t page, uint32_t * data)
{
	uint32_t * address = FLASH_PAGE_BASE(page);
	memcpy(data, address, FLASH_PAGE_SIZE);
}


/*
 * PRIVATE FUNCTIONS
 */

static void FLASH_Erase(uint32_t * address)
{
	FLASH->PECR |= FLASH_PECR_PROG | FLASH_PECR_ERASE;
	// Write 0 to the first word of the page to erase
	*address = 0;
	FLASH_WAIT_FOR_OPERATION();
	FLASH->PECR &= ~(FLASH_PECR_PROG | FLASH_PECR_ERASE);
}

static void FLASH_Unlock(void)
{
	// This sequence must not be interrupted.
	uint32_t primask_bit = __get_PRIMASK();
	__disable_irq();

	FLASH->PEKEYR = FLASH_PEKEY1;
	FLASH->PEKEYR = FLASH_PEKEY2;
	FLASH->PRGKEYR = FLASH_PRGKEY1;
	FLASH->PRGKEYR = FLASH_PRGKEY2;

	__set_PRIMASK(primask_bit);
}

static inline void FLASH_Lock(void)
{
	// Can these be done both at once?
	FLASH->PECR |= FLASH_PECR_PELOCK | FLASH_PECR_PRGLOCK;
}

__RAM_FUNC void FLASH_WriteHalfPage(uint32_t * __restrict address, const uint32_t * __restrict data)
{
	for (uint32_t i = 0; i < ((FLASH_PAGE_SIZE/2) / sizeof(uint32_t)); i++)
	{
		*(__IO uint32_t*)(address) = *data++;
	}
	FLASH_WAIT_FOR_OPERATION();
}

/*
 * INTERRUPT ROUTINES
 */


