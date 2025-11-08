
#include "USB_PD.h"
#ifdef USB_PD

#include "CLK.h"
#include "IRQ.h"
#include "util/FIFO.h"

/*
 * PRIVATE DEFINITIONS
 */

#if defined(USB_PD_SOURCE)
#error "This has not yet been developed"
#elif defined(USB_PD_SINK)
#else
#error "Please define role as source or sink"
#endif

#if USB_PD == 1
#define UCPD		UCPD1
#define UCPD_IRQ_NO				IRQ_No_UCPD1
#elif USB_PD == 2
#define UCPD		UCPD2
#define UCPD_IRQ_NO				IRQ_No_UCPD2
#endif

#ifndef USB_PD_IRQ_PRIO
#define USB_PD_IRQ_PRIO			1 // Will usually be equal to USB prio, as the irqn is actually shared.
#endif //USB_PD_IRQ_PRIO

#define _USB_PD_ENABLE(ucpd)	(ucpd->CFG1 |= UCPD_CFG1_UCPDEN)
#define _USB_PD_DISABLE(ucpd)	(ucpd->CFG1 &= ~UCPD_CFG1_UCPDEN)

#define UCPD_STROBE()			(SYSCFG->CFGR1 |= SYSCFG_CFGR1_UCPD1_STROBE | SYSCFG_CFGR1_UCPD2_STROBE)
#define UCPD_KCLK_FREQ()		(16000000) // Fed from the HSI16

#define USB_PD_PAYLOAD_MAX		30
#define USB_PD_PAYLOAD_IDLE		0xFF
#define USB_PD_TX_FIFO_SIZE		64

#define USB_PD_SPEC_REVISION	2


#define USB_PD_SYNC1			0b11000
#define USB_PD_SYNC2			0b10001
#define USB_PD_SYNC2			0b10001


/*
 * PRIVATE TYPES
 */

typedef enum {
	USB_PD_KCode_Sync1	=	0b11000,
	USB_PD_KCode_Sync2	=	0b10001,
	USB_PD_KCode_Sync3	=	0b00110,
	USB_PD_KCode_Rst1	=	0b00111,
	USB_PD_KCode_Rst2	=	0b11001,
	USB_PD_KCode_EOP	=	0b01101,
} USB_PD_KCode_t;

typedef enum {
	USB_PD_SOP_0 			= 0,
	USB_PD_SOP_1 			= 1,
	USB_PD_SOP_2 			= 2,
	USB_PD_SOP_1_Debug 		= 3,
	USB_PD_SOP_2_Debug 		= 4,
	USB_PD_SOP_Cable_Reset 	= 5,
	USB_PD_SOP_Extn1		= 6,
	USB_PD_SOP_Extn2		= 7,
	USB_PD_SOP_None			= 0xFF,
} USB_PD_SOP_t;

typedef enum {
	USB_PD_SOPK_0			= (USB_PD_KCode_Sync1 << 0) | (USB_PD_KCode_Sync1 << 5) | (USB_PD_KCode_Sync1 << 10) | (USB_PD_KCode_Sync2 << 15),
	USB_PD_SOPK_1			= (USB_PD_KCode_Sync1 << 0) | (USB_PD_KCode_Sync1 << 5) | (USB_PD_KCode_Sync3 << 10) | (USB_PD_KCode_Sync3 << 15),
	USB_PD_SOPK_2			= (USB_PD_KCode_Sync1 << 0) | (USB_PD_KCode_Sync3 << 5) | (USB_PD_KCode_Sync1 << 10) | (USB_PD_KCode_Sync3 << 15),
	USB_PD_SOPK_1_Debug		= (USB_PD_KCode_Sync1 << 0) | (USB_PD_KCode_Rst2  << 5) | (USB_PD_KCode_Rst2  << 10) | (USB_PD_KCode_Sync3 << 15),
	USB_PD_SOPK_2_Debug		= (USB_PD_KCode_Sync1 << 0) | (USB_PD_KCode_Rst2  << 5) | (USB_PD_KCode_Sync3 << 10) | (USB_PD_KCode_Sync2 << 15),
	USB_PD_SOPK_Cable_Reset	= (USB_PD_KCode_Rst1  << 0) | (USB_PD_KCode_Sync1 << 5) | (USB_PD_KCode_Rst1  << 10) | (USB_PD_KCode_Sync1 << 15),
} USB_PD_SOPK_t;

typedef enum {
	USB_PD_Msg_GoodCrc 				= 0x01,
	USB_PD_Msg_GotoMin 				= 0x02,
	USB_PD_Msg_Accept 				= 0x03,
	USB_PD_Msg_Reject 				= 0x04,
	USB_PD_Msg_Ping 				= 0x05,
	USB_PD_Msg_PsReady 				= 0x06,
	USB_PD_Msg_GetSourceCap		 	= 0x07,
	USB_PD_Msg_GetSinkCap 			= 0x08,
	USB_PD_Msg_DrSwap 				= 0x09,
	USB_PD_Msg_PrSwap 				= 0x0A,
	USB_PD_Msg_VconnSwap 			= 0x0B,
	USB_PD_Msg_Wait 				= 0x0C,
	USB_PD_Msg_SoftReset 			= 0x0D,
	USB_PD_Msg_DataReset 			= 0x0E,
	USB_PD_Msg_DataResetComplete 	= 0x0F,
	USB_PD_Msg_NotSupported 		= 0x10,
	USB_PD_Msg_GetSourceCapExtended = 0x11,
	USB_PD_Msg_GetStatus 			= 0x12,
	USB_PD_Msg_FrSwap				= 0x13,
	USB_PD_Msg_GetPpsStatus 		= 0x14,
	USB_PD_Msg_GetCountryCodes 		= 0x15,
	USB_PD_Msg_GetSinkCapExtended 	= 0x16,
	USB_PD_Msg_GetSourceInfo 		= 0x17,
	USB_PD_Msg_GetRevision 			= 0x18,

	USB_PD_Msg_DataMessage			= 0x80,
	USB_PD_Msg_SourceCapabilties 	= 0x81,
	USB_PD_Msg_Request				= 0x82,
	USB_PD_Msg_Bist					= 0x83,
	USB_PD_Msg_SinkCapabilities		= 0x84,
	USB_PD_Msg_BatteryStatus		= 0x85,
	USB_PD_Msg_Alert				= 0x86,
	USB_PD_Msg_GetCountryInfo		= 0x87,
	USB_PD_Msg_EnterUsb				= 0x88,
	USB_PD_Msg_EprRequest			= 0x89,
	USB_PD_Msg_EprMode				= 0x8A,
	USB_PD_Msg_SourceInfo			= 0x8B,
	USB_PD_Msg_Revision				= 0x8C,
} USB_PD_Msg_t;

/*
 * PRIVATE PROTOTYPES
 */

static void USB_PDx_Init(void);
static void USB_PDx_Deinit(void);

static void USB_PD_SelectPhy(USB_PD_Flag_t flag);
static void USB_PD_Reset(void);

/*
 * PRIVATE VARIABLES
 */

static struct {
	uint32_t max_voltage;
	uint32_t voltage_requested;
	uint8_t msg_id;

	struct {
		uint8_t bfr[USB_PD_PAYLOAD_MAX];
		uint8_t size;
		uint8_t sop;
	} rx;

	struct {
		FIFO_DECLARE(fifo, USB_PD_TX_FIFO_SIZE);
		uint8_t size;
	} tx;

} gUSB_PD;

/*
 * PUBLIC
 */

// TODO:
// How TF to poll in low power mode?
// Notifications for USB PD detection
// Change PD negotiation state on USB PD detection
// PD negotiation
// Notifications for USB PD negotiation
// Chunk messages into uint16_ts to make alignment easier?

void USB_PD_Init(void)
{
	CLK_EnableUCPDCLK();
	USB_PDx_Init();

	uint32_t src_freq = UCPD_KCLK_FREQ();
	uint32_t ucpd_freq = 12000000; // Target is 6-12 MHz
	uint32_t ucpd_prescalar = CLK_SelectPrescalar(src_freq, 1, 16, &ucpd_freq);
	uint32_t hbit_div = ucpd_freq / 600000;

	uint32_t cfg1 =   (UCPD_CFG1_PSC_UCPDCLK_0 * ucpd_prescalar)
					| (UCPD_CFG1_TRANSWIN_0 * 9)
					| (UCPD_CFG1_IFRGAP_0 * 15)
					| (UCPD_CFG1_HBITCLKDIV_0 * (hbit_div - 1))
					| UCPD_CFG1_RXORDSETEN_0 // SOP
					| UCPD_CFG1_RXORDSETEN_3 // Hard reset
					;

	UCPD->CFG1 |= cfg1;
	UCPD->CFG2 = 0;
	UCPD->IMR = 0;

	_USB_PD_ENABLE(UCPD);

	uint32_t cr = UCPD_CR_CCENABLE_0 | UCPD_CR_CCENABLE_1;
#ifdef USB_PD_SINK
	cr |= UCPD_CR_ANAMODE;
#endif
	UCPD->CR = cr;

	UCPD_STROBE();
}

void USB_PD_Deinit(void)
{
	_USB_PD_DISABLE(UCPD);
	USB_PDx_Deinit();
	UCPD_STROBE();
	CLK_DisableUCPDCLK();
}

USB_PD_Flag_t USB_PD_Read(void)
{
	uint32_t sr = UCPD->SR;
	if (sr & UCPD_SR_TYPEC_VSTATE_CC1)
	{
		USB_PD_Flag_t current = (sr & UCPD_SR_TYPEC_VSTATE_CC1) >> UCPD_SR_TYPEC_VSTATE_CC1_Pos;
		return USB_PD_Flag_CC1 | (current * USB_PD_Flag_500mA);
	}
	if (sr & UCPD_SR_TYPEC_VSTATE_CC2)
	{
		USB_PD_Flag_t current = (sr & UCPD_SR_TYPEC_VSTATE_CC2) >> UCPD_SR_TYPEC_VSTATE_CC2_Pos;
		return USB_PD_Flag_CC2 | (current * USB_PD_Flag_500mA);
	}
	return 0;
}

void USB_PD_Start(uint32_t voltage_limit)
{
	gUSB_PD.max_voltage = voltage_limit;

	USB_PD_Flag_t flags = USB_PD_Read();
	USB_PD_SelectPhy(flags);

	UCPD->IMR |= UCPD_IMR_RXMSGENDIE | UCPD_IMR_RXHRSTDETIE | UCPD_IMR_RXNEIE | UCPD_IMR_RXORDDETIE | UCPD_IMR_TXISIE | UCPD_IMR_TXMSGSENTIE;

	USB_PD_Reset();

	IRQ_Enable(UCPD_IRQ_NO, USB_PD_IRQ_PRIO);
}

void USB_PD_Stop(void)
{
	UCPD->IMR &= ~(UCPD_IMR_RXMSGENDIE | UCPD_IMR_RXHRSTDETIE | UCPD_IMR_RXNEIE | UCPD_IMR_RXORDDETIE | UCPD_IMR_TXISIE | UCPD_IMR_TXMSGSENTIE);
	USB_PD_SelectPhy(0);
}

/*
 * PRIVATE FUNCTIONS
 */

static void USB_PD_Reset(void)
{
	gUSB_PD.msg_id = 0;
	gUSB_PD.voltage_requested = 0;
	FIFO_Clear(&gUSB_PD.tx.fifo);
	gUSB_PD.tx.size = 0;
	gUSB_PD.rx.sop = USB_PD_SOP_None;
}

static void USB_PD_SelectPhy(USB_PD_Flag_t flag)
{
	uint32_t cr = UCPD->CR;

	cr &= ~(UCPD_CR_PHYRXEN | UCPD_CR_PHYCCSEL);

	if (flag & USB_PD_Flag_CC1)
	{
		cr |= UCPD_CR_PHYRXEN;
	}
	else if (flag & USB_PD_Flag_CC2)
	{
		cr |= UCPD_CR_PHYRXEN | UCPD_CR_PHYCCSEL;
	}

	UCPD->CR = cr;

	//UCPD_STROBE();
}

static void USB_PDx_Init(void)
{
#if USB_PD == 1
	__HAL_RCC_UCPD1_CLK_ENABLE();
#elif USB_PD == 2
	__HAL_RCC_UCPD2_CLK_ENABLE();
#endif
}

static void USB_PDx_Deinit(void)
{
#if USB_PD == 1
	__HAL_RCC_UCPD1_CLK_DISABLE();
#elif USB_PD == 2
	__HAL_RCC_UCPD2_CLK_DISABLE();
#endif
}

static void USB_PD_Queue(USB_PD_Msg_t msg_type, uint8_t msg_id, const uint32_t * objects, uint8_t count)
{
	// [    15]: Extended
	// [14..12]: Number of data objects
	// [11.. 9]: MessageID
	// [     8]: Power Power Role (SOP only?)
	// [ 7.. 6]: Specification revision (SOP only?)
	// [     5]: Power Data Role (SOL only?)
	// [ 4.. 0]: Message Type

	uint16_t header =
			  count << 12
			| (msg_id << 9)
			| (USB_PD_SPEC_REVISION << 6)
			| (msg_type & 0x1F) ;
	uint8_t size = 2 + (count * 4);

	uint8_t bfr[USB_PD_PAYLOAD_MAX + 1];
	bfr[0] = size;
	bfr[1] = header;
	bfr[2] = header >> 8;
	memcpy(bfr + 3, objects, count * 4);

	FIFO_Enqueue(&gUSB_PD.tx.fifo, bfr, size+1);
}

static void USB_PD_QueueAck(uint8_t msg_id)
{
	USB_PD_Queue(USB_PD_Msg_GoodCrc, msg_id, NULL, 0);
}

static void USB_PD_TransmitNext(void)
{
	if (UCPD->TX_PAYSZ == 0)
	{
		if (gUSB_PD.tx.size)
		{
			// We could have had unwritten data, if the previous message was aborted.
			// Discard this now.
			FIFO_Discard(&gUSB_PD.tx.fifo, gUSB_PD.tx.size);
			gUSB_PD.tx.size = 0;
		}

		uint8_t size;
		if (FIFO_Pop(&gUSB_PD.tx.fifo, &size))
		{
			gUSB_PD.tx.size = size;
			UCPD->TX_ORDSET = USB_PD_SOPK_0;
			UCPD->TX_PAYSZ = size;
			UCPD->CR |= UCPD_CR_TXSEND;
		}
	}
}

static void USB_PD_HandleSourceCapabilties(const uint8_t * data, uint8_t objects)
{
	//if (gUSB_PD.voltage_requested != 0)
	//	return;

	uint32_t accepted_object = 0;
	uint32_t accepted_voltage = 0;

	for (uint8_t i = 0; i < objects; i++)
	{
		uint32_t object = (data[3] << 24) | (data[2] << 16) | (data[1] << 8) | data[0];
		data += 4;

		uint32_t min_voltage;
		uint32_t max_voltage;
		uint32_t current;

		uint32_t type = object >> 30;
		switch (type)
		{
		case 0: // Fixed supply
			current = ((object >> 0) & 0x3FF); // In 10mA units
			min_voltage = ((object >> 10) & 0x3FF) * 50;
			max_voltage = min_voltage;
			break;
		case 1: // Battery
		case 2: // Variable supply
			current = ((object >>  0) & 0x3FF); // In 10mA units
			min_voltage = ((object >> 10) & 0x3FF) * 50;
			max_voltage = ((object >> 20) & 0x3FF) * 50;
			break;
		case 3:
		default:
			continue;
		}

		if (max_voltage <= gUSB_PD.max_voltage && min_voltage > accepted_voltage)
		{
			accepted_voltage = min_voltage;

			// Valid for the basic types (not for augmented data objects).
			// [31..28]: Object Position (1 indexed)
			// [27..20]: Flags & reserved bits. Leave zero.
			// [19..10]: Operating current
			// [ 9.. 0]: Maximum operating current (deprecated)
			accepted_object = ((i+1) << 28) | (current << 10) | current;
		}
	}

	if (accepted_object != 0)
	{
		gUSB_PD.voltage_requested = accepted_voltage;
		// Not ready for this yet.....
		USB_PD_Queue(USB_PD_Msg_Request, (gUSB_PD.msg_id++) & 0x7, &accepted_object, 1);
	}
}

static void USB_PD_HandleMessage(const uint8_t * packet, uint8_t size)
{
	if (size < 2)
		return; // No header. What do?


	uint16_t header = packet[0] | (packet[1] << 8);

	bool extended = header & 0x8000;
	uint32_t data_objects = header >> 12 & 0x7;
	uint32_t msg_id = (header >> 9) & 0x7;
	uint32_t command = header & 0x1F;

	if (extended)
		return; // not supported.

	if ( size != 2 + (data_objects * 4) )
		return; // Bad packet size.

	USB_PD_QueueAck(msg_id);

	// We use a high bit to specify commands.
	switch (command | (data_objects ? USB_PD_Msg_DataMessage : 0))
	{
	case USB_PD_Msg_SourceCapabilties:
		USB_PD_HandleSourceCapabilties(packet + 2, data_objects);
		break;

	case USB_PD_Msg_PsReady:
		break;

	default:
		break;
	}
}

/*
 * INTERRUPT ROUTINES
 */

void USB_PD_IRQHandler(void)
{
	uint32_t sr = UCPD->SR;

	if (sr & UCPD_SR_RXHRSTDET)
	{
		// Reset condition detected
		USB_PD_Reset();
		UCPD->ICR = UCPD_ICR_RXHRSTDETCF;
	}

	if (sr & UCPD_SR_RXORDDET)
	{
		// Ordered set detect. This should be our start-of-packet (SOP)
		uint32_t ordset = UCPD->RX_ORDSET & UCPD_RX_ORDSET_RXORDSET;
		gUSB_PD.rx.sop = ordset;
		gUSB_PD.rx.size = 0;
		UCPD->ICR = UCPD_ICR_RXORDDETCF;
	}

	while (UCPD->SR & UCPD_SR_RXNE)
	{
		// Read data until fifo empty
		uint8_t data = UCPD->RXDR;

		if (gUSB_PD.rx.sop == USB_PD_SOP_0)
		{
			if (gUSB_PD.rx.size < sizeof(gUSB_PD.rx.bfr))
			{
				gUSB_PD.rx.bfr[gUSB_PD.rx.size++] = data;
			}
			else
			{
				// Overrun. Abort.
				gUSB_PD.rx.sop = USB_PD_SOP_None;
			}
		}
	}

	if (sr & UCPD_SR_RXMSGEND)
	{
		// End of message.

		bool valid = gUSB_PD.rx.sop == USB_PD_SOP_0
					&& !(UCPD->SR & UCPD_SR_RXERR)
					&& gUSB_PD.rx.size == UCPD->RX_PAYSZ
					;
		if (valid)
		{
			USB_PD_HandleMessage(gUSB_PD.rx.bfr, gUSB_PD.rx.size);
		}
		gUSB_PD.rx.sop = USB_PD_SOP_None;
		UCPD->ICR = UCPD_ICR_RXMSGENDCF;

		// We probably queued up a message (or two)
		// Start the TX.
		USB_PD_TransmitNext();
	}

	if (sr & UCPD_SR_TXMSGSENT)
	{
		UCPD->ICR = UCPD_ICR_TXMSGSENTCF;

		// We might have a second message. Start it.
		USB_PD_TransmitNext();
	}

	while (UCPD->SR & UCPD_SR_TXIS)
	{
		if (gUSB_PD.tx.size)
		{
			gUSB_PD.tx.size--;
			UCPD->TXDR = FIFO_BlindPop(&gUSB_PD.tx.fifo);
		}
		else
		{
			// big uh-oh.
			UCPD->TXDR = 0;
			__BKPT();
		}
	}
}

#endif //USB_PD
