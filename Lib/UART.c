
#include "UART.h"
#include "Core.h"
#include "GPIO.h"
#include "CLK.h"

/*
 * PRIVATE DEFINITIONS
 */

#define UART_BFR_WRAP(v)	((v) & (UART_BFR_SIZE - 1))

#if (UART_BFR_WRAP(UART_BFR_SIZE) != 0)
#error "UART_BFR_SIZE must be a power of two"
#endif


#if defined(STM32G0) || defined(STM32WL)
#define USART_CR1_RXNEIE				USART_CR1_RXNEIE_RXFNEIE
#define USART_CR1_TXEIE					USART_CR1_TXEIE_TXFNFIE
#define USART_ISR_RXNE					USART_ISR_RXNE_RXFNE
#define USART_ISR_TXE					USART_ISR_TXE_TXFNF
#endif

#if defined(STM32L0)
#define USART4_IRQn			USART4_5_IRQn
#define USART5_IRQn			USART4_5_IRQn
#define USART_4_5_IRQ_JOIN
#elif defined(STM32F0)
#define USART3_IRQn			USART3_4_IRQn
#define USART4_IRQn			USART3_4_IRQn
#define USART_3_4_IRQ_JOIN
#elif defined(STM32G0)
#ifdef LPUART2
#define USART2_IRQn			USART2_LPUART2_IRQn
#endif //LPUART2
#define USART3_IRQn			USART3_4_5_6_LPUART1_IRQn
#define USART4_IRQn			USART3_4_5_6_LPUART1_IRQn
#define USART5_IRQn			USART3_4_5_6_LPUART1_IRQn
#define USART6_IRQn			USART3_4_5_6_LPUART1_IRQn
#define USART_3_6_IRQ_JOIN
#endif

#define __UART_RX_ENABLE(uart) 	(uart->Instance->CR1 |= USART_CR1_RXNEIE)
#define __UART_RX_DISABLE(uart) (uart->Instance->CR1 &= ~USART_CR1_RXNEIE)
#define __UART_TX_ENABLE(uart) 	(uart->Instance->CR1 |= USART_CR1_TXEIE)
#define __UART_TX_DISABLE(uart) (uart->Instance->CR1 &= ~USART_CR1_TXEIE)
#define __UART_TX_BUSY(uart)	(!(uart->Instance->ISR & USART_ISR_TC))
#define __UART_CLEAR_FLAGS(uart, flags) (uart->Instance->ICR |= flags)

#ifndef UART_IRQ_PRIO
#define UART_IRQ_PRIO					1
#endif //UART_IRQ_PRIO

/*
 * PRIVATE TYPES
 */

/*
 * PRIVATE PROTOTYPES
 */

static void UARTx_Init(UART_t * uart);
static void UARTx_Deinit(UART_t * uart);

/*
 * PRIVATE VARIABLES
 */

#ifdef UARTLP_PINS
static UART_t gUART_LP = {
	.Instance = LPUART1
};
UART_t * const UART_LP = &gUART_LP;
#endif
#ifdef UART1_PINS
static UART_t gUART_1 = {
	.Instance = USART1
};
UART_t * const UART_1 = &gUART_1;
#endif
#ifdef UART2_PINS
static UART_t gUART_2 = {
	.Instance = USART2
};
UART_t * const UART_2 = &gUART_2;
#endif
#ifdef UART3_PINS
static UART_t gUART_3 = {
	.Instance = USART3
};
UART_t * const UART_3 = &gUART_3;
#endif
#ifdef UART4_PINS
static UART_t gUART_4 = {
	.Instance = USART4
};
UART_t * const UART_4 = &gUART_4;
#endif
#ifdef UART5_PINS
static UART_t gUART_5 = {
	.Instance = USART5
};
UART_t * const UART_5 = &gUART_5;
#endif
#ifdef UART6_PINS
static UART_t gUART_6 = {
	.Instance = USART6
};
UART_t * const UART_6 = &gUART_6;
#endif

/*
 * PUBLIC FUNCTIONS
 */


void UART_Init(UART_t * uart, uint32_t baud, UART_Mode_t mode)
{
	uart->tx.head = uart->tx.tail = 0;
	uart->rx.head = uart->rx.tail = 0;

	// Enable the uart specific GPIO and clocks.
	UARTx_Init(uart);

	__HAL_UART_DISABLE(uart);
	// Configure to standard settings: 8N1, no flow control.
	uint32_t cr1 = (uint32_t)UART_WORDLENGTH_8B | UART_PARITY_NONE | UART_MODE_TX_RX | UART_OVERSAMPLING_16;
	uint32_t cr2 = UART_STOPBITS_1;
	uint32_t cr3 = (uint32_t)UART_HWCONTROL_NONE | UART_ONE_BIT_SAMPLE_DISABLE;

	if (mode & UART_Mode_Inverted) 		{ cr2 |= USART_CR2_RXINV | USART_CR2_TXINV; }
	if (mode & UART_Mode_Swap) 			{ cr2 |= USART_CR2_SWAP; }
	// The M bit here changes the frame size to 9 bits.
	// The parity bit counts as part of the frame.
	if (mode & UART_Mode_EvenParity) 	{ cr1 |= USART_CR1_PCE | USART_CR1_M0; }
	if (mode & UART_Mode_OddParity) 	{ cr1 |= USART_CR1_PCE | USART_CR1_PS | USART_CR1_M0; }

	const uint32_t cr1msk = USART_CR1_M0 | USART_CR1_M1 | USART_CR1_PCE | USART_CR1_PS | USART_CR1_TE | USART_CR1_RE | USART_CR1_OVER8;
	MODIFY_REG(uart->Instance->CR1, cr1msk,	cr1);

	const uint32_t cr2msk = USART_CR2_STOP | USART_CR2_RXINV | USART_CR2_TXINV | USART_CR2_SWAP | USART_CR2_LINEN | USART_CR2_CLKEN;
	MODIFY_REG(uart->Instance->CR2, cr2msk, cr2);

	const uint32_t cr3msk = USART_CR3_RTSE | USART_CR3_CTSE | USART_CR3_ONEBIT | USART_CR3_SCEN | USART_CR3_HDSEL | USART_CR3_IREN;
	MODIFY_REG(uart->Instance->CR3, cr3msk, cr3);

	// Calculate baud rate.
	uint32_t pclk = CLK_GetPCLKFreq();

#ifdef UARTLP_PINS
	if (UART_INSTANCE_LOWPOWER(uart))
	{
#if defined(STM32G0) || defined(STM32WL)
		uart->Instance->BRR = UART_DIV_LPUART(pclk, baud, UART_PRESCALER_DIV1);
#else
		uart->Instance->BRR = UART_DIV_LPUART(pclk, baud);
#endif
	}
	else
#endif
	{
#if defined(STM32G0) || defined(STM32WL)
		uart->Instance->BRR = UART_DIV_SAMPLING16(pclk, baud, UART_PRESCALER_DIV1);
#else
		uart->Instance->BRR = UART_DIV_SAMPLING16(pclk, baud);
#endif
	}
	__HAL_UART_ENABLE(uart);

	// Enable RX IRQ.
	__UART_RX_ENABLE(uart);
}

void UART_Deinit(UART_t * uart)
{
	// Disable RX IRQ, and TX IRQ in case a tx is underway.
	__UART_RX_DISABLE(uart);
	__UART_TX_DISABLE(uart);

	__HAL_UART_DISABLE(uart);
	// Clear all control registers.
	uart->Instance->CR1 = 0x0U;
	uart->Instance->CR2 = 0x0U;
	uart->Instance->CR3 = 0x0U;

	// Disable uart specific GPIO and clocks.
	UARTx_Deinit(uart);
}

void UART_Write(UART_t * uart, const uint8_t * data, uint32_t count)
{
	while (count--)
	{
		// calculate the next head. We cant assign it yet, as the IRQ relies on it.
		uint32_t head = UART_BFR_WRAP(uart->tx.head + 1);

		// If the head has caught up with tail.. wait.
		while (head == uart->tx.tail) { CORE_Idle(); }

		uart->tx.buffer[uart->tx.head] = *data++;
		uart->tx.head = head;

		// Enable transmitter - it may turn itself off at any time.
		__UART_TX_ENABLE(uart);
	}
}

void UART_WriteStr(UART_t * uart, const char * str)
{
	UART_Write(uart, (const uint8_t *)str, strlen(str));
}

uint32_t UART_ReadCount(UART_t * uart)
{
	__UART_RX_DISABLE(uart);
	// We have to disable the IRQs, as the IRQ may bump the tail.
	uint32_t count = UART_BFR_WRAP(uart->rx.head - uart->rx.tail);
	__UART_RX_ENABLE(uart);
	return count;
}

uint32_t UART_Read(UART_t * uart, uint8_t * data, uint32_t count)
{
	uint32_t available = UART_ReadCount(uart);
	if (available < count)
	{
		count = available;
	}

	// As long as we read faster than the tail is nudged, we should be fine.
	uint32_t tail = uart->rx.tail;
	for (uint32_t i = 0; i < count; i++)
	{
		*data++ = uart->rx.buffer[tail];
		tail = UART_BFR_WRAP(tail + 1);
	}
	uart->rx.tail = tail;

	return count;
}

uint32_t UART_Seek(UART_t * uart, uint8_t delimiter)
{
	uint32_t count = UART_ReadCount(uart);
	uint32_t tail = uart->rx.tail;
	for (uint32_t i = 0; i < count; i++)
	{
		if (uart->rx.buffer[tail] == delimiter)
		{
			return i + 1;
		}
		tail = UART_BFR_WRAP(tail + 1);
	}
	return 0;
}

uint8_t UART_Pop(UART_t * uart)
{
	uint32_t tail = uart->rx.tail;
	uint8_t b = uart->rx.buffer[tail];
	uart->rx.tail = UART_BFR_WRAP(tail + 1);
	return b;
}

void UART_ReadFlush(UART_t * uart)
{
	__UART_RX_DISABLE(uart);
	uart->rx.tail = uart->rx.head;
	__UART_RX_ENABLE(uart);
}

void UART_WriteFlush(UART_t * uart)
{
	while (UART_WriteCount(uart))
	{
		CORE_Idle();
	}
}

uint32_t UART_WriteCount(UART_t * uart)
{
	__UART_TX_DISABLE(uart);
	uint32_t count = UART_BFR_WRAP(uart->tx.head - uart->tx.tail);
	// Include the outgoing character
	if (__UART_TX_BUSY(uart)) { count++; }
	// Restore the transmitter if we still have pending data
	if (count) { __UART_TX_ENABLE(uart); }
	return count;
}

/*
 * PRIVATE FUNCTIONS
 */

static void UARTx_Init(UART_t * uart)
{
#ifdef UARTLP_PINS
	if (uart == UART_LP)
	{
		__HAL_RCC_LPUART1_CLK_ENABLE();
		GPIO_EnableAlternate(UARTLP_PINS, 0, UARTLP_AF);
		HAL_NVIC_EnableIRQ(LPUART1_IRQn);
		HAL_NVIC_SetPriority(LPUART1_IRQn, UART_IRQ_PRIO, UART_IRQ_PRIO);
	}
#endif
#ifdef UART1_PINS
	if (uart == UART_1)
	{
		__HAL_RCC_USART1_CLK_ENABLE();
		GPIO_EnableAlternate(UART1_PINS, 0, UART1_AF);
		HAL_NVIC_EnableIRQ(USART1_IRQn);
		HAL_NVIC_SetPriority(USART1_IRQn, UART_IRQ_PRIO, UART_IRQ_PRIO);
	}
#endif
#ifdef UART2_PINS
	if (uart == UART_2)
	{
		__HAL_RCC_USART2_CLK_ENABLE();
		GPIO_EnableAlternate(UART2_PINS, 0, UART2_AF);
		HAL_NVIC_EnableIRQ(USART2_IRQn);
		HAL_NVIC_SetPriority(USART2_IRQn, UART_IRQ_PRIO, UART_IRQ_PRIO);
	}
#endif
#ifdef UART3_PINS
	if (uart == UART_3)
	{
		__HAL_RCC_USART3_CLK_ENABLE();
		GPIO_EnableAlternate(UART3_PINS, 0, UART3_AF);
		HAL_NVIC_EnableIRQ(USART3_IRQn);
		HAL_NVIC_SetPriority(USART3_IRQn, UART_IRQ_PRIO, UART_IRQ_PRIO);
	}
#endif
#ifdef UART4_PINS
	if (uart == UART_4)
	{
		__HAL_RCC_USART4_CLK_ENABLE();
		GPIO_EnableAlternate(UART4_PINS, 0, UART4_AF);
		HAL_NVIC_EnableIRQ(USART4_IRQn);
		HAL_NVIC_SetPriority(USART4_IRQn, UART_IRQ_PRIO, UART_IRQ_PRIO);
	}
#endif
#ifdef UART5_PINS
	if (uart == UART_5)
	{
		__HAL_RCC_USART5_CLK_ENABLE();
		GPIO_EnableAlternate(UART5_PINS, 0, UART5_AF);
		HAL_NVIC_EnableIRQ(USART5_IRQn);
		HAL_NVIC_SetPriority(USART5_IRQn, UART_IRQ_PRIO, UART_IRQ_PRIO);
	}
#endif
#ifdef UART6_PINS
	if (uart == UART_6)
	{
		__HAL_RCC_USART6_CLK_ENABLE();
		GPIO_EnableAlternate(UART6_PINS, 0, UART6_AF);
		HAL_NVIC_EnableIRQ(USART6_IRQn);
		HAL_NVIC_SetPriority(USART6_IRQn, UART_IRQ_PRIO, UART_IRQ_PRIO);
	}
#endif
}

static void UARTx_Deinit(UART_t * uart)
{
#ifdef UARTLP_PINS
	if (uart == UART_LP)
	{
		HAL_NVIC_DisableIRQ(LPUART1_IRQn);
		__HAL_RCC_LPUART1_CLK_DISABLE();
		GPIO_Deinit(UARTLP_PINS);
	}
#endif
#ifdef UART1_PINS
	if (uart == UART_1)
	{
		HAL_NVIC_DisableIRQ(USART1_IRQn);
		__HAL_RCC_USART1_CLK_DISABLE();
		GPIO_Deinit(UART1_PINS);
	}
#endif
#ifdef UART2_PINS
	if (uart == UART_2)
	{
		HAL_NVIC_DisableIRQ(USART2_IRQn);
		__HAL_RCC_USART2_CLK_DISABLE();
		GPIO_Deinit(UART2_PINS);
	}
#endif
#ifdef UART3_PINS
	if (uart == UART_3)
	{
		//HAL_NVIC_DisableIRQ(USART3_IRQn);
		__HAL_RCC_USART3_CLK_DISABLE();
		GPIO_Deinit(UART3_PINS);
	}
#endif
#ifdef UART4_PINS
	if (uart == UART_4)
	{
		// TODO: Handle IRQ contention between UART_4 & UART_5
		//HAL_NVIC_DisableIRQ(USART4_IRQn);
		__HAL_RCC_USART4_CLK_DISABLE();
		GPIO_Deinit(UART4_PINS);
	}
#endif
#ifdef UART5_PINS
	if (uart == UART_5)
	{
		//HAL_NVIC_DisableIRQ(USART5_IRQn);
		__HAL_RCC_USART5_CLK_DISABLE();
		GPIO_Deinit(UART5_PINS);
	}
#endif
#ifdef UART6_PINS
	if (uart == UART_6)
	{
		//HAL_NVIC_DisableIRQ(USART6_IRQn);
		__HAL_RCC_USART6_CLK_DISABLE();
		GPIO_Deinit(UART6_PINS);
	}
#endif
}

/*
 * INTERRUPT ROUTINES
 */


void UART_IRQHandler(UART_t *uart)
{
	uint32_t flags = uart->Instance->ISR;

	if (flags & USART_ISR_RXNE)
	{
		// New RX data. Put it in the RX buffer.
		uart->rx.buffer[uart->rx.head] = uart->Instance->RDR;
		uart->rx.head = UART_BFR_WRAP(uart->rx.head + 1);
		if (uart->rx.head == uart->rx.tail) {
			// The head just caught up with the tail. Uh oh. Increment the tail.
			// Note, this causes flaming huge issues.
			uart->rx.tail = UART_BFR_WRAP(uart->rx.tail + 1);
		}
	}

	if (flags & USART_ISR_TXE)
	{
		// No byte being transmitted..
		if (uart->tx.head != uart->tx.tail)
		{
			// Send a byte out.
			uart->Instance->TDR = uart->tx.buffer[uart->tx.tail];
			uart->tx.tail = UART_BFR_WRAP(uart->tx.tail + 1);
		}
		else
		{
			// Tail caught up with head: no bytes remain.
			// Disable the TX IRQ.
			__UART_TX_DISABLE(uart);
		}
	}

	if (flags & (USART_ISR_ORE | USART_ISR_PE | USART_ISR_NE | USART_ISR_FE))
	{
		__UART_CLEAR_FLAGS(uart, (UART_CLEAR_PEF | UART_CLEAR_FEF | UART_CLEAR_NEF | UART_CLEAR_OREF));
	}
}


#ifdef UARTLP_PINS
void LPUART1_IRQHandler(void)
{
	UART_IRQHandler(UART_LP);
}
#endif
#ifdef UART1_PINS
void USART1_IRQHandler(void)
{
	UART_IRQHandler(UART_1);
}
#endif
#ifdef UART2_PINS
void USART2_IRQHandler(void)
{
	UART_IRQHandler(UART_2);
}
#endif

#if !(defined(USART_3_4_IRQ_JOIN) || defined(USART_3_6_IRQ_JOIN))
#ifdef UART3_PINS
void USART3_IRQHandler(void)
{
	UART_IRQHandler(UART_3);
}
#endif
#endif // USART_3_4_IRQ_JOIN || USART_3_6_IRQ_JOIN


#ifdef USART_3_4_IRQ_JOIN
#if (defined(UART3_PINS) || defined(UART4_PINS))
void USART3_4_IRQHandler(void)
{
#ifdef UART3_PINS
	UART_IRQHandler(UART_3);
#endif
#ifdef UART4_PINS
	UART_IRQHandler(UART_4);
#endif
}
#endif
#endif //USART_3_4_IRQ_JOIN


#ifdef USART_4_5_IRQ_JOIN
#if defined(UART4_PINS) || defined(UART5_PINS)
void USART4_5_IRQHandler(void)
{
#ifdef UART4_PINS
	UART_IRQHandler(UART_4);
#endif
#ifdef UART5_PINS
	UART_IRQHandler(UART_5);
#endif
}
#endif
#endif //USART_4_5_IRQ_JOIN

#ifdef USART_3_6_IRQ_JOIN
#if defined(UART3_PINS) || defined(UART4_PINS) || defined(UART5_PINS) || defined(UART6_PINS)
void USART3_4_5_6_LPUART1_IRQHandler(void)
{
#ifdef UART3_PINS
	UART_IRQHandler(UART_3);
#endif
#ifdef UART4_PINS
	UART_IRQHandler(UART_4);
#endif
#ifdef UART5_PINS
	UART_IRQHandler(UART_5);
#endif
#ifdef UART6_PINS
	UART_IRQHandler(UART_6);
#endif
}
#endif
#endif //USART_3_6_IRQ_JOIN
