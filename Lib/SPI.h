#ifndef SPI_H
#define SPI_H

#include "stm32l0xx_hal.h"
#include "Board.h"

/*
 *  EXAMPLE BOARD DEFINITION
 */

/*
#define SPI1_GPIO		GPIOB
#define SPI1_PINS		(GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5)
#define SPI1_AF			GPIO_AF0_SPI1
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
void SPI_Tx(SPI_t * spi, const uint8_t * data, uint16_t count);
void SPI_Rx(SPI_t * spi, uint8_t * data, uint16_t count);
void SPI_TxRx(SPI_t * spi, const uint8_t * txdata, uint8_t * rxdata, uint16_t count);
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


#endif //SPI_H
