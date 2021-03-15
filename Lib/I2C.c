
#include "I2C.h"
#include "Core.h"
#include "GPIO.h"

/*
 * PRIVATE DEFINITIONS
 */

// Filter configuration
#define I2C_USE_ANALOGFILTER
#ifndef I2C_DIGITALFILTER_SIZE
#define I2C_DIGITALFILTER_SIZE	0
#endif

#ifdef I2C_USE_ANALOGFILTER
// This can probably be calculated but it seems to match what ST use
#define I2C_ANALOGFILTER_CYCLES	1
#else
#define I2C_ANALOGFILTER_CYCLES	0
#endif

// Duty is time spent high.
// This might need to be dynamic: but ST seem to use this duty even at standard speed
#define I2C_SCL_DUTY_PCT			33
#define I2C_SCL_SYNC_CYCLES			(3 + I2C_DIGITALFILTER_SIZE + I2C_ANALOGFILTER_CYCLES)

// The maximum bittime is where where SCLL exceeds 255
#define I2C_BITTIME_MAX				(((255 + I2C_SCL_SYNC_CYCLES) * 100) / (100 - I2C_SCL_DUTY_PCT))

#define NS_TO_CYCLES(clk, ns)		(clk/(1000000000/ns))

#define I2C_BUSY_TIMEOUT			10

/*
 * PRIVATE TYPES
 */

/*
 * PRIVATE PROTOTYPES
 */

static void I2Cx_Init(I2C_t * i2c);
static void I2Cx_Deinit(I2C_t * i2c);
static uint32_t I2C_SelectTiming(uint32_t bitrate);


static HAL_StatusTypeDef I2C_WaitOnFlagUntilTimeout(I2C_t *hi2c, uint32_t Flag, FlagStatus Status, uint32_t Timeout, uint32_t Tickstart);
static HAL_StatusTypeDef I2C_WaitOnTXISFlagUntilTimeout(I2C_t *hi2c, uint32_t Timeout, uint32_t Tickstart);
static HAL_StatusTypeDef I2C_WaitOnSTOPFlagUntilTimeout(I2C_t *hi2c, uint32_t Timeout, uint32_t Tickstart);
static HAL_StatusTypeDef I2C_WaitOnRXNEFlagUntilTimeout(I2C_t * hi2c, uint32_t Timeout, uint32_t Tickstart);
static HAL_StatusTypeDef I2C_IsAcknowledgeFailed(I2C_t *hi2c, uint32_t Timeout, uint32_t Tickstart);
static void I2C_TransferConfig(I2C_t *hi2c, uint16_t DevAddress, uint8_t Size, uint32_t Mode, uint32_t Request);

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
	
	//void HAL_I2CEx_EnableFastModePlus(uint32_t ConfigFastModePlus)

	__HAL_I2C_ENABLE(i2c);
}

void I2C_Deinit(I2C_t * i2c)
{
	I2Cx_Deinit(i2c);
}

bool I2C_Tx(I2C_t * i2c, uint8_t address, const uint8_t * data, uint32_t count)
{
	address = address << 1;

    uint32_t tickstart = HAL_GetTick();

    if (I2C_WaitOnFlagUntilTimeout(i2c, I2C_FLAG_BUSY, SET, I2C_BUSY_TIMEOUT, tickstart) != HAL_OK)
    {
    	return false;
    }

    bool started = false;

    while (count > 0)
    {
		uint32_t block = (count > 255) ? 255 : count;
		count -= block;
		I2C_TransferConfig(i2c, address, block,
				count > 0 ? I2C_RELOAD_MODE : I2C_AUTOEND_MODE,
				started ? I2C_NO_STARTSTOP : I2C_GENERATE_START_WRITE);

		started = true;

		while (block > 0)
		{
			if (I2C_WaitOnTXISFlagUntilTimeout(i2c, 100, tickstart) != HAL_OK)
			{
				return false;
			}

			i2c->Instance->TXDR = *data++;
			block--;
		}

		if (count > 0)
		{
			if (I2C_WaitOnFlagUntilTimeout(i2c, I2C_FLAG_TCR, RESET, 100, tickstart) != HAL_OK)
			{
				return false;
			}
		}
    }

    if (I2C_WaitOnSTOPFlagUntilTimeout(i2c, 100, tickstart) != HAL_OK)
    {
      return false;
    }

    __HAL_I2C_CLEAR_FLAG(i2c, I2C_FLAG_STOPF);
    I2C_RESET_CR2(i2c);

    return true;
}


bool I2C_Rx(I2C_t * i2c, uint8_t address, uint8_t * data, uint32_t count)
{
	address = (address << 1) | 0x01;

    uint32_t tickstart = HAL_GetTick();

    if (I2C_WaitOnFlagUntilTimeout(i2c, I2C_FLAG_BUSY, SET, I2C_BUSY_TIMEOUT, tickstart) != HAL_OK)
    {
    	return false;
    }

    bool started = false;

    while (count > 0)
	{
		uint32_t block = (count > 255) ? 255 : count;
		count -= block;
		I2C_TransferConfig(i2c, address, block,
				count > 0 ? I2C_RELOAD_MODE : I2C_AUTOEND_MODE,
				started ? I2C_NO_STARTSTOP : I2C_GENERATE_START_READ);
		started = true;

		while (block > 0)
		{
			if (I2C_WaitOnRXNEFlagUntilTimeout(i2c, 100, tickstart) != HAL_OK)
			{
				return false;
			}
			*data++ = (uint8_t)i2c->Instance->RXDR;
			block--;
		}

		if (count > 0)
		{
			if (I2C_WaitOnFlagUntilTimeout(i2c, I2C_FLAG_TCR, RESET, 100, tickstart) != HAL_OK)
			{
				return false;
			}
		}
	}

    if (I2C_WaitOnSTOPFlagUntilTimeout(i2c, 100, tickstart) != HAL_OK)
    {
    	return false;
    }

    __HAL_I2C_CLEAR_FLAG(i2c, I2C_FLAG_STOPF);
    I2C_RESET_CR2(i2c);

    return true;
}


bool I2C_TxRx(I2C_t * i2c, uint8_t address, const uint8_t * txdata, uint32_t txcount, uint8_t * rxdata, uint32_t rxcount)
{
	return I2C_Tx(i2c, address, txdata, txcount) && I2C_Rx(i2c, address, rxdata, rxcount);
}


bool I2C_Scan(I2C_t * i2c, uint8_t address);

/*
 * PRIVATE FUNCTIONS
 */

static uint32_t I2C_SelectTiming(uint32_t bitrate)
{
	uint32_t clk = HAL_RCC_GetPCLK1Freq();

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
	if (bitrate <= I2C_Mode_Standard)
	{
		scl_del = NS_TO_CYCLES(clk, 250);
	}
	else if (bitrate <= I2C_Mode_Fast)
	{
		scl_del = NS_TO_CYCLES(clk, 100);
	}
	else
	{
		scl_del = NS_TO_CYCLES(clk, 50);
	}

	return (prescalar << 28) | (scl_del << 20) | (sda_del << 16) | (scl_h << 8) | scl_l;
}

static void I2Cx_Init(I2C_t * i2c)
{
	GPIO_InitTypeDef gpio = {0};
	gpio.Mode = GPIO_MODE_AF_OD;
	gpio.Pull = GPIO_NOPULL;
	gpio.Speed = GPIO_SPEED_HIGH;

#ifdef I2C1_GPIO
	if (i2c == I2C_1)
	{
		__HAL_RCC_I2C1_CLK_ENABLE();
		gpio.Pin = I2C1_PINS;
		gpio.Alternate = I2C1_AF;
		HAL_GPIO_Init(I2C1_GPIO, &gpio);
	}
#endif
#ifdef I2C2_GPIO
	if (i2c == I2C_2)
	{
		__HAL_RCC_I2C2_CLK_ENABLE();
		gpio.Pin = I2C2_PINS;
		gpio.Alternate = I2C2_AF;
		HAL_GPIO_Init(I2C2_GPIO, &gpio);
	}
#endif
#ifdef I2C3_GPIO
	if (i2c == I2C_3)
	{
		__HAL_RCC_I2C3_CLK_ENABLE();
		gpio.Pin = I2C3_PINS;
		gpio.Alternate = I2C3_AF;
		HAL_GPIO_Init(I2C3_GPIO, &gpio);
	}
#endif
}

static void I2Cx_Deinit(I2C_t * i2c)
{
#ifdef I2C1_GPIO
	if (i2c == I2C_1)
	{
		__HAL_RCC_I2C1_CLK_DISABLE();
		GPIO_Disable(I2C1_GPIO, I2C1_PINS);
	}
#endif
#ifdef I2C2_GPIO
	if (i2c == I2C_2)
	{
		__HAL_RCC_I2C2_CLK_DISABLE();
		GPIO_Disable(I2C2_GPIO, I2C2_PINS);
	}
#endif
#ifdef I2C3_GPIO
	if (i2c == I2C_3)
	{
		__HAL_RCC_I2C3_CLK_DISABLE();
		GPIO_Disable(I2C3_GPIO, I2C3_PINS);
	}
#endif
}

/*
 * INTERRUPT ROUTINES
 */

/*
 * HAL GARBAGE
 */

static HAL_StatusTypeDef I2C_WaitOnFlagUntilTimeout(I2C_t *hi2c, uint32_t Flag, FlagStatus Status, uint32_t Timeout, uint32_t Tickstart)
{
  while (__HAL_I2C_GET_FLAG(hi2c, Flag) == Status)
  {
    /* Check for the Timeout */
    if (Timeout != HAL_MAX_DELAY)
    {
      if (((HAL_GetTick() - Tickstart) > Timeout) || (Timeout == 0U))
      {
        return HAL_ERROR;
      }
    }
  }
  return HAL_OK;
}

static HAL_StatusTypeDef I2C_WaitOnTXISFlagUntilTimeout(I2C_t *hi2c, uint32_t Timeout, uint32_t Tickstart)
{
  while (__HAL_I2C_GET_FLAG(hi2c, I2C_FLAG_TXIS) == RESET)
  {
    /* Check if a NACK is detected */
    if (I2C_IsAcknowledgeFailed(hi2c, Timeout, Tickstart) != HAL_OK)
    {
      return HAL_ERROR;
    }

    /* Check for the Timeout */
    if (Timeout != HAL_MAX_DELAY)
    {
      if (((HAL_GetTick() - Tickstart) > Timeout) || (Timeout == 0U))
      {
        return HAL_ERROR;
      }
    }
  }
  return HAL_OK;
}

static HAL_StatusTypeDef I2C_WaitOnSTOPFlagUntilTimeout(I2C_t *hi2c, uint32_t Timeout, uint32_t Tickstart)
{
  while (__HAL_I2C_GET_FLAG(hi2c, I2C_FLAG_STOPF) == RESET)
  {
    /* Check if a NACK is detected */
    if (I2C_IsAcknowledgeFailed(hi2c, Timeout, Tickstart) != HAL_OK)
    {
      return HAL_ERROR;
    }

    /* Check for the Timeout */
    if (((HAL_GetTick() - Tickstart) > Timeout) || (Timeout == 0U))
    {
      return HAL_ERROR;
    }
  }
  return HAL_OK;
}

static HAL_StatusTypeDef I2C_WaitOnRXNEFlagUntilTimeout(I2C_t * hi2c, uint32_t Timeout, uint32_t Tickstart)
{
  while (__HAL_I2C_GET_FLAG(hi2c, I2C_FLAG_RXNE) == RESET)
  {
    /* Check if a NACK is detected */
    if (I2C_IsAcknowledgeFailed(hi2c, Timeout, Tickstart) != HAL_OK)
    {
      return HAL_ERROR;
    }

    /* Check if a STOPF is detected */
    if (__HAL_I2C_GET_FLAG(hi2c, I2C_FLAG_STOPF) == SET)
    {
      /* Check if an RXNE is pending */
      /* Store Last receive data if any */
      if ((__HAL_I2C_GET_FLAG(hi2c, I2C_FLAG_RXNE) == SET))
      {
        /* Return HAL_OK */
        /* The Reading of data from RXDR will be done in caller function */
        return HAL_OK;
      }
      else
      {
        /* Clear STOP Flag */
        __HAL_I2C_CLEAR_FLAG(hi2c, I2C_FLAG_STOPF);

        /* Clear Configuration Register 2 */
        I2C_RESET_CR2(hi2c);
        return HAL_ERROR;
      }
    }

    /* Check for the Timeout */
    if (((HAL_GetTick() - Tickstart) > Timeout) || (Timeout == 0U))
    {
      return HAL_ERROR;
    }
  }
  return HAL_OK;
}

static void I2C_Flush_TXDR(I2C_t *hi2c)
{
  /* If a pending TXIS flag is set */
  /* Write a dummy data in TXDR to clear it */
  if (__HAL_I2C_GET_FLAG(hi2c, I2C_FLAG_TXIS) != RESET)
  {
    hi2c->Instance->TXDR = 0x00U;
  }

  /* Flush TX register if not empty */
  if (__HAL_I2C_GET_FLAG(hi2c, I2C_FLAG_TXE) == RESET)
  {
    __HAL_I2C_CLEAR_FLAG(hi2c, I2C_FLAG_TXE);
  }
}

static HAL_StatusTypeDef I2C_IsAcknowledgeFailed(I2C_t *hi2c, uint32_t Timeout, uint32_t Tickstart)
{
  if (__HAL_I2C_GET_FLAG(hi2c, I2C_FLAG_AF) == SET)
  {
    /* Wait until STOP Flag is reset */
    /* AutoEnd should be initiate after AF */
    while (__HAL_I2C_GET_FLAG(hi2c, I2C_FLAG_STOPF) == RESET)
    {
      /* Check for the Timeout */
      if (Timeout != HAL_MAX_DELAY)
      {
        if (((HAL_GetTick() - Tickstart) > Timeout) || (Timeout == 0U))
        {
          return HAL_ERROR;
        }
      }
    }

    /* Clear NACKF Flag */
    __HAL_I2C_CLEAR_FLAG(hi2c, I2C_FLAG_AF);

    /* Clear STOP Flag */
    __HAL_I2C_CLEAR_FLAG(hi2c, I2C_FLAG_STOPF);

    /* Flush TX register */
    I2C_Flush_TXDR(hi2c);

    /* Clear Configuration Register 2 */
    I2C_RESET_CR2(hi2c);

    return HAL_ERROR;
  }
  return HAL_OK;
}

static void I2C_TransferConfig(I2C_t *hi2c, uint16_t DevAddress, uint8_t Size, uint32_t Mode, uint32_t Request)
{
  /* update CR2 register */
  MODIFY_REG(hi2c->Instance->CR2, ((I2C_CR2_SADD | I2C_CR2_NBYTES | I2C_CR2_RELOAD | I2C_CR2_AUTOEND | (I2C_CR2_RD_WRN & (uint32_t)(Request >> (31U - I2C_CR2_RD_WRN_Pos))) | I2C_CR2_START | I2C_CR2_STOP)), \
             (uint32_t)(((uint32_t)DevAddress & I2C_CR2_SADD) | (((uint32_t)Size << I2C_CR2_NBYTES_Pos) & I2C_CR2_NBYTES) | (uint32_t)Mode | (uint32_t)Request));
}
