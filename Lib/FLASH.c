
#include "FLASH.h"
#include <string.h>

/*
 * PRIVATE DEFINITIONS
 */


#if defined(STM32L0)

#define _FLASH_SET_CR(bit)			(FLASH->PECR |= bit)
#define _FLASH_CLR_CR(bit)			(FLASH->PECR &= ~bit)
#define _FLASH_CR_PROG				(FLASH_PECR_PROG | FLASH_PECR_FPRG)
#define _FLASH_CR_LOCK				FLASH_PECR_LOCK
#define _FLASH_CR_ERASE				(FLASH_PECR_PROG | FLASH_PECR_ERASE)

#elif defined(STM32F0)

// This is not defined on these devices
#define FLASH_SIZE					(FLASH_BANK1_END + 1 - FLASH_BASE)

#define _FLASH_SET_CR(bit)			(FLASH->CR |= bit)
#define _FLASH_CLR_CR(bit)			(FLASH->CR &= ~bit)
#define _FLASH_CR_PROG				FLASH_CR_PG
#define _FLASH_CR_LOCK				FLASH_CR_LOCK
#define _FLASH_CR_ERASE				FLASH_CR_PER

#endif


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
static void FLASH_Write(uint32_t * address, const uint32_t * data);
#if defined(STM32L0)
__RAM_FUNC void FLASH_WriteHalfPage(uint32_t * __restrict address, const uint32_t * __restrict data);
#elif defined(STM32F0)
static inline void FLASH_WriteWord(uint32_t * address, uint32_t word);
#endif

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
	FLASH_Write(address, data);
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

static void FLASH_Unlock(void)
{
	// This sequence must not be interrupted.
	uint32_t primask_bit = __get_PRIMASK();
	__disable_irq();

#if defined(STM32L0)
	FLASH->PEKEYR = FLASH_PEKEY1;
	FLASH->PEKEYR = FLASH_PEKEY2;
	FLASH->PRGKEYR = FLASH_PRGKEY1;
	FLASH->PRGKEYR = FLASH_PRGKEY2;
#elif defined(STM32F0)
	FLASH->KEYR |= FLASH_KEY1;
	FLASH->KEYR |= FLASH_KEY2;
#endif

	__set_PRIMASK(primask_bit);
}

static inline void FLASH_Lock(void)
{
	_FLASH_SET_CR(_FLASH_CR_LOCK);
}

static void FLASH_Erase(uint32_t * address)
{
	_FLASH_SET_CR(_FLASH_CR_ERASE);

#if defined(STM32L0)
	// Write 0 to the first word of the page to erase
	*address = 0;
	FLASH_WAIT_FOR_OPERATION();
#elif defined(STM32F0)
    FLASH->AR = (uint32_t)address;
    _FLASH_SET_CR(FLASH_CR_STRT);
#endif
    FLASH_WAIT_FOR_OPERATION();
	_FLASH_CLR_CR(_FLASH_CR_ERASE);
}

static void FLASH_Write(uint32_t * address, const uint32_t * data)
{
	_FLASH_SET_CR(_FLASH_CR_PROG);
#if defined (STM32L0)
	FLASH_WriteHalfPage(address, data);
	FLASH_WriteHalfPage(address + (FLASH_PAGE_SIZE/2), data + (FLASH_PAGE_SIZE/2));
#else
	for (uint32_t i = 0; i < (FLASH_PAGE_SIZE / sizeof(uint32_t)); i++)
	{
		FLASH_WriteWord(address, data[i]);
	}
#endif
	_FLASH_CLR_CR(_FLASH_CR_PROG);
}

#if defined(STM32L0)
__RAM_FUNC void FLASH_WriteHalfPage(uint32_t * __restrict address, const uint32_t * __restrict data)
{
	for (uint32_t i = 0; i < ((FLASH_PAGE_SIZE/2) / sizeof(uint32_t)); i++)
	{
		*(__IO uint32_t*)(address) = *data++;
	}
	FLASH_WAIT_FOR_OPERATION();
}
#elif defined(STM32F0)
static inline void FLASH_WriteWord(uint32_t * address, uint32_t word)
{
	uint16_t * words = (uint16_t *)(&word);

	*(__IO uint16_t*)((uint32_t)address + 0) = (uint16_t)words[0];
	FLASH_WAIT_FOR_OPERATION();

	*(__IO uint16_t*)((uint32_t)address + 2) = (uint16_t)words[1];
	FLASH_WAIT_FOR_OPERATION();
}
#endif

/*
 * INTERRUPT ROUTINES
 */


