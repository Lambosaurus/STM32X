
#include "CAN.h"

#ifdef CAN_GPIO
#include "GPIO.h"

/*
 * PRIVATE DEFINITIONS
 */

#define FILTER_BANK_COUNT			14
#define _CAN_RX_FIFO0_COUNT(can) 	(can->RF0R & CAN_RF0R_FMP0)
#define _CAN_RX_FIFO1_COUNT(can) 	(can->RF1R & CAN_RF0R_FMP0)

#define _CAN_RESET(can)				(can->MCR |= CAN_MCR_RESET)

#ifndef CAN_SEG1_TQ
#define AUTO_CALC_TQ
#endif

// CAN_MCR_TTCM: Enabled time based transmit
// CAN_MCR_AWUM: Enabled the automatic wake-up mode. Probably good in future.
// CAN_MCR_NART: Disables automatic retransmission
// CAN_MCR_RFLM: Enables the receive FIFO locked mode
// CAN_MCR_TXFP: Set TX priority based on mailbox position, not ID
// CAN_MCR_ABOM: Enable automatic bus-off management
#define CAN_FEATURE_BITS			(CAN_MCR_TTCM | CAN_MCR_AWUM | CAN_MCR_NART | CAN_MCR_RFLM | CAN_MCR_TXFP | CAN_MCR_ABOM)
#define CAN_ENABLED_FEATURES		(CAN_MCR_ABOM)

/*
 * PRIVATE TYPES
 */

/*
 * PRIVATE PROTOTYPES
 */

static void CANx_Init(void);
static void CANx_Deinit(void);
static void CAN_ReadMailbox(CAN_FIFOMailBox_TypeDef * mailbox, CANMsg_t * msg);
static void CAN_WriteMailbox(CAN_TxMailBox_TypeDef * mailbox, const CANMsg_t * msg);
#ifdef AUTO_CALC_TQ
static uint32_t CAN_SelectNominalBitTime(uint32_t base_freq, uint32_t bitrate);
#endif

/*
 * PRIVATE VARIABLES
 */

/*
 * PUBLIC FUNCTIONS
 */

void CAN_Init(uint32_t bitrate)
{

	CANx_Init();

	uint32_t freq = HAL_RCC_GetPCLK1Freq();

#ifdef AUTO_CALC_TQ
	uint32_t nbt = CAN_SelectNominalBitTime(freq, bitrate);
	uint32_t prescalar = freq / (bitrate * nbt);
	uint32_t ts2 = (nbt - 1) * 1 / 3;
	uint32_t ts1 = (nbt - 1) - ts2;   		// for nbt = 8 this gives 5 / 2
	uint32_t sjw = ts1 >= 8 ? 4 : ts1 / 2; 	// for nbt = 8 this gives 2.
#else
	uint32_t ts1 = CAN_SEG1_TQ;
	uint32_t ts2 = CAN_SEG2_TQ;
	uint32_t sjw = CAN_SJW_TQ;
	uint32_t prescalar = freq / (bitrate * (CAN_SEG1_TQ + CAN_SEG2_TQ + 1));
#endif

	CLEAR_BIT(CAN->MCR, CAN_MCR_SLEEP);
	while ((CAN->MSR & CAN_MSR_SLAK) != 0U);

	// Request initialisation
	SET_BIT(CAN->MCR, CAN_MCR_INRQ);
	while ((CAN->MSR & CAN_MSR_INAK) == 0U);

	MODIFY_REG(CAN->MCR, CAN_FEATURE_BITS, CAN_ENABLED_FEATURES);

	// Timing bit register.
	CAN->BTR = CAN_MODE_NORMAL | ((sjw - 1) << CAN_BTR_SJW_Pos) | ((ts1 - 1) << CAN_BTR_TS1_Pos) | ((ts2 - 1) << CAN_BTR_TS2_Pos) | (prescalar - 1U);

	// Clear init mode - this starts everything.
	CAN->MCR &= ~CAN_MCR_INRQ;
	// Wait for ack. This is probably not required.
	// Unsure if it messes with subsequent TX.
	while (CAN->MSR & CAN_MSR_INAK);
}

void CAN_EnableFilter(uint32_t bank, uint32_t id, uint32_t mask)
{
	uint32_t filter_bit = 1 << (bank & 0x1FU);

    // Enable filter initialisation
	CAN->FMR |= CAN_FMR_FINIT;
    // Disable filter
	CAN->FA1R &= ~filter_bit;

    // Enable 32-bit filter mode (as opposed to dual 16 bit filters)
	CAN->FS1R |= filter_bit;
	CAN->sFilterRegister[bank].FR1 = id;
	CAN->sFilterRegister[bank].FR2 = mask;

    // Enable ID/Mask mode. Note: we could clear this for id list mode.
	CAN->FM1R &= ~filter_bit;

#ifdef CAN_DUAL_FIFO
    if (bank & 0x01)
    {
    	// Odd banks will be routed to FIFO1 as load balancing.
    	CAN->FFA1R |= filter_bit;
    }
    else
    {
    	CAN->FFA1R &= ~filter_bit;
    }
#else
    // Route to FIFO0
    CAN->FFA1R &= ~filter_bit;
#endif

    // Enable filter
    CAN->FA1R |= filter_bit;
    // Clear init mode
    CAN->FMR &= ~CAN_FMR_FINIT;
}

void CAN_Deinit(void)
{
	// Request initialisation (stop the tx/rx), then wait for ack
	CAN->MCR |= CAN_MCR_INRQ;
	while (!(CAN->MSR & CAN_MSR_INAK));
	// Exit sleep mode. (Is this required?)
	CAN->MCR &= ~CAN_MCR_SLEEP;

	_CAN_RESET(CAN);
	CANx_Deinit();
}

bool CAN_Write(const CANMsg_t * msg)
{
	uint32_t tsr = CAN->TSR;

	if ((tsr & (CAN_TSR_TME0 | CAN_TSR_TME1 | CAN_TSR_TME2)) == 0)
	{
		// All Tx mailboxes are full.
		return false;
	}

	uint32_t free_mailbox = (tsr & CAN_TSR_CODE) >> CAN_TSR_CODE_Pos;
	CAN_WriteMailbox(&CAN->sTxMailBox[free_mailbox], msg);

	return true;
}


uint8_t CAN_ReadCount()
{
#ifdef CAN_DUAL_FIFO
	return _CAN_RX_FIFO0_COUNT(CAN) + _CAN_RX_FIFO1_COUNT(CAN);
#endif
	return _CAN_RX_FIFO0_COUNT(CAN);
}

bool CAN_Read(CANMsg_t * msg)
{
	if (_CAN_RX_FIFO0_COUNT(CAN))
	{
		CAN_ReadMailbox(&CAN->sFIFOMailBox[CAN_RX_FIFO0], msg);

		// Release the FIFO
		CAN->RF0R |= CAN_RF0R_RFOM0;
		return true;
	}
#ifdef CAN_DUAL_FIFO
	if (_CAN_RX_FIFO1_COUNT())
	{
		CAN_ReadMailbox(&CAN->sFIFOMailBox[CAN_RX_FIFO1], msg);

		// Release the FIFO
		CAN->RF1R |= CAN_RF0R_RFOM0;
		return true;
	}
#endif
	return false;
}

/*
 * PRIVATE FUNCTIONS
 */

#ifdef AUTO_CALC_TQ
static uint32_t CAN_SelectNominalBitTime(uint32_t base_freq, uint32_t bitrate)
{
	uint32_t bestNbt;
	uint32_t bestDelta = 0xFFFFFFFF;

	for (int nbt = 8; nbt < 25; nbt++)
	{
		uint32_t prescalar = base_freq / (bitrate * nbt);
		uint32_t actual_bitrate = base_freq / (nbt * prescalar);
		// Prescalar will undershoot, so actual_bitrate will always overshoot.
		// Therefore an unsigned operation is ok
		uint32_t delta = actual_bitrate - bitrate;
		if (delta == 0)
		{
			// On most bitrates this should just return immediately on 8.
			return nbt;
		}
		else if (delta < bestDelta)
		{
			bestNbt = nbt;
			bestDelta = delta;
		}

	}
	return bestNbt;
}
#endif

static void CAN_ReadMailbox(CAN_FIFOMailBox_TypeDef * mailbox, CANMsg_t * msg)
{
	uint32_t rir = mailbox->RIR;
	if (rir & CAN_RI0R_IDE)
	{
		// Extended IDE
		msg->id = (rir & (CAN_RI0R_EXID | CAN_RI0R_STID)) >> CAN_RI0R_EXID_Pos;
	}
	else
	{
		// Standard IDE (Trash)
		msg->id = (rir & CAN_RI0R_STID) >> CAN_TI0R_STID_Pos;
	}

	msg->len = (mailbox->RDTR & CAN_RDT0R_DLC) >> CAN_RDT0R_DLC_Pos;

	// Note: we rely on the packing alignment of the CANMsg_t for this to work.
	uint32_t * data = (uint32_t *)&msg->data;
	data[0] = mailbox->RDLR;
	data[1] = mailbox->RDHR;
}

static void CAN_WriteMailbox(CAN_TxMailBox_TypeDef * mailbox, const CANMsg_t * msg)
{
	// Only support std ide.
	mailbox->TIR = (msg->id << CAN_TI0R_EXID_Pos) | CAN_TI0R_IDE;
	mailbox->TDTR = msg->len;

	// Note: we rely on the packing alignment of the CANMsg_t for this to work.
	uint32_t * data = (uint32_t *)&msg->data;
	mailbox->TDHR = data[1];
	mailbox->TDLR = data[0];

	// Request transmission.
	mailbox->TIR |= CAN_TI0R_TXRQ;
}

static void CANx_Init(void)
{
	__HAL_RCC_CAN1_CLK_ENABLE();
	GPIO_EnableAlternate(CAN_GPIO, CAN_PINS, GPIO_MODE_AF_PP, CAN_AF);
	//HAL_NVIC_EnableIRQ(CEC_CAN_IRQn);
}

static void CANx_Deinit(void)
{
	//HAL_NVIC_DisableIRQ(CEC_CAN_IRQn);
	__HAL_RCC_CAN1_CLK_DISABLE();
	GPIO_Deinit(CAN_GPIO, CAN_PINS);
}

/*
 * INTERRUPT ROUTINES
 */

#endif
