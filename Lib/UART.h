#ifndef UART_H
#define UART_H

#include "STM32X.h"

/*
 * FUNCTIONAL TESTING
 * STM32L0: Y
 * STM32F0: Y
 * STM32G0: Y
 * STM32WL: Y
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
} UART_Buffer_t;

typedef struct {
	UART_Buffer_t tx;
	UART_Buffer_t rx;
	USART_TypeDef * Instance;
} UART_t;

typedef enum {
	UART_Mode_Default 	= 0,
	UART_Mode_Inverted 	= (1 << 0),
	UART_Mode_Swap		= (1 << 1),
	UART_Mode_EvenParity = (1 << 2),
	UART_Mode_OddParity = (1 << 3),
} UART_Mode_t;

/*
 * PUBLIC FUNCTIONS
 */

// Initialisation
void UART_Init(UART_t * uart, uint32_t baud, UART_Mode_t mode);
void UART_Deinit(UART_t * uart);

// Transmit
void UART_Write(UART_t * uart, const uint8_t * data, uint32_t count);
void UART_WriteStr(UART_t * uart, const char * str);
void UART_WriteFlush(UART_t * uart);
uint32_t UART_WriteCount(UART_t * uart);

// Recieve
uint32_t UART_ReadCount(UART_t * uart);
uint32_t UART_Read(UART_t * uart, uint8_t * data, uint32_t count);
uint8_t UART_Pop(UART_t * uart);
void UART_ReadFlush(UART_t * uart);
uint32_t UART_Seek(UART_t * uart, uint8_t delimiter);


void UART_IRQHandler(UART_t * uart);

/*
 * EXTERN DECLARATIONS
 */

#ifdef UARTLP_PINS
extern UART_t * const UART_LP;
#endif
#ifdef UART1_PINS
extern UART_t * const UART_1;
#endif
#ifdef UART2_PINS
extern UART_t * const UART_2;
#endif
#ifdef UART3_PINS
extern UART_t * const UART_3;
#endif
#ifdef UART4_PINS
extern UART_t * const UART_4;
#endif
#ifdef UART5_PINS
extern UART_t * const UART_5;
#endif
#ifdef UART6_PINS
extern UART_t * const UART_6;
#endif

#endif //UART_H
