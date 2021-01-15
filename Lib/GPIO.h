#ifndef GPIO_H
#define GPIO_H

#include "Board.h"

/*
 * PUBLIC DEFINITIONS
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

#define GPIO_SET(gpio, pin) 	gpio->BSRR = (uint32_t)pin
#define GPIO_RESET(gpio, pin) 	gpio->BRR = (uint32_t)pin
#define GPIO_READ(gpio, pin)	((gpio->IDR & pin) > 0)

void GPIO_EnableOutput(GPIO_TypeDef * gpio, uint32_t pin, GPIO_PinState state);
void GPIO_EnableInput(GPIO_TypeDef * gpio, uint32_t pin, uint32_t pullup);
void GPIO_Write(GPIO_TypeDef * gpio, uint32_t pin, GPIO_PinState state);
void GPIO_Disable(GPIO_TypeDef * gpio, uint32_t pin);

#ifdef USE_GPIO_IRQS
void GPIO_EnableIRQ(GPIO_TypeDef * gpio, uint32_t pin, uint32_t pullup, GPIO_IT_Dir_t dir, VoidFunction_t callback);
#endif //USE_GPIO_IRQS

/*
 * PUBLIC TYPES
 */


#endif //GPIO_H
