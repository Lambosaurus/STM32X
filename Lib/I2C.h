#ifndef I2C_H
#define I2C_H

#include "STM32X.h"

/*
 * FUNCTIONAL TESTING
 * STM32L0: Y
 * STM32F0: N
 * STM32G0: Y
 * STM32WL: Y
 */

/*
 * PUBLIC DEFINITIONS
 */

/*
 * PUBLIC TYPES
 */

typedef enum {
	I2C_Mode_Standard 	=  100000,
	I2C_Mode_Fast 		=  400000,
#ifdef USE_I2C_FASTMODEPLUS
	I2C_Mode_FastPlus 	= 1000000,
#endif
} I2C_Mode_t;

typedef struct {
	I2C_TypeDef * Instance;
	I2C_Mode_t mode;
} I2C_t;

/*
 * PUBLIC FUNCTIONS
 */

void I2C_Init(I2C_t * i2c, I2C_Mode_t mode);
void I2C_Deinit(I2C_t * i2c);

bool I2C_Scan(I2C_t * i2c, uint8_t address);

bool I2C_Write(I2C_t * i2c, uint8_t address, const uint8_t * data, uint32_t count);
bool I2C_Read(I2C_t * i2c, uint8_t address, uint8_t * data, uint32_t count);
bool I2C_Transfer(I2C_t * i2c, uint8_t address, const uint8_t * txdata, uint32_t txcount, uint8_t * rxdata, uint32_t rxcount);

/*
 * EXTERN DECLARATIONS
 */

#ifdef I2C1_PINS
extern I2C_t * I2C_1;
#endif
#ifdef I2C2_PINS
extern I2C_t * I2C_2;
#endif
#ifdef I2C3_PINS
extern I2C_t * I2C_3;
#endif


#endif //I2C_H
