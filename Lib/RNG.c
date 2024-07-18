#include "RNG.h"

#ifdef RNG_ENABLE
#include "CLK.h"

/*
 * PRIVATE DEFINITIONS
 */

#define RNG_SR_ERROR_BITS	(RNG_SR_SECS | RNG_SR_CECS)

#define RNG_HTCFG_KEY   	0x17590ABCU
#define RNG_HTCFG    	 	0x0000AA74U // Recommended value for NIST compliance

/*
 * PRIVATE TYPES
 */

/*
 * PRIVATE PROTOTYPES
 */

static void RNG_Init(void);
static void RNG_Deinit(void);
static uint32_t RNG_ReadWord(void);

/*
 * PRIVATE VARIABLES
 */

/*
 * PUBLIC FUNCTIONS
 */

uint32_t RNG_Read(void)
{
	RNG_Init();
	uint32_t word = RNG_ReadWord();
	RNG_Deinit();
	return word;
}

void RNG_ReadBytes(uint8_t * bfr, uint32_t size)
{
	if (size == 0) { return; }
	RNG_Init();
    uint32_t word = RNG_ReadWord();
    switch (size % 4)
	{
        do
        {
    case 0: *bfr++ = word >> 24;
    case 3: *bfr++ = word >> 16;
    case 2: *bfr++ = word >> 8;
    case 1: *bfr++ = word;
            size -= 4;
            word = RNG_ReadWord();
        }
        while ((int32_t)size > 0);
	}
	RNG_Deinit();
}

int32_t RNG_RandInt(int32_t min, int32_t max)
{
	uint32_t range = max - min;
	return (int32_t)(RNG_Read() % (range + 1)) + min;
}

/*
 * PRIVATE FUNCTIONS
 */

static void RNG_Init(void)
{
	CLK_EnableRNGCLK();
	__HAL_RCC_RNG_CLK_ENABLE();

	// Put in idle mode
	CLEAR_BIT(RNG->CR, RNG_CR_RNGEN);

	// Clock Error Detection Configuration when CONDRT bit is set to 1
	MODIFY_REG(RNG->CR, RNG_CR_CED | RNG_CR_CONDRST, RNG_CR_CONDRST);

	// Default NIST RNG behavior.
	#if defined(RNG_VER_3_2) || defined(RNG_VER_3_1) || defined(RNG_VER_3_0)
	WRITE_REG(RNG->HTCR, RNG_HTCFG_KEY);
	WRITE_REG(RNG->HTCR, RNG_HTCFG);
	#endif

	// Wait for conditioning
	CLEAR_BIT(RNG->CR, RNG_CR_CONDRST);
	while (HAL_IS_BIT_SET(RNG->CR, RNG_CR_CONDRST));

	// Enable
	SET_BIT(RNG->CR, RNG_CR_RNGEN);
}

static void RNG_Deinit(void)
{
	CLEAR_BIT(RNG->CR, RNG_CR_RNGEN);
	__HAL_RCC_RNG_CLK_DISABLE();
	CLK_DisableRNGCLK();
}

static uint32_t RNG_ReadWord(void)
{
	// Wait for a ready word or error
	while (!(RNG->SR & (RNG_SR_DRDY | RNG_SR_ERROR_BITS)));

	if (RNG->SR & RNG_SR_ERROR_BITS)
	{
		return 0;
	}
	return RNG->DR;
}

/*
 * INTERRUPT ROUTINES
 */

#endif //RNG_ENABLE
