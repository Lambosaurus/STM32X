#ifndef GPIO_H
#define GPIO_H

#include "STM32X.h"

/*
 * FUNCTIONAL TESTING
 * STM32L0: Y
 * STM32F0: Y
 * STM32G0: Y
 */

/*
 * PUBLIC DEFINITIONS
 */

#define GPIOCFG_MODE_POS		0
#define GPIOCFG_PULL_POS		4
#define GPIOCFG_SPEED_POS		8
#define GPIOCFG_FLAG_POS		12

/*
 * PUBLIC TYPES
 */

typedef enum {
	GPIO_IT_None			= 0x00,
	GPIO_IT_Rising 			= 0x01,
	GPIO_IT_Falling 		= 0x02,
	GPIO_IT_Both 			= GPIO_IT_Rising | GPIO_IT_Falling,
} GPIO_IT_Dir_t;

typedef enum {
	GPIO_Pull_None 			= GPIO_NOPULL << GPIOCFG_PULL_POS,
	GPIO_Pull_Up   			= GPIO_PULLUP << GPIOCFG_PULL_POS,
	GPIO_Pull_Down 			= GPIO_PULLDOWN << GPIOCFG_PULL_POS,
	GPIO_Pull_MASK			= 0x03 << GPIOCFG_PULL_POS,
} GPIO_Pull_t;

typedef enum {
	GPIO_Mode_Input			= 0x00 << GPIOCFG_MODE_POS,
	GPIO_Mode_Output 		= 0x01 << GPIOCFG_MODE_POS,
	GPIO_Mode_Alternate 	= 0x02 << GPIOCFG_MODE_POS,
	GPIO_Mode_Analog    	= 0x03 << GPIOCFG_MODE_POS,
	GPIO_Mode_MASK			= 0x03 << GPIOCFG_MODE_POS,
} GPIO_Mode_t;

typedef enum {
	GPIO_Speed_Slow     	= 0x00 << GPIOCFG_SPEED_POS,
	GPIO_Speed_Medium		= 0x01 << GPIOCFG_SPEED_POS,
	GPIO_Speed_Fast			= 0x02 << GPIOCFG_SPEED_POS,
	GPIO_Speed_High			= 0x03 << GPIOCFG_SPEED_POS,
	GPIO_Speed_MASK			= 0x03 << GPIOCFG_SPEED_POS,
} GPIO_Speed_t;

typedef enum {
	GPIO_Flag_None			= 0,
	GPIO_Flag_OpenDrain  	= 0x01 << GPIOCFG_FLAG_POS,
} GPIO_Flag_t;

typedef bool GPIO_State_t;

typedef enum {
	GPIO_Port_A = (0 << 16),
	GPIO_Port_B = (1 << 16),
	GPIO_Port_C = (2 << 16),
	GPIO_Port_D = (3 << 16),

	GPIO_Pin_0  = (1 <<  0),
	GPIO_Pin_1  = (1 <<  1),
	GPIO_Pin_2  = (1 <<  2),
	GPIO_Pin_3  = (1 <<  3),
	GPIO_Pin_4  = (1 <<  4),
	GPIO_Pin_5  = (1 <<  5),
	GPIO_Pin_6  = (1 <<  6),
	GPIO_Pin_7  = (1 <<  7),
	GPIO_Pin_8  = (1 <<  8),
	GPIO_Pin_9  = (1 <<  9),
	GPIO_Pin_10 = (1 << 10),
	GPIO_Pin_11 = (1 << 11),
	GPIO_Pin_12 = (1 << 12),
	GPIO_Pin_13 = (1 << 13),
	GPIO_Pin_14 = (1 << 14),
	GPIO_Pin_15 = (1 << 15),
	GPIO_Pin_All = 0xFFFF,

	PA0  = GPIO_Port_A | GPIO_Pin_0,
	PA1  = GPIO_Port_A | GPIO_Pin_1,
	PA2  = GPIO_Port_A | GPIO_Pin_2,
	PA3  = GPIO_Port_A | GPIO_Pin_3,
	PA4  = GPIO_Port_A | GPIO_Pin_4,
	PA5  = GPIO_Port_A | GPIO_Pin_5,
	PA6  = GPIO_Port_A | GPIO_Pin_6,
	PA7  = GPIO_Port_A | GPIO_Pin_7,
	PA8  = GPIO_Port_A | GPIO_Pin_8,
	PA9  = GPIO_Port_A | GPIO_Pin_9,
	PA10 = GPIO_Port_A | GPIO_Pin_10,
	PA11 = GPIO_Port_A | GPIO_Pin_11,
	PA12 = GPIO_Port_A | GPIO_Pin_12,
	PA13 = GPIO_Port_A | GPIO_Pin_13,
	PA14 = GPIO_Port_A | GPIO_Pin_14,
	PA15 = GPIO_Port_A | GPIO_Pin_15,

	PB0  = GPIO_Port_B | GPIO_Pin_0,
	PB1  = GPIO_Port_B | GPIO_Pin_1,
	PB2  = GPIO_Port_B | GPIO_Pin_2,
	PB3  = GPIO_Port_B | GPIO_Pin_3,
	PB4  = GPIO_Port_B | GPIO_Pin_4,
	PB5  = GPIO_Port_B | GPIO_Pin_5,
	PB6  = GPIO_Port_B | GPIO_Pin_6,
	PB7  = GPIO_Port_B | GPIO_Pin_7,
	PB8  = GPIO_Port_B | GPIO_Pin_8,
	PB9  = GPIO_Port_B | GPIO_Pin_9,
	PB10 = GPIO_Port_B | GPIO_Pin_10,
	PB11 = GPIO_Port_B | GPIO_Pin_11,
	PB12 = GPIO_Port_B | GPIO_Pin_12,
	PB13 = GPIO_Port_B | GPIO_Pin_13,
	PB14 = GPIO_Port_B | GPIO_Pin_14,
	PB15 = GPIO_Port_B | GPIO_Pin_15,

	PC0  = GPIO_Port_C | GPIO_Pin_0,
	PC1  = GPIO_Port_C | GPIO_Pin_1,
	PC2  = GPIO_Port_C | GPIO_Pin_2,
	PC3  = GPIO_Port_C | GPIO_Pin_3,
	PC4  = GPIO_Port_C | GPIO_Pin_4,
	PC5  = GPIO_Port_C | GPIO_Pin_5,
	PC6  = GPIO_Port_C | GPIO_Pin_6,
	PC7  = GPIO_Port_C | GPIO_Pin_7,
	PC8  = GPIO_Port_C | GPIO_Pin_8,
	PC9  = GPIO_Port_C | GPIO_Pin_9,
	PC10 = GPIO_Port_C | GPIO_Pin_10,
	PC11 = GPIO_Port_C | GPIO_Pin_11,
	PC12 = GPIO_Port_C | GPIO_Pin_12,
	PC13 = GPIO_Port_C | GPIO_Pin_13,
	PC14 = GPIO_Port_C | GPIO_Pin_14,
	PC15 = GPIO_Port_C | GPIO_Pin_15,

	PD0  = GPIO_Port_D | GPIO_Pin_0,
	PD1  = GPIO_Port_D | GPIO_Pin_1,
	PD2  = GPIO_Port_D | GPIO_Pin_2,
	PD3  = GPIO_Port_D | GPIO_Pin_3,
	PD4  = GPIO_Port_D | GPIO_Pin_4,
	PD5  = GPIO_Port_D | GPIO_Pin_5,
	PD6  = GPIO_Port_D | GPIO_Pin_6,
	PD7  = GPIO_Port_D | GPIO_Pin_7,
	PD8  = GPIO_Port_D | GPIO_Pin_8,
	PD9  = GPIO_Port_D | GPIO_Pin_9,
	PD10 = GPIO_Port_D | GPIO_Pin_10,
	PD11 = GPIO_Port_D | GPIO_Pin_11,
	PD12 = GPIO_Port_D | GPIO_Pin_12,
	PD13 = GPIO_Port_D | GPIO_Pin_13,
	PD14 = GPIO_Port_D | GPIO_Pin_14,
	PD15 = GPIO_Port_D | GPIO_Pin_15,

} GPIO_Pin_t;


/*
 * PUBLIC FUNCTIONS
 */

// Base initialisation. This does not need to be called by the user.
void GPIO_Init(GPIO_Pin_t pins, GPIO_Flag_t flag);

// Initialisation
static inline void GPIO_EnableOutput(GPIO_Pin_t pins, GPIO_State_t state);
static inline void GPIO_EnableInput(GPIO_Pin_t pins, GPIO_Pull_t pull);
void GPIO_EnableAlternate(GPIO_Pin_t pins, GPIO_Flag_t flags, uint32_t af);
static inline void GPIO_Deinit(GPIO_Pin_t pins);

#ifdef GPIO_USE_IRQS
// Note: 	The IRQ will not be deinitialised on GPIO_Deinit.
// 			To safely reuse the IO, call GPIO_OnChange(gpio, pin, GPIO_IT_None, NULL);
void GPIO_OnChange(GPIO_Pin_t pin, GPIO_IT_Dir_t dir, VoidFunction_t callback);
void GPIO_IRQHandler(uint32_t n);
#endif //GPIO_USE_IRQS

// Outputs
void GPIO_Write(GPIO_Pin_t pins, GPIO_State_t state);
void GPIO_Set(GPIO_Pin_t pins);
void GPIO_Reset(GPIO_Pin_t pins);

// Inputs
GPIO_State_t GPIO_Read(GPIO_Pin_t pin);
GPIO_Pin_t GPIO_ReadPort(GPIO_Pin_t pins);


/*
 * EXTERN DECLARATIONS
 */

#include "GPIO.inl.h"

#endif //GPIO_H
