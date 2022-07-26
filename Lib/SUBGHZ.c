
#include "SUBGHZ.h"

#ifdef SUBGHZ_ENABLED
#include "CLK.h"
#include "Core.h"
#include "US.h"

/*
 * PRIVATE DEFINITIONS
 */

#define SUBGHZ_TIMEOUT		100

#define SUBGHZ_SPI_SELECT 	LL_PWR_SelectSUBGHZSPI_NSS
#define SUBGHZ_SPI_DESELECT	LL_PWR_UnselectSUBGHZSPI_NSS

/*
 * PRIVATE TYPES
 */

/*
 * PRIVATE PROTOTYPES
 */

static void SUBGHZ_SPI_Init(void);
static void SUBGHZ_SPI_Deinit(void);
static void SUBGHZ_SPI_Write(const uint8_t * data, uint32_t count);
static void SUBGHZ_SPI_Read(uint8_t * data, uint32_t count);

// All transactions follow a write-read or write-write structure.
static void SUBGHZ_SPI_WriteTransaction(const uint8_t * tx1, uint32_t tx1_len, const uint8_t * tx2, uint32_t tx2_len);
static void SUBGHZ_SPI_ReadTransaction(const uint8_t * tx, uint32_t tx_len, uint8_t * rx, uint32_t rx_len);

static void SUBGHZ_CheckDeviceReady(void);
static void SUBGHZ_WaitOnBusy(void);

/*
 * PRIVATE VARIABLES
 */

static struct {
	bool deep_sleep;
	SUBGHZ_Callback_t callback;
} gSubghz;

/*
 * PUBLIC FUNCTIONS
 */

void SUBGHZ_Init(void)
{
	__HAL_RCC_SUBGHZSPI_CLK_ENABLE();
	HAL_NVIC_EnableIRQ(SUBGHZ_Radio_IRQn);

	LL_RCC_RF_DisableReset();
	while (LL_RCC_IsRFUnderReset());

	LL_PWR_UnselectSUBGHZSPI_NSS();

#if defined(CM0PLUS)
	LL_C2_EXTI_EnableIT_32_63(LL_EXTI_LINE_44);
	LL_C2_PWR_SetRadioBusyTrigger(LL_PWR_RADIO_BUSY_TRIGGER_WU_IT);
#else
	LL_EXTI_EnableIT_32_63(LL_EXTI_LINE_44);
	LL_PWR_SetRadioBusyTrigger(LL_PWR_RADIO_BUSY_TRIGGER_WU_IT);
#endif // CM0PLUS

	LL_PWR_ClearFlag_RFBUSY();

	SUBGHZ_SPI_Init();

	gSubghz.deep_sleep = true;
}

void SUBGHZ_Deinit(void)
{
	SUBGHZ_SPI_Deinit();
	HAL_NVIC_DisableIRQ(SUBGHZ_Radio_IRQn);
	__HAL_RCC_SUBGHZSPI_CLK_DISABLE();
}

void SUBGHZ_OnEvent(SUBGHZ_Callback_t callback)
{
	gSubghz.callback = callback;
}

void SUBGHZ_ExecuteSet(SUBGHZ_RadioSetCmd_t cmd, const uint8_t * bfr, uint32_t size)
{
	SUBGHZ_CheckDeviceReady();

	gSubghz.deep_sleep = (cmd == RADIO_SET_SLEEP) || (cmd == RADIO_SET_RXDUTYCYCLE);

	uint8_t tx[] = { cmd };
	SUBGHZ_SPI_SELECT();
	SUBGHZ_SPI_Write(tx, sizeof(tx));
	SUBGHZ_SPI_Write(bfr, size);
	SUBGHZ_SPI_DESELECT();

	if (cmd != RADIO_SET_SLEEP)
	{
		SUBGHZ_WaitOnBusy();
	}

}

void SUBGHZ_ExecuteGet(SUBGHZ_RadioGetCmd_t cmd, uint8_t * bfr, uint32_t size)
{
	uint8_t tx[] = {
			cmd,
			0x00
	};
	SUBGHZ_SPI_ReadTransaction( tx, sizeof(tx), bfr, size);
}

void SUBGHZ_ReadBuffer(uint8_t offset, uint8_t * bfr, uint32_t size)
{
	uint8_t tx[] = {
		SUBGHZ_RADIO_READ_BUFFER,
		offset,
		0x00,
	};
	SUBGHZ_SPI_ReadTransaction(tx, sizeof(tx), bfr, size);
}

void SUBGHZ_WriteBuffer(uint8_t offset, const uint8_t * bfr, uint32_t size)
{
	uint8_t tx[] = {
		SUBGHZ_RADIO_WRITE_BUFFER,
		offset,
	};
	SUBGHZ_SPI_WriteTransaction(tx, sizeof(tx), bfr, size);
}

void SUBGHZ_ReadRegister(uint16_t address, uint8_t * value)
{
	SUBGHZ_ReadRegisters(address, value, 1);
}

void SUBGHZ_WriteRegister(uint16_t address, uint8_t value)
{
	SUBGHZ_WriteRegisters(address, &value, 1);
}

void SUBGHZ_ReadRegisters(uint16_t address, uint8_t * value, uint32_t count)
{
	uint8_t tx[] = {
			SUBGHZ_RADIO_READ_REGISTER,
			(uint8_t)(address >> 8),
			(uint8_t)address,
			0x00
	};

	SUBGHZ_SPI_ReadTransaction(tx, sizeof(tx), value, count);
}

void SUBGHZ_WriteRegisters(uint16_t address, const uint8_t * value, uint32_t count)
{
	uint8_t tx[] = {
			SUBGHZ_RADIO_WRITE_REGISTER,
			(uint8_t)(address >> 8),
			(uint8_t)address,
	};

	SUBGHZ_SPI_WriteTransaction(tx, sizeof(tx), value, count);
}

/*
 * PRIVATE FUNCTIONS
 */

static void SUBGHZ_SPI_WriteTransaction(const uint8_t * tx1, uint32_t tx1_len, const uint8_t * tx2, uint32_t tx2_len)
{
	SUBGHZ_CheckDeviceReady();
	SUBGHZ_SPI_SELECT();
	SUBGHZ_SPI_Write(tx1, tx1_len);
	SUBGHZ_SPI_Write(tx2, tx2_len);
	SUBGHZ_SPI_DESELECT();
	SUBGHZ_WaitOnBusy();
}

static void SUBGHZ_SPI_ReadTransaction(const uint8_t * tx, uint32_t tx_len, uint8_t * rx, uint32_t rx_len)
{
	SUBGHZ_CheckDeviceReady();
	SUBGHZ_SPI_SELECT();
	SUBGHZ_SPI_Write(tx, tx_len);
	SUBGHZ_SPI_Read(rx, rx_len);
	SUBGHZ_SPI_DESELECT();
	SUBGHZ_WaitOnBusy();
}

static void SUBGHZ_SPI_Init(void)
{
	// Disable
	CLEAR_BIT(SUBGHZSPI->CR1, SPI_CR1_SPE);

	// Select the highest presclar from 2 to 256 that does not exceed the target bitrate
	uint32_t bitrate = 16000000;
	uint32_t prescalar = CLK_SelectPrescalar(CLK_GetHCLKFreq(), 2, 256, &bitrate) << SPI_CR1_BR_Pos;

	WRITE_REG(SUBGHZSPI->CR1, (SPI_CR1_MSTR | SPI_CR1_SSI | prescalar | SPI_CR1_SSM));
	WRITE_REG(SUBGHZSPI->CR2, (SPI_CR2_FRXTH |  SPI_CR2_DS_0 | SPI_CR2_DS_1 | SPI_CR2_DS_2));

	// Enable
	SET_BIT(SUBGHZSPI->CR1, SPI_CR1_SPE);
}

static void SUBGHZ_SPI_Deinit(void)
{
	CLEAR_BIT(SUBGHZSPI->CR1, SPI_CR1_SPE);
}

static void SUBGHZ_SPI_Write(const uint8_t * data, uint32_t count)
{
	while (count--)
	{
		// Wait for TXE, then write the data
		while (!READ_BIT(SUBGHZSPI->SR, SPI_SR_TXE));
		// Not sure why this has to be an 8 bit read?
		*((volatile uint8_t *)&SUBGHZSPI->DR) = *data++;

		// Wait for RXNE then flush read data
		while (!READ_BIT(SUBGHZSPI->SR, SPI_SR_RXNE));
		(void)READ_REG(SUBGHZSPI->DR);
	}
}


static void SUBGHZ_SPI_Read(uint8_t * data, uint32_t count)
{
	while (count--)
	{
		// Wait for TXE, then write dummy data
		while (!READ_BIT(SUBGHZSPI->SR, SPI_SR_TXE));
		*((volatile uint8_t *)&SUBGHZSPI->DR) = 0xFF;

		// Wait for RXNE, then read the data
		while (!READ_BIT(SUBGHZSPI->SR, SPI_SR_RXNE));
		*data++ = (uint8_t)READ_REG(SUBGHZSPI->DR);
	}
}

static void SUBGHZ_CheckDeviceReady(void)
{
	if (gSubghz.deep_sleep)
	{
		SUBGHZ_SPI_SELECT();
		US_Delay(20);
		SUBGHZ_SPI_DESELECT();
	}
	return SUBGHZ_WaitOnBusy();
}

static void SUBGHZ_WaitOnBusy(void)
{
	uint32_t start = CORE_GetTick();

	while (CORE_GetTick() - start < SUBGHZ_TIMEOUT)
	{
		uint32_t mask = LL_PWR_IsActiveFlag_RFBUSYMS();
		if ((LL_PWR_IsActiveFlag_RFBUSYS() & mask) != 0x01)
		{
			break;
		}
	}
}

/*
 * INTERRUPT ROUTINES
 */

void SUBGHZ_Radio_IRQHandler(void)
{
	uint8_t isr_data[2];
	SUBGHZ_ExecuteGet(RADIO_GET_IRQSTATUS, isr_data, sizeof(isr_data));
	uint16_t itsource = isr_data[0] << 8 | isr_data[1];

	if (itsource & SUBGHZ_IT_TX_CPLT)
	{
		gSubghz.callback(SUBGHZ_Event_TxComplete);
	}

	if (itsource & SUBGHZ_IT_RX_CPLT)
	{
		gSubghz.callback(SUBGHZ_Event_RxComplete);
	}

	if (itsource & SUBGHZ_IT_PREAMBLE_DETECTED)
	{
		gSubghz.callback(SUBGHZ_Event_PreambleDetected);
	}

	if (itsource & SUBGHZ_IT_SYNCWORD_VALID)
	{
		gSubghz.callback(SUBGHZ_Event_SyncwordValid);
	}

	if (itsource & SUBGHZ_IT_HEADER_VALID)
	{
		gSubghz.callback(SUBGHZ_Event_HeaderValid);
	}

	if (itsource & SUBGHZ_IT_HEADER_ERROR)
	{
		gSubghz.callback(SUBGHZ_Event_HeaderError);
	}

	if (itsource & SUBGHZ_IT_CRC_ERROR)
	{
		gSubghz.callback(SUBGHZ_Event_CRCError);
	}

	if (itsource & SUBGHZ_IT_CAD_DONE)
	{
		if (itsource & SUBGHZ_IT_CAD_ACTIVITY_DETECTED)
		{
			gSubghz.callback(SUBGHZ_Event_CADDetected);
		}
		else
		{
			gSubghz.callback(SUBGHZ_Event_CADClear);
		}
	}

	if (itsource & SUBGHZ_IT_RX_TX_TIMEOUT)
	{
		gSubghz.callback(SUBGHZ_Event_RxTxTimeout);
	}

	SUBGHZ_ExecuteSet(RADIO_CLR_IRQSTATUS, isr_data, sizeof(isr_data));
}


#endif // SUBGHZ_ENABLED
