
#include "SPI.h"
#include "Core.h"

/*
 * PRIVATE DEFINITIONS
 */

#define _SPI_RX(spi) 		(*(__IO uint8_t *)&spi->Instance->DR)
#define _SPI_TX(spi, b) 	*(__IO uint8_t *)&spi->Instance->DR = (b)

/*
 * PRIVATE TYPES
 */

/*
 * PRIVATE PROTOTYPES
 */

static void SPIx_Init(SPI_t * spi);
static void SPIx_Deinit(SPI_t * spi);
static uint32_t SPI_SelectPrescalar(SPI_t * spi, uint32_t target);

/*
 * PRIVATE VARIABLES
 */

#ifdef SPI1_GPIO
static SPI_t gSPI_1 = {
	.Instance = SPI1
};
SPI_t * SPI_1 = &gSPI_1;
#endif
#ifdef SPI2_GPIO
static SPI_t gSPI_2 = {
	.Instance = SPI2
};
SPI_t * SPI_2 = &gSPI_2;
#endif

/*
 * PUBLIC FUNCTIONS
 */

void SPI_Init(SPI_t * spi, uint32_t bitrate, SPIMode_t mode)
{
	SPIx_Init(spi);

	__HAL_SPI_DISABLE(spi);

	uint32_t prescalar = SPI_SelectPrescalar(spi, bitrate);

	spi->Instance->CR1 =  SPI_MODE_MASTER | SPI_DIRECTION_2LINES | SPI_DATASIZE_8BIT | mode
						| (SPI_NSS_SOFT & SPI_CR1_SSM) | prescalar | SPI_FIRSTBIT_MSB | SPI_CRCCALCULATION_DISABLE;

	spi->Instance->CR2 = ((SPI_NSS_SOFT >> 16u) & SPI_CR2_SSOE) | SPI_TIMODE_DISABLE;
	spi->Instance->I2SCFGR &= ~SPI_I2SCFGR_I2SMOD;

	__HAL_SPI_ENABLE(spi);
}

void SPI_Deinit(SPI_t * spi)
{
	__HAL_SPI_DISABLE(spi);
	SPIx_Deinit(spi);
}

void SPI_Tx(SPI_t * spi, const uint8_t * data, uint32_t count)
{
	for (uint32_t i = 0; i < count; i++)
	{
		while (!__HAL_SPI_GET_FLAG(spi, SPI_FLAG_TXE));
		_SPI_TX(spi, data[i]);
	}
	while (__HAL_SPI_GET_FLAG(spi, SPI_FLAG_BSY));
	__HAL_SPI_CLEAR_OVRFLAG(spi);
}

void SPI_Rx(SPI_t * spi, uint8_t * data, uint32_t count)
{
	for (uint32_t i = 0; i < count; i++)
	{
		while (!__HAL_SPI_GET_FLAG(spi, SPI_FLAG_TXE));
		_SPI_TX(spi, 0xFF);
		while (!__HAL_SPI_GET_FLAG(spi, SPI_FLAG_RXNE));
		data[i] = _SPI_RX(spi);
	}
	while (__HAL_SPI_GET_FLAG(spi, SPI_FLAG_BSY));
	__HAL_SPI_CLEAR_OVRFLAG(spi);
}

void SPI_TxRx(SPI_t * spi, const uint8_t * txdata, uint8_t * rxdata, uint32_t count)
{
	for (uint32_t i = 0; i < count; i++)
	{
		while (!__HAL_SPI_GET_FLAG(spi, SPI_FLAG_TXE));
		_SPI_TX(spi, txdata[i]);
		while (!__HAL_SPI_GET_FLAG(spi, SPI_FLAG_RXNE));
		rxdata[i] = _SPI_RX(spi);
	}
	while (__HAL_SPI_GET_FLAG(spi, SPI_FLAG_BSY));
	__HAL_SPI_CLEAR_OVRFLAG(spi);
}

uint8_t SPI_Xfer(SPI_t * spi, uint8_t byte)
{
	while (!__HAL_SPI_GET_FLAG(spi, SPI_FLAG_TXE));
	_SPI_TX(spi, byte);
	while (!__HAL_SPI_GET_FLAG(spi, SPI_FLAG_RXNE));
	return _SPI_RX(spi);
}

/*
 * PRIVATE FUNCTIONS
 */

static uint32_t SPI_SelectPrescalar(SPI_t * spi, uint32_t target)
{
	// Div clock by 2, because the prescalars start at 2
	uint32_t clk = HAL_RCC_GetPCLK1Freq() >> 1;
	uint32_t actual;
	uint32_t k;
	for (k = 0; k <= 0x7; k++)
	{
		actual = clk >> k;
		if (actual <= target)
		{
			break;
		}
	}
	spi->bitrate = actual;
	return k << SPI_CR1_BR_Pos;
}

static void SPIx_Init(SPI_t * spi)
{
	GPIO_InitTypeDef gpio = {0};
	gpio.Mode = GPIO_MODE_AF_PP;
	gpio.Pull = GPIO_NOPULL;
	gpio.Speed = GPIO_SPEED_FREQ_HIGH;

#ifdef SPI1_GPIO
	if (spi == SPI_1)
	{
		__HAL_RCC_SPI1_CLK_ENABLE();
		gpio.Pin = SPI1_PINS;
		gpio.Alternate = SPI1_AF;
		HAL_GPIO_Init(SPI1_GPIO, &gpio);
	}
#endif
#ifdef SPI2_GPIO
	if (spi == SPI_2)
	{
		__HAL_RCC_SPI2_CLK_ENABLE();
		gpio.Pin = SPI2_PINS;
		gpio.Alternate = SPI2_AF;
		HAL_GPIO_Init(SPI2_GPIO, &gpio);
	}
#endif
}

static void SPIx_Deinit(SPI_t * spi)
{
#ifdef SPI1_GPIO
	if (spi == SPI_1)
	{
		__HAL_RCC_SPI1_CLK_DISABLE();
		HAL_GPIO_DeInit(SPI1_GPIO, SPI1_PINS);
	}
#endif
#ifdef SPI2_GPIO
	if (spi == SPI_2)
	{
		__HAL_RCC_SPI2_CLK_DISABLE();
		HAL_GPIO_DeInit(SPI2_GPIO, SPI2_PINS);
	}
#endif
}

/*
 * INTERRUPT ROUTINES
 */

