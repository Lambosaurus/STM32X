
#include "I2C.h"
#include "Core.h"
#include "GPIO.h"

/*
 * PRIVATE DEFINITIONS
 */

#define I2C_SCL_SYNC_CYCLES		3

#ifndef I2C_DIGITALFILTER_SIZE
#define I2C_DIGITALFILTER_SIZE	0
#endif
#define I2C_USE_ANALOGFILTER

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
	
	i2c->Instance->TIMINGR = I2C_SelectTiming(mode);
	i2c->Instance->CR1 = I2C_NOSTRETCH_DISABLE | I2C_ANALOGFILTER_ENABLE | (I2C_DIGITALFILTER_SIZE << 8);
	i2c->Instance->CR2 = I2C_CR2_AUTOEND | I2C_CR2_NACK;
	
	// Disable own addressing modes.
	i2c->Instance->OAR1 = 0;
	i2c->Instance->OAR2 = 0;
	//i2c->Instance->OAR1 &= ~I2C_OAR1_OA1EN;
	//i2c->Instance->OAR1 = I2C_OAR1_OA1EN | ownAddress1;
	//i2c->Instance->OAR2 &= ~I2C_OAR2_OA2EN;
	//i2c->Instance->OAR2 = I2C_OAR2_OA2EN | ownAddress2 | SMBUS_OA2_NOMASK;
	
	__HAL_I2C_ENABLE(i2c);
}

void I2C_Deinit(I2C_t * i2c)
{
	I2Cx_Deinit(i2c);
}

bool I2C_Tx(I2C_t * i2c, uint8_t address, const uint8_t * data, uint32_t count);
bool I2C_Rx(I2C_t * i2c, uint8_t address, uint8_t * data, uint32_t count);
bool I2C_TxRx(I2C_t * i2c, uint8_t address, const uint8_t * txdata, uint8_t * rxdata, uint32_t count);
bool I2C_Scan(I2C_t * i2c, uint8_t address);

/*
 * PRIVATE FUNCTIONS
 */

#define NS_TO_CYCLES(clk, ns)		(clk/(1000000000/ns))

static uint32_t I2C_SelectTiming(I2C_Mode_t mode)
{
	uint32_t clk = HAL_RCC_GetPCLK1Freq();

	uint32_t freq;
	uint32_t scl_delay;
	uint32_t sda_delay = 0;

	switch(mode)
	{
	default:
	case I2C_Mode_Standard:
		scl_delay = NS_TO_CYCLES(clk, 250);
		freq = 100000;
		break;
	case I2C_Mode_Fast:
		scl_delay = NS_TO_CYCLES(clk, 100);
		freq = 400000;
		break;
	case I2C_Mode_FastPlus:
		scl_delay = NS_TO_CYCLES(clk, 50);
		freq = 1000000;
		break;
	}

	uint32_t prescalar = 0;
	uint32_t bittime = clk / freq;

	uint32_t scl_l = 0;
	uint32_t scl_h = 0;


	return (prescalar << 28) | (scl_del << 20) | (sda_del << 16) | (scl_h << 8) | scl_l;
}

static void I2Cx_Init(I2C_t * i2c)
{
	GPIO_InitTypeDef gpio = {0};
	gpio.Mode = GPIO_MODE_AF_PP;
	gpio.Pull = GPIO_NOPULL;
	gpio.Speed = GPIO_SPEED_FREQ_HIGH;

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

