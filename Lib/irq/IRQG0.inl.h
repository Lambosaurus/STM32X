
/*
 * PRIVATE INCLUDES
 */

#include "COMP.h"
#include "DMA.h"
#include "GPIO.h"
#include "LPTIM.h"
#include "RTC.h"
#include "TIM.h"
#include "UART.h"
#include "USB.h"

/*
 * PRIVATE DEFINITIONS
 */

// Here is where we do horrible bodges to get the IRQ names correct....
// Let the IRQ handlers use the most inclusive names, and these defines can fix it.

#ifndef LPUART2
#define USART2_LPUART2_IRQHandler		USART2_IRQHandler
#endif //LPUART2

#ifndef TIM4
#define TIM3_TIM4_IRQHandler			TIM3_IRQHandler
#endif

#ifndef FDCAN1
#define TIM16_FDCAN_IT0_IRQHandler		TIM16_IRQHandler
#define TIM17_FDCAN_IT0_IRQHandler		TIM17_IRQHandler
#endif

#ifndef I2C3
#define I2C2_3_IRQHandler				I2C2_IRQHandler
#endif

#ifndef SPI3
#define SPI2_3_IRQHandler				SPI2_IRQHandler
#endif

#ifndef DMA2
#ifndef DMA1_Channel6
#define DMA1_Ch4_7_DMA2_Ch1_5_DMAMUX1_OVR_IRQHandler		DMA1_Ch4_5_DMAMUX1_OVR_IRQHandler
#else
#define DMA1_Ch4_7_DMA2_Ch1_5_DMAMUX1_OVR_IRQHandler		DMA1_Ch4_7_DMAMUX1_OVR_IRQHandler
#endif // DMA1_Channel6
#endif //DMA2

#ifndef USART5
#ifdef LPUART1
#define USART3_4_5_6_LPUART1_IRQHandler	USART3_4_LPUART1_IRQHandler
#else
#define USART3_4_5_6_LPUART1_IRQHandler	USART3_4_IRQHandler
#endif
#endif

/*
 * INTERRUPT HANDLERS
 */

//void SysTick_Handler(void); // Allow this to be defined in Core.h
//void WWDG_IRQHandler(void);
//void PVD_VDDIO2_IRQHandler(void);

#if defined(RTC_USE_IRQS)
void RTC_TAMP_IRQHandler(void)
{
	RTC_IRQHandler();
}
#endif //defined(RTC_USE_IRQS)

//void FLASH_IRQHandler(void);
//void RCC_CRS_IRQHandler(void);

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
void USB_UCPD1_2_IRQHandler(void)
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

#if defined(DMA_CH4_ENABLE) || defined(DMA_CH5_ENABLE) || defined(DMA_CH6_ENABLE) || defined(DMA_CH7_ENABLE)
void DMA1_Ch4_7_DMA2_Ch1_5_DMAMUX1_OVR_IRQHandler(void)
{
#ifdef DMA_CH4_ENABLE
	DMA_IRQHandler(DMA_CH4);
#endif
#ifdef DMA_CH5_ENABLE
	DMA_IRQHandler(DMA_CH5);
#endif
#ifdef DMA_CH6_ENABLE
	DMA_IRQHandler(DMA_CH6);
#endif
#ifdef DMA_CH7_ENABLE
	DMA_IRQHandler(DMA_CH7);
#endif
}
#endif // DMA_CH4_ENABLE .. DMA_CH7_ENABLE

#ifdef COMP_USE_IRQS
void ADC1_COMP_IRQHandler(void)
{
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

#if defined(TIM_USE_IRQS) && defined(TIM2_ENABLE)
void TIM2_IRQHandler(void)
{
	TIM_IRQHandler(TIM_2);
}
#endif //defined(TIM_USE_IRQS) && defined(TIM2_ENABLE)

#if defined(TIM_USE_IRQS) && (defined(TIM3_ENABLE) || defined(TIM4_ENABLE))
void TIM3_TIM4_IRQHandler(void)
{
#ifdef TIM3_ENABLE
	TIM_IRQHandler(TIM_3);
#endif
#ifdef TIM4_ENABLE
	TIM_IRQHandler(TIM_4);
#endif
}
#endif //defined(TIM_USE_IRQS) && (defined(TIM3_ENABLE) || defined(TIM4_ENABLE))

#if (defined(TIM_USE_IRQS) && defined(TIM6_ENABLE)) || (defined(LPTIM_USE_IRQS) && defined(LPTIM1_ENABLE))
void TIM6_DAC_LPTIM1_IRQHandler(void)
{
#if defined(TIM_USE_IRQS) && defined(TIM6_ENABLE)
	TIM_IRQHandler(TIM_6);
#endif
#if defined(LPTIM_USE_IRQS) && defined(LPTIM1_ENABLE)
	LPTIM_IRQHandler(LPTIM_1);
#endif
}
#endif //(defined(TIM_USE_IRQS) && defined(TIM6_ENABLE)) || (defined(LPTIM_USE_IRQS) && defined(LPTIM1_ENABLE))

#if (defined(TIM_USE_IRQS) && defined(TIM7_ENABLE)) || (defined(LPTIM_USE_IRQS) && defined(LPTIM2_ENABLE))
void TIM7_LPTIM2_IRQHandler(void)
{
#if defined(TIM_USE_IRQS) && defined(TIM7_ENABLE)
	TIM_IRQHandler(TIM_7);
#endif
#if defined(LPTIM_USE_IRQS) && defined(LPTIM2_ENABLE)
	LPTIM_IRQHandler(LPTIM_2);
#endif
}
#endif //(defined(TIM_USE_IRQS) && defined(TIM7_ENABLE)) || (defined(LPTIM_USE_IRQS) && defined(LPTIM2_ENABLE))

#if defined(TIM_USE_IRQS) && defined(TIM14_ENABLE)
void TIM14_IRQHandler(void)
{
	TIM_IRQHandler(TIM_14);
}
#endif //defined(TIM_USE_IRQS) && defined(TIM14_ENABLE)

#if defined(TIM_USE_IRQS) && defined(TIM15_ENABLE)
void TIM15_IRQHandler(void)
{
	TIM_IRQHandler(TIM_15);
}
#endif //defined(TIM_USE_IRQS) && defined(TIM15_ENABLE)

#if defined(TIM_USE_IRQS) && defined(TIM16_ENABLE)
void TIM16_FDCAN_IT0_IRQHandler(void)
{
	TIM_IRQHandler(TIM_16);
}
#endif //defined(TIM_USE_IRQS) && defined(TIM16_ENABLE)

#if defined(TIM_USE_IRQS) && defined(TIM17_ENABLE)
void TIM17_FDCAN_IT1_IRQHandler(void)
{
	TIM_IRQHandler(TIM_17);
}
#endif //defined(TIM_USE_IRQS) && defined(TIM17_ENABLE)

//void I2C1_IRQHandler(void);
//void I2C2_3_IRQHandler(void);
//void SPI1_IRQHandler(void);
//void SPI2_3_IRQHandler(void);

#if defined(UART1_PINS)
void USART1_IRQHandler(void)
{
	UART_IRQHandler(UART_1);
}
#endif // defined(UART1_PINS)

#if defined(UART2_PINS)
void USART2_LPUART2_IRQHandler(void)
{
	UART_IRQHandler(UART_2);
}
#endif //defined(UART2_PINS)

#if defined(UART3_PINS) || defined(UART4_PINS) || defined(UART5_PINS) || defined(UART6_PINS) || defined(LPUART_PINS)
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
#ifdef LPUART_PINS
	UART_IRQHandler(UART_LP);
#endif
}
#endif //defined(UART3_PINS) || defined(UART4_PINS) || defined(UART5_PINS) || defined(UART6_PINS) || defined(LPUART_PINS)


//void CEC_IRQHandler(void);



