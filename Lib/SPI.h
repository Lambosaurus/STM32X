#ifndef SPI_H
#define SPI_H

#include "STM32X.h"


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
	SPI_MODE0 = SPI_POLARITY_LOW | SPI_PHASE_1EDGE,
	SPI_MODE1 = SPI_POLARITY_LOW | SPI_PHASE_2EDGE,
	SPI_MODE2 = SPI_POLARITY_HIGH | SPI_PHASE_1EDGE,
	SPI_MODE3 = SPI_POLARITY_HIGH | SPI_PHASE_2EDGE,
} SPIMode_t;


/*
 * PUBLIC FUNCTIONS
 */

// Initialisation
void SPI_Init(SPI_t * spi, uint32_t bitrate, SPIMode_t mode);
void SPI_Deinit(SPI_t * spi);

// Transaction functions
void SPI_Tx(SPI_t * spi, const uint8_t * data, uint32_t count);
void SPI_Rx(SPI_t * spi, uint8_t * data, uint32_t count);
void SPI_TxRx(SPI_t * spi, const uint8_t * txdata, uint8_t * rxdata, uint32_t count);
uint8_t SPI_Xfer(SPI_t * spi, uint8_t data);


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
