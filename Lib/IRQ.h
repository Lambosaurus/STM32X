#ifndef IRQ_H
#define IRQ_H

#include "STM32X.h"

/*
 * FUNCTIONAL TESTING
 * STM32L0: N
 * STM32F0: N
 * STM32G0: N
 * STM32WL: N
 */

/*
 * PUBLIC DEFINITIONS
 */

/*
 * PUBLIC TYPES
 */

#if defined(STM32L0)
#error "Please enter IRQ numbers for L0 devices"
#elif defined(STM32F0)
#error "Please enter IRQ numbers for F0 devices"

#elif defined(STM32G0)

typedef enum {
	IRQ_No_Systick 		= -1,
	IRQ_No_WWDG			= 0,
	IRQ_No_RTC 			= 2,
	IRQ_No_FLASH		= 3,
	IRQ_No_EXTI0		= 5,
	IRQ_No_EXTI2		= 6,
	IRQ_No_EXTI4		= 7,
	IRQ_No_USB			= 8,
	IRQ_No_UCPD1		= 8,
	IRQ_No_UCPD2		= 8,
	IRQ_No_DMA1_CH1		= 9,
	IRQ_No_DMA1_CH2		= 10,
	IRQ_No_DMA1_CH4 	= 11,
	IRQ_No_DMA2_CH1 	= 11,
	IRQ_No_ADC1			= 12,
	IRQ_No_COMP			= 12,
	IRQ_No_TIM1_COM		= 13,
	IRQ_No_TIM1			= 14,
	IRQ_No_TIM2			= 15,
	IRQ_No_TIM3			= 16,
	IRQ_No_TIM4			= 16,
	IRQ_No_TIM6			= 17,
	IRQ_No_DAC			= 17,
	IRQ_No_LPTIM1		= 17,
	IRQ_No_TIM7			= 18,
	IRQ_No_LPTIM2		= 18,
	IRQ_No_TIM14		= 19,
	IRQ_No_TIM15		= 20,
	IRQ_No_TIM16		= 21,
	IRQ_No_FDCAN_IT0  	= 21,
	IRQ_No_TIM17		= 22,
	IRQ_No_FDCAN_IT1  	= 22,
	IRQ_No_I2C1			= 23,
	IRQ_No_I2C2			= 24,
	IRQ_No_I2C3			= 24,
	IRQ_No_SPI1			= 25,
	IRQ_No_SPI2			= 26,
	IRQ_No_SPI3			= 26,
	IRQ_No_UART1		= 27,
	IRQ_No_UART2		= 28,
	IRQ_No_LPUART2		= 28,
	IRQ_No_UART3		= 29,
	IRQ_No_UART4		= 29,
	IRQ_No_UART5		= 29,
	IRQ_No_UART6		= 29,
	IRQ_No_LPUART1		= 29,
	IRQ_No_CEC			= 30,
} IRQ_No_t;

#else
#error "IRQ support not added for this series"
#endif

/*
 * PUBLIC FUNCTIONS
 */

void IRQ_Enable(IRQ_No_t irq, uint32_t priority);
void IRQ_Disable(IRQ_No_t irq);

/*
 * EXTERN DECLARATIONS
 */

#endif //IRQ_H
