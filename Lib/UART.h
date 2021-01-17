#ifndef UART_H
#define UART_H

#include "STM32X.h"

/*
 * EXAMPLE BOARD DEFINITION
 */

/*
#define UART1_GPIO	GPIOA
#define UART1_PINS	(GPIO_PIN_9 | GPIO_PIN_10)
#define UART1_AF	GPIO_AF4_USART1
//#define UART_BFR_SIZE 128
*/


/*
 * PUBLIC DEFINITIONS
 */

#ifndef UART_BFR_SIZE
#define UART_BFR_SIZE 64
#endif

/*
 * PUBLIC TYPES
 */

typedef struct
{
	uint8_t buffer[UART_BFR_SIZE];
	uint32_t head;
	uint32_t tail;
} UARTBuffer_t;

typedef struct {
	UARTBuffer_t tx;
	UARTBuffer_t rx;
	USART_TypeDef * Instance;
} UART_t;


/*
 * PUBLIC FUNCTIONS
 */

// Initialisation
void UART_Init(UART_t * uart, uint32_t baud);
void UART_Deinit(UART_t * uart);

// Transmit
void UART_Tx(UART_t * uart, const uint8_t * data, uint32_t count);
void UART_TxStr(UART_t * uart, const char * str);
void UART_TxFlush(UART_t * uart);

// Recieve
uint32_t UART_RxCount(UART_t * uart);
uint32_t UART_Rx(UART_t * uart, uint8_t * data, uint32_t count);
uint8_t UART_RxPop(UART_t * uart);
void UART_RxFlush(UART_t * uart);


/*
 * EXTERN DECLARATIONS
 */

#ifdef UART1_GPIO
extern UART_t * UART_1;
#endif
#ifdef UART2_GPIO
extern UART_t * UART_2;
#endif


#endif //UART_H
