/*
 * PRIVATE INCLUDES
 */

#include "COMP.h"
#include "DMA.h"
#include "GPIO.h"
#include "RTC.h"
#include "TIM.h"
#include "UART.h"
#include "USB.h"

/*
 * PRIVATE DEFINITIONS
 */

#ifndef COMP1
#define ADC1_COMP_IRQHandler            ADC1_IRQHandler
#endif //COMP1

#if !defined(USART3) && !defined(USART4) && !defined(LPUART1)
#define USART3_4_LPUART1_IRQHandler     USART3_IRQHandler   // never instantiated
#endif

#ifndef USB_DRD_FS
#define USB_DRD_FS_IRQHandler           USB_IRQHandler
#endif

/*
 * INTERRUPT HANDLERS
 */

//void SysTick_Handler(void); // Allow this to be defined in Core.h
//void WWDG_IRQHandler(void);
//void RTC_IRQHandler(void); // Allow this to be defined in RTC.h
//void FLASH_IRQHandler(void);
//void RCC_IRQHandler(void);

#if defined(GPIO_IRQ0_ENABLE) || defined(GPIO_IRQ1_ENABLE)
void EXTI0_1_IRQHandler(void)
{
#ifdef GPIO_IRQ0_ENABLE
	EXTI_IRQHandler(0);
#endif
#ifdef GPIO_IRQ1_ENABLE
	EXTI_IRQHandler(1);
#endif
}
#endif //defined(GPIO_IRQ0_ENABLE) || defined(GPIO_IRQ1_ENABLE)

#if defined(GPIO_IRQ2_ENABLE) || defined(GPIO_IRQ3_ENABLE)
void EXTI2_3_IRQHandler(void)
{
#ifdef GPIO_IRQ2_ENABLE
	EXTI_IRQHandler(2);
#endif
#ifdef GPIO_IRQ3_ENABLE
	EXTI_IRQHandler(3);
#endif
}
#endif //defined(GPIO_IRQ2_ENABLE) || defined(GPIO_IRQ3_ENABLE)

#if    defined(GPIO_IRQ4_ENABLE) || defined(GPIO_IRQ5_ENABLE) || defined(GPIO_IRQ6_ENABLE)   \
    || defined(GPIO_IRQ7_ENABLE) || defined(GPIO_IRQ8_ENABLE) || defined(GPIO_IRQ9_ENABLE)   \
	|| defined(GPIO_IRQ10_ENABLE) || defined(GPIO_IRQ11_ENABLE) || defined(GPIO_IRQ12_ENABLE)\
	|| defined(GPIO_IRQ13_ENABLE) || defined(GPIO_IRQ14_ENABLE) || defined(GPIO_IRQ15_ENABLE)
void EXTI4_15_IRQHandler(void)
{
#ifdef GPIO_IRQ4_ENABLE
	EXTI_IRQHandler(4);
#endif
#ifdef GPIO_IRQ5_ENABLE
	EXTI_IRQHandler(5);
#endif
#ifdef GPIO_IRQ6_ENABLE
	EXTI_IRQHandler(6);
#endif
#ifdef GPIO_IRQ7_ENABLE
	EXTI_IRQHandler(7);
#endif
#ifdef GPIO_IRQ8_ENABLE
	EXTI_IRQHandler(8);
#endif
#ifdef GPIO_IRQ9_ENABLE
	EXTI_IRQHandler(9);
#endif
#ifdef GPIO_IRQ10_ENABLE
	EXTI_IRQHandler(10);
#endif
#ifdef GPIO_IRQ11_ENABLE
	EXTI_IRQHandler(11);
#endif
#ifdef GPIO_IRQ12_ENABLE
	EXTI_IRQHandler(12);
#endif
#ifdef GPIO_IRQ13_ENABLE
	EXTI_IRQHandler(13);
#endif
#ifdef GPIO_IRQ14_ENABLE
	EXTI_IRQHandler(14);
#endif
#ifdef GPIO_IRQ15_ENABLE
	EXTI_IRQHandler(15);
#endif
}
#endif // GPIO_IRQ4_ENABLE .. GPIO_IRQ15_ENABLE

#if defined(USB_ENABLE)
void USB_DRD_FS_IRQHandler(void)
{
	USB_IRQHandler();
}
#endif //defined(USB_ENABLE)

#if defined(DMA_CH1_ENABLE)
void DMA1_Channel1_IRQHandler(void)
{
	DMA_IRQHandler(DMA_CH1);
}
#endif //defined(DMA_CH1_ENABLE)

#if defined(DMA_CH2_ENABLE) || defined(DMA_CH3_ENABLE)
void DMA1_Channel2_3_IRQHandler(void)
{
#ifdef DMA_CH2_ENABLE
	DMA_IRQHandler(DMA_CH2);
#endif
#ifdef DMA_CH3_ENABLE
	DMA_IRQHandler(DMA_CH3);
#endif
}
#endif //defined(DMA_CH2_ENABLE) || defined(DMA_CH3_ENABLE)

// Note: On STM32C0 the DMA controller only has 3 channels (1, 2, 3) exposed on
// dedicated vectors. There is no DMA1_Channel4..7 like on F0/G0/L0, so the
// "channel 4-7" block that exists in the F0/G0/L0 files is deliberately omitted.
// If a user #defines DMA_CH4..CH7_ENABLE, no handler will be emitted here -
// which matches the hardware (no such channels exist).

//void DMAMUX1_IRQHandler(void); // Allow this to be defined in DMA.h if needed

#ifdef COMP_USE_IRQS
void ADC1_COMP_IRQHandler(void)
{
	// On C0 this macro aliases to ADC1_IRQHandler (no COMP peripheral exists).
	COMP_IRQHandler();
}
#endif //COMP_USE_IRQS

#if defined(TIM_USE_IRQS) && defined(TIM1_ENABLE)
void TIM1_BRK_UP_TRG_COM_IRQHandler(void)
{
	TIM_IRQHandler(TIM_1);
}

void TIM1_CC_IRQHandler(void)
{
	TIM_IRQHandler(TIM_1);
}
#endif //defined(TIM_USE_IRQS) && defined(TIM1_ENABLE)

#if defined(TIM_USE_IRQS) && defined(TIM3_ENABLE)
void TIM3_IRQHandler(void)
{
	TIM_IRQHandler(TIM_3);
}
#endif //defined(TIM_USE_IRQS) && defined(TIM3_ENABLE)

#if defined(TIM_USE_IRQS) && defined(TIM14_ENABLE)
void TIM14_IRQHandler(void)
{
	TIM_IRQHandler(TIM_14);
}
#endif //defined(TIM_USE_IRQS) && defined(TIM14_ENABLE)

#if defined(TIM_USE_IRQS) && defined(TIM16_ENABLE)
void TIM16_IRQHandler(void)
{
	TIM_IRQHandler(TIM_16);
}
#endif //defined(TIM_USE_IRQS) && defined(TIM16_ENABLE)

#if defined(TIM_USE_IRQS) && defined(TIM17_ENABLE)
void TIM17_IRQHandler(void)
{
	TIM_IRQHandler(TIM_17);
}
#endif //defined(TIM_USE_IRQS) && defined(TIM17_ENABLE)

// Note: The STM32C0 has no TIM2, TIM6, TIM7, TIM15 (baseline C011/C031/C051).
// Some parts in the family do add a TIM2 - add that handler here if support is
// needed later.

//void I2C1_IRQHandler(void);
//void SPI1_IRQHandler(void);

#if defined(UART1_PINS)
void USART1_IRQHandler(void)
{
	UART_IRQHandler(UART_1);
}
#endif // defined(UART1_PINS)

#if defined(UART2_PINS)
void USART2_IRQHandler(void)
{
	UART_IRQHandler(UART_2);
}
#endif //defined(UART2_PINS)

#if defined(UART3_PINS) || defined(UART4_PINS) || defined(LPUART_PINS)
void USART3_4_LPUART1_IRQHandler(void)
{
#ifdef UART3_PINS
	UART_IRQHandler(UART_3);
#endif
#ifdef UART4_PINS
	UART_IRQHandler(UART_4);
#endif
#ifdef LPUART_PINS
	UART_IRQHandler(UART_LP);
#endif
}
#endif //defined(UART3_PINS) || defined(UART4_PINS) || defined(LPUART_PINS)
