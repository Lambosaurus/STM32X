#ifndef BOARD_H
#define BOARD_H

#define STM32L0
//#define STM32F0
//#define STM32G0

// Core config
//#define CORE_USE_TICK_IRQ

// CLK config
//#define CLK_USE_HSE
//#define CLK_USE_LSE
//#define CLK_LSE_BYPASS
//#define CLK_LSE_FREQ		32768
//#define CLK_SYSCLK_FREQ	32000000

// RTC config
//#define RTC_USE_IRQS

// US config
//#define US_TIM			TIM_22
//#define US_RES			1

// ADC config
//#define ADC_VREF	        3300

// GPIO config
//#define GPIO_USE_IRQS
//#define GPIO_IRQ0_ENABLE

// TIM config
//#define TIM_USE_IRQS
//#define TIM2_ENABLE

// UART config
//#define UART1_GPIO		GPIOA
//#define UART1_PINS		(GPIO_PIN_9 | GPIO_PIN_10)
//#define UART1_AF		    GPIO_AF4_USART1
//#define UART_BFR_SIZE     128

// SPI config
//#define SPI1_GPIO		    GPIOB
//#define SPI1_PINS		    (GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5)
//#define SPI1_AF			GPIO_AF0_SPI1

// I2C config
//#define I2C1_GPIO			GPIOB
//#define I2C1_PINS			(GPIO_PIN_6 | GPIO_PIN_7)
//#define I2C1_AF			GPIO_AF1_I2C1
//#define I2C_USE_FASTMODEPLUS
//#define I2C_USE_LONG_TRANSFER

// CAN config
//#define CAN_GPIO			GPIOB
//#define CAN_PINS			(GPIO_PIN_8 | GPIO_PIN_9)
//#define CAN_AF			GPIO_AF4_CAN
//#define CAN_DUAL_FIFO

// USB config
//#define USB_ENABLE
//#define USB_CLASS_CDC
//#define USB_CDC_BFR_SIZE	512

// TSC config
//#define TSC_ENABLE
//#define TSC_AF					GPIO_AF3_TSC

#endif /* BOARD_H */
