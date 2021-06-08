#ifndef SPI_H
#define SPI_H

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
	SPI_TypeDef * Instance;
	uint32_t bitrate;
} SPI_t;

typedef enum {
	SPI_Mode_0 = SPI_POLARITY_LOW | SPI_PHASE_1EDGE,
	SPI_Mode_1 = SPI_POLARITY_LOW | SPI_PHASE_2EDGE,
	SPI_Mode_2 = SPI_POLARITY_HIGH | SPI_PHASE_1EDGE,
	SPI_Mode_3 = SPI_POLARITY_HIGH | SPI_PHASE_2EDGE,
} SPI_Mode_t;


/*
 * PUBLIC FUNCTIONS
 */

// Initialisation
void SPI_Init(SPI_t * spi, uint32_t bitrate, SPI_Mode_t mode);
void SPI_Deinit(SPI_t * spi);

// Transaction functions
void SPI_Write(SPI_t * spi, const uint8_t * data, uint32_t count);
void SPI_Read(SPI_t * spi, uint8_t * data, uint32_t count);
void SPI_Transfer(SPI_t * spi, const uint8_t * txdata, uint8_t * rxdata, uint32_t count);
uint8_t SPI_TransferByte(SPI_t * spi, uint8_t data);


/*
 * EXTERN DECLARATIONS
 */

#ifdef SPI1_GPIO
extern SPI_t * SPI_1;
#endif
#ifdef SPI2_GPIO
extern SPI_t * SPI_2;
#endif
#ifdef SPI3_GPIO
extern SPI_t * SPI_3;
#endif


#endif //SPI_H
