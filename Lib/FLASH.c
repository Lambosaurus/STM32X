
#include "FLASH.h"

/*
 * PRIVATE DEFINITIONS
 */

#ifdef FLASH_ENABLE


#if defined(STM32L0)

#define _FLASH_SET_CR(bit)			(FLASH->PECR |= bit)
#define _FLASH_CLR_CR(bit)			(FLASH->PECR &= ~bit)
#define _FLASH_CR_PROG				FLASH_PECR_PROG
#define _FLASH_CR_FPROG				(FLASH_PECR_PROG | FLASH_PECR_FPRG)
#define _FLASH_CR_LOCK				(FLASH_PECR_PELOCK | FLASH_PECR_PRGLOCK)
#define _FLASH_CR_ERASE				(FLASH_PECR_PROG | FLASH_PECR_ERASE)

#elif defined(STM32F0)

// This is not defined on these devices
#define FLASH_SIZE					(FLASH_BANK1_END + 1 - FLASH_BASE)

#define _FLASH_SET_CR(bit)			(FLASH->CR |= bit)
#define _FLASH_CLR_CR(bit)			(FLASH->CR &= ~bit)
#define _FLASH_CR_PROG				FLASH_CR_PG
#define _FLASH_CR_LOCK				FLASH_CR_LOCK
#define _FLASH_CR_ERASE				FLASH_CR_PER

#elif defined(STM32G0) || defined(STM32WL)

#define _FLASH_SET_CR(bit)			(FLASH->CR |= bit)
#define _FLASH_CLR_CR(bit)			(FLASH->CR &= ~bit)
#define _FLASH_CR_PROG				FLASH_CR_PG
#define _FLASH_CR_LOCK				FLASH_CR_LOCK
#define _FLASH_CR_ERASE				FLASH_CR_PER

#endif


// Treat this like a function: It dereferences an address, so must be calculated (On some series)
#define FLASH_PAGE_COUNT()		(FLASH_SIZE / FLASH_PAGE_SIZE)

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
#if defined(STM32L0)
__RAM_FUNC void FLASH_WriteHalfPage(uint32_t * __restrict address, const uint32_t * __restrict data);
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

const uint32_t * FLASH_GetPage(uint32_t page)
{
	return (const uint32_t *)(FLASH_BASE + (FLASH_PAGE_SIZE * page));
}

void FLASH_Erase(const uint32_t * address)
{
	FLASH_Unlock();
	_FLASH_SET_CR(_FLASH_CR_ERASE);

#if defined(STM32L0)
	// Write 0 to the first word of the page to erase
	*((uint32_t *)address) = 0;
#elif defined(STM32F0)
    FLASH->AR = (uint32_t)address;
    _FLASH_SET_CR(FLASH_CR_STRT);
#elif defined(STM32G0) || defined(STM32WL)
    uint32_t page_number = ((uint32_t)address - FLASH_BASE) / FLASH_PAGE_SIZE;
    MODIFY_REG(FLASH->CR, FLASH_CR_PNB, page_number << FLASH_CR_PNB_Pos);
    _FLASH_SET_CR(FLASH_CR_STRT);
#endif

    FLASH_WAIT_FOR_OPERATION();
	_FLASH_CLR_CR(_FLASH_CR_ERASE);
	FLASH_Lock();
}

void FLASH_Write(const uint32_t * address, const uint32_t * data, uint32_t size)
{
	uint32_t dest = (uint32_t)address;
	const void * data_head = data;
	const void * data_end = data_head + size;

	FLASH_Unlock();

#if defined (STM32L0)
	if ((data_end - data_head) >= (FLASH_PAGE_SIZE/2))
	{
		_FLASH_SET_CR(_FLASH_CR_FPROG);
		while ((data_end - data_head) >= (FLASH_PAGE_SIZE/2))
		{
			FLASH_WriteHalfPage((uint32_t *)dest, data_head);
			dest += (FLASH_PAGE_SIZE/2);
			data_head += (FLASH_PAGE_SIZE/2);
		}
		_FLASH_CLR_CR(_FLASH_CR_FPROG);
	}
	if (size >= sizeof(uint32_t))
	{
		_FLASH_SET_CR(_FLASH_CR_PROG);
		while (data_end - data_head >= sizeof(uint32_t))
		{
			*(__IO uint32_t*)(dest) = *(uint32_t*)data_head;
			data_head += sizeof(uint32_t);
			dest += sizeof(uint32_t);
			FLASH_WAIT_FOR_OPERATION();
		}
		_FLASH_CLR_CR(_FLASH_CR_PROG);
	}

#elif defined(STM32F0)
	_FLASH_SET_CR(_FLASH_CR_PROG);
	while (data_end - data_head >= sizeof(uint16_t))
	{
		*(__IO uint16_t*)(dest) = *(uint16_t*)data_head;
		data_head += sizeof(uint16_t);
		dest += sizeof(uint16_t);
		FLASH_WAIT_FOR_OPERATION();
	}
	_FLASH_CLR_CR(_FLASH_CR_PROG);

#elif defined(STM32G0) || defined(STM32WL)
	_FLASH_SET_CR(_FLASH_CR_PROG);
	while (data_end - data_head >= (2 * sizeof(uint32_t)))
	{
		*(__IO uint32_t *)dest = *(uint32_t*)data_head;
		dest += sizeof(uint32_t);
		data_head += sizeof(uint32_t);

		__ISB();

		*(__IO uint32_t *)dest = *(uint32_t*)data_head;
		dest += sizeof(uint32_t);
		data_head += sizeof(uint32_t);
	}
	_FLASH_CLR_CR(_FLASH_CR_PROG);

#endif

	FLASH_Lock();
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
#elif defined(STM32G0)  || defined(STM32WL)
	FLASH->KEYR = FLASH_KEY1;
	FLASH->KEYR = FLASH_KEY2;
#endif

	__set_PRIMASK(primask_bit);
}

static inline void FLASH_Lock(void)
{
	_FLASH_SET_CR(_FLASH_CR_LOCK);
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
#endif

/*
 * INTERRUPT ROUTINES
 */

#endif //FLASH_ENABLE

