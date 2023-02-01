
#include "I2C.h"
#include "Core.h"
#include "GPIO.h"
#include "CLK.h"

#include "system_stm32f4xx.h"

/*
 * PRIVATE DEFINITIONS
 */

/*
 * PRIVATE TYPES
 */

/*
 * PRIVATE PROTOTYPES
 */

static void I2Cx_Init(I2C_t * i2c);
static void I2Cx_Deinit(I2C_t * i2c);

/*
 * PRIVATE VARIABLES
 */

#ifdef I2C1_PINS
static I2C_t gI2C_1 = {
	.Instance = I2C1
};
I2C_t * I2C_1 = &gI2C_1;
#endif
#ifdef I2C2_PINS
static I2C_t gI2C_2 = {
	.Instance = I2C2
};
I2C_t * I2C_2 = &gI2C_2;
#endif
#ifdef I2C3_PINS
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

	i2c->handle.Instance = i2c->Instance;
	i2c->handle.Init = (I2C_InitTypeDef){
		.ClockSpeed = mode,
		.DutyCycle = I2C_DUTYCYCLE_16_9,
		.AddressingMode = I2C_ADDRESSINGMODE_7BIT,
		.DualAddressMode = I2C_DUALADDRESS_DISABLE,
		.GeneralCallMode = I2C_GENERALCALL_DISABLE,
		.NoStretchMode = I2C_NOSTRETCH_DISABLE,
	};

	HAL_I2C_Init(&i2c->handle);
}

void I2C_Deinit(I2C_t * i2c)
{
	__HAL_I2C_DISABLE(i2c);

	I2Cx_Deinit(i2c);
	i2c->mode = 0;
}

bool I2C_Write(I2C_t * i2c, uint8_t address, const uint8_t * data, uint32_t count)
{
	return HAL_I2C_Master_Transmit(&i2c->handle, address << 1, (uint8_t*)data, count, 50) == HAL_OK;
}

bool I2C_Read(I2C_t * i2c, uint8_t address, uint8_t * data, uint32_t count)
{
	return HAL_I2C_Master_Receive(&i2c->handle, (address << 1) | 0x01, data, count, 50) == HAL_OK;
}

bool I2C_Transfer(I2C_t * i2c, uint8_t address, const uint8_t * txdata, uint32_t txcount, uint8_t * rxdata, uint32_t rxcount)
{
	return I2C_Write(i2c, address, txdata, txcount)
		&& I2C_Read(i2c, address, rxdata, rxcount);
}

bool I2C_Scan(I2C_t * i2c, uint8_t address)
{
	return I2C_Write(i2c, address, NULL, 0);
}

/*
 * PRIVATE FUNCTIONS
 */



static void I2Cx_Init(I2C_t * i2c)
{
#ifdef I2C1_PINS
	if (i2c == I2C_1)
	{
		__HAL_RCC_I2C1_CLK_ENABLE();
		GPIO_EnableAlternate(I2C1_PINS, GPIO_Flag_OpenDrain, I2C1_AF);
	}
#endif
#ifdef I2C2_PINS
	if (i2c == I2C_2)
	{
		__HAL_RCC_I2C2_CLK_ENABLE();
		GPIO_EnableAlternate(I2C2_PINS, GPIO_Flag_OpenDrain, I2C2_AF);
	}
#endif
#ifdef I2C3_PINS
	if (i2c == I2C_3)
	{
		__HAL_RCC_I2C3_CLK_ENABLE();
		GPIO_EnableAlternate(I2C3_PINS, GPIO_Flag_OpenDrain, I2C3_AF);
	}
#endif
}

static void I2Cx_Deinit(I2C_t * i2c)
{
#ifdef I2C1_PINS
	if (i2c == I2C_1)
	{
		__HAL_RCC_I2C1_CLK_DISABLE();
		GPIO_Deinit(I2C1_PINS);
	}
#endif
#ifdef I2C2_PINS
	if (i2c == I2C_2)
	{
		__HAL_RCC_I2C2_CLK_DISABLE();
		GPIO_Deinit(I2C2_PINS);
	}
#endif
#ifdef I2C3_PINS
	if (i2c == I2C_3)
	{
		__HAL_RCC_I2C3_CLK_DISABLE();
		GPIO_Deinit(I2C3_PINS);
	}
#endif
}

#ifdef I2C_USE_FASTMODEPLUS
static uint32_t I2Cx_GetFMPBit(I2C_t * i2c)
{
	uint32_t bit;
#ifdef I2C1_PINS
	if (i2c == I2C_1)
	{
		bit = I2C_FASTMODEPLUS_I2C1;
	}
#endif
#ifdef I2C2_PINS
	if (i2c == I2C_2)
	{
		bit = I2C_FASTMODEPLUS_I2C2;
	}
#endif
#ifdef I2C3_PINS
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

