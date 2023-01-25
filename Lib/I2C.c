
#include "I2C.h"
#include "Core.h"
#include "GPIO.h"
#include "CLK.h"

#ifdef I2C_ENABLE

/*
 * PRIVATE DEFINITIONS
 */

// Filter configuration
#define I2C_USE_ANALOGFILTER
#define I2C_DIGITALFILTER_SIZE	0
#define I2C_SCL_DUTY_PCT 60

#ifdef I2C_USE_ANALOGFILTER
// This can probably be calculated but it seems to match what ST use
#define I2C_ANALOGFILTER_CYCLES	1
#else
#define I2C_ANALOGFILTER_CYCLES	0
#endif

// Duty is time spent high.
// This might need to be dynamic: but ST seem to use this duty even at standard speed
#define I2C_SCL_SYNC_CYCLES			(3 + I2C_DIGITALFILTER_SIZE + I2C_ANALOGFILTER_CYCLES)

// The maximum bittime is where where SCLL exceeds 255
#define I2C_BITTIME_MAX				(((255 + I2C_SCL_SYNC_CYCLES) * 100) / (100 - I2C_SCL_DUTY_PCT))

#define NS_TO_CYCLES(clk, ns)		(clk/(1000000000/ns))

#ifndef I2C_TIMEOUT
#define I2C_TIMEOUT					5
#endif

#define I2C_READ_MODE				I2C_CR2_RD_WRN
#define I2C_WRITE_MODE				0U
#define I2C_START_MODE				I2C_CR2_START

#define _I2C_GET_FLAGS(i2c)			(i2c->Instance->ISR)
#define _I2C_SET_FMP(bit)			(SYSCFG->CFGR2 |= bit)
#define _I2C_CLR_FMP(bit)			(SYSCFG->CFGR2 &= ~bit)

/*
 * PRIVATE TYPES
 */

/*
 * PRIVATE PROTOTYPES
 */

static void I2Cx_Init(I2C_t * i2c);
static void I2Cx_Deinit(I2C_t * i2c);
static uint32_t I2C_SelectTiming(uint32_t bitrate);

static bool I2C_IsAcknowledgeFailed(I2C_t *hi2c);
static bool I2C_WaitForIdle(I2C_t * i2c);
static bool I2C_WaitForFlag(I2C_t * i2c, uint32_t flag);
static bool I2C_WaitForRXNE(I2C_t * i2c);
static inline void I2C_StartTransfer(I2C_t * i2c, uint8_t address, uint8_t size, uint32_t mode);

static bool I2C_XferBlock(I2C_t * i2c, uint8_t address, uint8_t * data, uint32_t count, uint32_t rw, uint32_t endMode);

#ifdef I2C_USE_FASTMODEPLUS
static uint32_t I2Cx_GetFMPBit(I2C_t * i2c);
#endif

/*
 * PRIVATE VARIABLES
 */

#ifdef I2C1_GPIO
static I2C_t gI2C_1 = {
	.Instance = I2C1
};
I2C_t * I2C_1 = &gI2C_1;
#endif
#ifdef I2C2_GPIO
static I2C_t gI2C_2 = {
	.Instance = I2C2
};
I2C_t * I2C_2 = &gI2C_2;
#endif
#ifdef I2C3_GPIO
static I2C_t gI2C_3 = {
	.Instance = I2C3
};
I2C_t * I2C_3 = &gI2C_3;
#endif


/*
 * PUBLIC FUNCTIONS
 */

void I2C_Init(I2C_t * i2c, I2C_Mode_t mode)
{
	i2c->mode = mode;
	I2Cx_Init(i2c);
	
	__HAL_I2C_DISABLE(i2c);
	
	i2c->Instance->TIMINGR = I2C_SelectTiming((uint32_t)mode); // The mode is the enumerated frequency
	uint32_t cr1 = I2C_NOSTRETCH_DISABLE | (I2C_DIGITALFILTER_SIZE << 8);
#ifndef I2C_USE_ANALOGFILTER
	cr1 |= I2C_CR1_ANFOFF;
#endif
	i2c->Instance->CR1 = cr1;
	i2c->Instance->CR2 = I2C_CR2_AUTOEND | I2C_CR2_NACK;
	
	// Disable own addressing modes.
	i2c->Instance->OAR1 = 0;
	i2c->Instance->OAR2 = 0;
	//i2c->Instance->OAR1 &= ~I2C_OAR1_OA1EN;
	//i2c->Instance->OAR1 = I2C_OAR1_OA1EN | ownAddress1;
	//i2c->Instance->OAR2 &= ~I2C_OAR2_OA2EN;
	//i2c->Instance->OAR2 = I2C_OAR2_OA2EN | ownAddress2 | SMBUS_OA2_NOMASK;
	
#ifdef I2C_USE_FASTMODEPLUS
	if (mode > I2C_Mode_Fast)
	{
		uint32_t bit = I2Cx_GetFMPBit(i2c);
		_I2C_SET_FMP(bit);
	}
#endif

	__HAL_I2C_ENABLE(i2c);
}

void I2C_Deinit(I2C_t * i2c)
{
	__HAL_I2C_DISABLE(i2c);

#ifdef I2C_USE_FASTMODEPLUS
	if (i2c->mode > I2C_Mode_Fast)
	{
		uint32_t bit = I2Cx_GetFMPBit(i2c);
		_I2C_CLR_FMP(bit);
	}
#endif

	I2Cx_Deinit(i2c);
	i2c->mode = 0;
}

bool I2C_Write(I2C_t * i2c, uint8_t address, const uint8_t * data, uint32_t count)
{
	return I2C_WaitForIdle(i2c)
		&& I2C_XferBlock(i2c, address, (uint8_t *)data, count, I2C_WRITE_MODE, I2C_AUTOEND_MODE);
}

bool I2C_Read(I2C_t * i2c, uint8_t address, uint8_t * data, uint32_t count)
{
	return I2C_WaitForIdle(i2c)
		&& I2C_XferBlock(i2c, address, data, count, I2C_READ_MODE, I2C_AUTOEND_MODE);
}

bool I2C_Transfer(I2C_t * i2c, uint8_t address, const uint8_t * txdata, uint32_t txcount, uint8_t * rxdata, uint32_t rxcount)
{
	return I2C_WaitForIdle(i2c)
		&& I2C_XferBlock(i2c, address, (uint8_t *)txdata, txcount, I2C_WRITE_MODE, I2C_SOFTEND_MODE)
		&& I2C_XferBlock(i2c, address, rxdata, rxcount, I2C_READ_MODE, I2C_AUTOEND_MODE);
}

bool I2C_Scan(I2C_t * i2c, uint8_t address)
{
	return I2C_Write(i2c, address, NULL, 0);
}

/*
 * PRIVATE FUNCTIONS
 */

static bool I2C_XferBlock(I2C_t * i2c, uint8_t address, uint8_t * data, uint32_t count, uint32_t rw, uint32_t endMode)
{
	// Correct the address.
	address = address << 1;
	if (rw == I2C_READ_MODE) { address |= 0x01; }

	// Default transaction parameters.
	uint32_t startMode = I2C_START_MODE;
	uint32_t stopMode = I2C_RELOAD_MODE;
	uint32_t block = 255;

	while (stopMode == I2C_RELOAD_MODE)
	{
		if (count <= 255)
		{
			// This is the final block of data.
			block = count;
			stopMode = endMode;
		}
		count -= block;

		I2C_StartTransfer(i2c, address, block, rw | startMode | stopMode);
		startMode = I2C_NO_STARTSTOP; // Following loops will not have a start condition.

		if (rw == I2C_READ_MODE)
		{
			while (block-- > 0)
			{
				if (!I2C_WaitForRXNE(i2c))
				{
					return false;
				}
				*data++ = i2c->Instance->RXDR;
			}
		}
		else
		{
			while (block-- > 0)
			{
				if (!I2C_WaitForFlag(i2c, I2C_FLAG_TXIS))
				{
					return false;
				}
				i2c->Instance->TXDR = *data++;
			}
		}

		uint32_t stopflag;
		switch (stopMode)
		{
		case I2C_RELOAD_MODE:
			stopflag = I2C_FLAG_TCR;
			break;
		case I2C_SOFTEND_MODE:
			stopflag = I2C_FLAG_TC;
			break;
		default:
		case I2C_AUTOEND_MODE:
			stopflag = I2C_FLAG_STOPF;
			break;
		}
		if (!I2C_WaitForFlag(i2c, stopflag))
		{
			return false;
		}
	}

	__HAL_I2C_CLEAR_FLAG(i2c, I2C_FLAG_STOPF);
	I2C_RESET_CR2(i2c);
	return true;
}

static bool I2C_WaitForIdle(I2C_t * i2c)
{
	uint32_t start = CORE_GetTick();
    while (_I2C_GET_FLAGS(i2c) & I2C_FLAG_BUSY)
    {
    	if (CORE_GetTick() - start > I2C_TIMEOUT)
    	{
    		return false;
    	}
    }
    return true;
}

static bool I2C_WaitForFlag(I2C_t * i2c, uint32_t flag)
{
	uint32_t start = CORE_GetTick();
	while (CORE_GetTick() - start < I2C_TIMEOUT)
	{
		if (_I2C_GET_FLAGS(i2c) & flag)
		{
			return true;
		}
		else if (I2C_IsAcknowledgeFailed(i2c))
		{
			return false;
		}
	}
	return false;
}

static bool I2C_WaitForRXNE(I2C_t * i2c)
{
	uint32_t start = CORE_GetTick();

	while (CORE_GetTick() - start < I2C_TIMEOUT)
	{
		uint32_t flags = _I2C_GET_FLAGS(i2c);
		if (flags & I2C_FLAG_RXNE)
		{
			return true;
		}
		else if (flags & I2C_FLAG_STOPF)
		{
			__HAL_I2C_CLEAR_FLAG(i2c, I2C_FLAG_STOPF);
			I2C_RESET_CR2(i2c);
			break;
		}
		else if (I2C_IsAcknowledgeFailed(i2c))
		{
			break;
		}
	}
	return false;
}

static void I2C_FlushTXDR(I2C_t * i2c)
{
	if (_I2C_GET_FLAGS(i2c) & I2C_FLAG_TXIS)
	{
		i2c->Instance->TXDR = 0x00U;
	}
	if (!(_I2C_GET_FLAGS(i2c) & I2C_FLAG_TXE))
	{
		__HAL_I2C_CLEAR_FLAG(i2c, I2C_FLAG_TXE);
	}
}

static bool I2C_IsAcknowledgeFailed(I2C_t * i2c)
{
  if (_I2C_GET_FLAGS(i2c) & I2C_FLAG_AF)
  {
    uint32_t start = CORE_GetTick();
    while (CORE_GetTick() - start < I2C_TIMEOUT)
    {
    	if (_I2C_GET_FLAGS(i2c) & I2C_FLAG_STOPF)
    	{
    		break;
    	}
    }

    __HAL_I2C_CLEAR_FLAG( i2c, I2C_FLAG_STOPF | I2C_FLAG_AF);
    I2C_FlushTXDR(i2c);
    I2C_RESET_CR2(i2c);

    return true;
  }
  return false;
}

static inline void I2C_StartTransfer(I2C_t * i2c, uint8_t address, uint8_t size, uint32_t mode)
{
	MODIFY_REG(i2c->Instance->CR2, ((I2C_CR2_SADD | I2C_CR2_NBYTES | I2C_CR2_RELOAD | I2C_CR2_AUTOEND | I2C_CR2_RD_WRN | I2C_CR2_START | I2C_CR2_STOP)), \
             (uint32_t)((uint32_t)address | ((uint32_t)size << I2C_CR2_NBYTES_Pos) | mode));
}

static uint32_t I2C_SelectTiming(uint32_t bitrate)
{
	uint32_t clk = CLK_GetPCLKFreq();

	uint32_t prescalar = 0;
	uint32_t bittime = clk / bitrate;
	if (bittime > I2C_BITTIME_MAX)
	{
		// Bittime is unachievable. Divide the clock down.
		prescalar = bittime / I2C_BITTIME_MAX;
		// Note: we really want the ceil of the above calc, but prescalar has an implicit +1
		clk /= prescalar + 1;
		bittime = clk / bitrate;
	}

	uint32_t scl_h = bittime * I2C_SCL_DUTY_PCT / 100;
	uint32_t scl_l = bittime - scl_h;
	scl_h -= I2C_SCL_SYNC_CYCLES;
	scl_l -= I2C_SCL_SYNC_CYCLES;

	uint32_t scl_del;
	uint32_t sda_del = 0;

#ifdef I2C_USE_FASTMODEPLUS
	if (bitrate > I2C_Mode_Fast)
	{
		scl_del = NS_TO_CYCLES(clk, 50);
	}
	else
#endif
	if (bitrate > I2C_Mode_Standard)
	{
		scl_del = NS_TO_CYCLES(clk, 100);
	}
	else
	{
		scl_del = NS_TO_CYCLES(clk, 250);
	}

	return (prescalar << 28) | (scl_del << 20) | (sda_del << 16) | (scl_h << 8) | scl_l;
}

static void I2Cx_Init(I2C_t * i2c)
{
#ifdef I2C1_GPIO
	if (i2c == I2C_1)
	{
		__HAL_RCC_I2C1_CLK_ENABLE();
		GPIO_EnableAlternate(I2C1_GPIO, I2C1_PINS, GPIO_Flag_OpenDrain, I2C1_AF);
	}
#endif
#ifdef I2C2_GPIO
	if (i2c == I2C_2)
	{
		__HAL_RCC_I2C2_CLK_ENABLE();
		GPIO_EnableAlternate(I2C2_GPIO, I2C2_PINS, GPIO_Flag_OpenDrain, I2C2_AF);
	}
#endif
#ifdef I2C3_GPIO
	if (i2c == I2C_3)
	{
		__HAL_RCC_I2C3_CLK_ENABLE();
		GPIO_EnableAlternate(I2C3_GPIO, I2C3_PINS, GPIO_Flag_OpenDrain, I2C3_AF);
	}
#endif
}

static void I2Cx_Deinit(I2C_t * i2c)
{
#ifdef I2C1_GPIO
	if (i2c == I2C_1)
	{
		__HAL_RCC_I2C1_CLK_DISABLE();
		GPIO_Deinit(I2C1_GPIO, I2C1_PINS);
	}
#endif
#ifdef I2C2_GPIO
	if (i2c == I2C_2)
	{
		__HAL_RCC_I2C2_CLK_DISABLE();
		GPIO_Deinit(I2C2_GPIO, I2C2_PINS);
	}
#endif
#ifdef I2C3_GPIO
	if (i2c == I2C_3)
	{
		__HAL_RCC_I2C3_CLK_DISABLE();
		GPIO_Deinit(I2C3_GPIO, I2C3_PINS);
	}
#endif
}

#ifdef I2C_USE_FASTMODEPLUS
static uint32_t I2Cx_GetFMPBit(I2C_t * i2c)
{
	uint32_t bit;
#ifdef I2C1_GPIO
	if (i2c == I2C_1)
	{
		bit = I2C_FASTMODEPLUS_I2C1;
	}
#endif
#ifdef I2C2_GPIO
	if (i2c == I2C_2)
	{
		bit = I2C_FASTMODEPLUS_I2C2;
	}
#endif
#ifdef I2C3_GPIO
	if (i2c == I2C_3)
	{
		bit = I2C_FASTMODEPLUS_I2C3;
	}
#endif
	if (bit & I2C_FMP_NOT_SUPPORTED)
	{
		return 0;
	}
	return bit;
}
#endif

/*
 * INTERRUPT ROUTINES
 */

#endif //I2C_ENABLE
