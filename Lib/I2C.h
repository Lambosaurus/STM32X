#ifndef I2C_H
#define I2C_H

#include "STM32X.h"

/*
 * FUNCTIONAL TESTING
 * STM32L0: Y
 * STM32F0: N
 */

/*
 * PUBLIC DEFINITIONS
 */

/*
 * PUBLIC TYPES
 */

typedef struct {
	I2C_TypeDef * Instance;
	uint32_t bitrate;
} I2C_t;


/*
 * PUBLIC FUNCTIONS
 */


void I2C_Init(I2C_t * i2c, uint32_t frequency);
void I2C_Deinit(I2C_t * i2c);

bool I2C_Scan(I2C_t * i2c, uint8_t address);

bool I2C_Tx(I2C_t * i2c, uint8_t address, const uint8_t * data, uint32_t count);
bool I2C_Rx(I2C_t * i2c, uint8_t address, uint8_t * data, uint32_t count);
bool I2C_TxRx(I2C_t * i2c, uint8_t address, const uint8_t * txdata, uint8_t * rxdata, uint32_t count);

/*
 * EXTERN DECLARATIONS
 */

#ifdef I2C1_GPIO
extern I2C_t * I2C_1;
#endif
#ifdef I2C2_GPIO
extern I2C_t * I2C_2;
#endif
#ifdef I2C3_GPIO
extern I2C_t * I2C_3;
#endif


#endif //I2C_H
