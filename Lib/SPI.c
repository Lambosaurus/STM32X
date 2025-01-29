
#include "SPI.h"
#include "GPIO.h"
#include "CLK.h"

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

#ifdef SPI1_PINS
static SPI_t gSPI_1 = {
	.Instance = SPI1
};
SPI_t * const SPI_1 = &gSPI_1;
#endif
#ifdef SPI2_PINS
static SPI_t gSPI_2 = {
	.Instance = SPI2
};
SPI_t * const SPI_2 = &gSPI_2;
#endif
#ifdef SPI3_PINS
static SPI_t gSPI_3 = {
	.Instance = SPI3
};
SPI_t * const SPI_3 = &gSPI_3;
#endif


/*
 * PUBLIC FUNCTIONS
 */

void SPI_Init(SPI_t * spi, uint32_t bitrate, SPI_Mode_t mode)
{
	SPIx_Init(spi);

	__HAL_SPI_DISABLE(spi);

	uint32_t prescalar = SPI_SelectPrescalar(spi, bitrate);

	uint32_t cr1 = SPI_MODE_MASTER | SPI_DIRECTION_2LINES | mode
			| (SPI_NSS_SOFT & SPI_CR1_SSM) | prescalar | SPI_FIRSTBIT_MSB | SPI_CRCCALCULATION_DISABLE;
	uint32_t cr2 = ((SPI_NSS_SOFT >> 16u) & SPI_CR2_SSOE) | SPI_TIMODE_DISABLE;

#ifdef STM32G0
	cr2 |= SPI_DATASIZE_8BIT | SPI_RXFIFO_THRESHOLD_QF;
#else
	cr1 |= SPI_DATASIZE_8BIT;
#endif

	spi->Instance->CR1 = cr1;
	spi->Instance->CR2 = cr2;

#ifdef SPI_I2SCFGR_I2SMOD
	// This register is only defined on some cores.
	spi->Instance->I2SCFGR &= ~SPI_I2SCFGR_I2SMOD;
#endif

	__HAL_SPI_ENABLE(spi);
}

void SPI_Deinit(SPI_t * spi)
{
	__HAL_SPI_DISABLE(spi);
	SPIx_Deinit(spi);
}

void SPI_Write(SPI_t * spi, const uint8_t * data, uint32_t count)
{
	for (uint32_t i = 0; i < count; i++)
	{
		while (!__HAL_SPI_GET_FLAG(spi, SPI_FLAG_TXE));
		_SPI_TX(spi, data[i]);
		while (!__HAL_SPI_GET_FLAG(spi, SPI_FLAG_RXNE));
		(void)_SPI_RX(spi);
	}
	while (__HAL_SPI_GET_FLAG(spi, SPI_FLAG_BSY));
	__HAL_SPI_CLEAR_OVRFLAG(spi);
}

void SPI_Read(SPI_t * spi, uint8_t * data, uint32_t count)
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

void SPI_Transfer(SPI_t * spi, const uint8_t * txdata, uint8_t * rxdata, uint32_t count)
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

uint8_t SPI_TransferByte(SPI_t * spi, uint8_t byte)
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
	uint32_t src_freq = CLK_GetPCLKFreq();
	spi->bitrate = target;
	uint32_t k = CLK_SelectPrescalar(src_freq, 2, 2 << 7, &spi->bitrate);
	return k << SPI_CR1_BR_Pos;
}

static void SPIx_Init(SPI_t * spi)
{
#ifdef SPI1_PINS
	if (spi == SPI_1)
	{
		__HAL_RCC_SPI1_CLK_ENABLE();
		GPIO_EnableAlternate(SPI1_PINS, 0, SPI1_AF);
	}
#endif
#ifdef SPI2_PINS
	if (spi == SPI_2)
	{
		__HAL_RCC_SPI2_CLK_ENABLE();
		GPIO_EnableAlternate(SPI2_PINS, 0, SPI2_AF);
	}
#endif
#ifdef SPI3_PINS
	if (spi == SPI_3)
	{
		__HAL_RCC_SPI3_CLK_ENABLE();
		GPIO_EnableAlternate(SPI3_PINS, 0, SPI3_AF);
	}
#endif
}

static void SPIx_Deinit(SPI_t * spi)
{
#ifdef SPI1_PINS
	if (spi == SPI_1)
	{
		__HAL_RCC_SPI1_CLK_DISABLE();
		GPIO_Deinit(SPI1_PINS);
	}
#endif
#ifdef SPI2_PINS
	if (spi == SPI_2)
	{
		__HAL_RCC_SPI2_CLK_DISABLE();
		GPIO_Deinit(SPI2_PINS);
	}
#endif
#ifdef SPI3_PINS
	if (spi == SPI_3)
	{
		__HAL_RCC_SPI3_CLK_DISABLE();
		GPIO_Deinit(SPI3_PINS);
	}
#endif
}

/*
 * INTERRUPT ROUTINES
 */

