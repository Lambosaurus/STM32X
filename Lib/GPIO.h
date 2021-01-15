#ifndef GPIO_H
#define GPIO_H

#include "Board.h"

/*
 *  EXAMPLE BOARD DEFINITION
 */

/*
//#define USE_GPIO_IRQS
//#define USE_EXTI_0
*/


/*
 * PUBLIC DEFINITIONS
 */

/*
 * PUBLIC TYPES
 */

typedef struct {
	GPIO_TypeDef * gpio;
	uint32_t pin;
} GPIO_t;

typedef void(*VoidFunction_t)(void);

typedef enum {
	GPIO_IT_RISING 	= GPIO_MODE_IT_RISING,
	GPIO_IT_FALLING = GPIO_MODE_IT_FALLING,
	GPIO_IT_BOTH 	= GPIO_MODE_IT_RISING_FALLING,
} GPIO_IT_Dir_t;


/*
 * PUBLIC FUNCTIONS
 */

// Initialisation
void GPIO_EnableOutput(GPIO_TypeDef * gpio, uint32_t pin, GPIO_PinState state);
void GPIO_EnableInput(GPIO_TypeDef * gpio, uint32_t pin, uint32_t pullup);
void GPIO_Disable(GPIO_TypeDef * gpio, uint32_t pin);
#ifdef USE_GPIO_IRQS
void GPIO_EnableIRQ(GPIO_TypeDef * gpio, uint32_t pin, uint32_t pullup, GPIO_IT_Dir_t dir, VoidFunction_t callback);
#endif //USE_GPIO_IRQS

// Outputs
void GPIO_Write(GPIO_TypeDef * gpio, uint32_t pin, GPIO_PinState state);
static inline void GPIO_Set(GPIO_TypeDef * gpio, uint32_t pin);
static inline void GPIO_Reset(GPIO_TypeDef * gpio, uint32_t pin);

// Inputs
static inline bool GPIO_Read(GPIO_TypeDef * gpio, uint32_t pin);
static inline uint32_t GPIO_ReadPort(GPIO_TypeDef * gpio, uint32_t pins);


/*
 * EXTERN DECLARATIONS
 */

#include "GPIO.inl"

#endif //GPIO_H
