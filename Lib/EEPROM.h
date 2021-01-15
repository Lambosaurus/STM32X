#ifndef EEPROM_H
#define EEPROM_H

#include "Board.h"

/*
 * EXAMPLE BOARD DEFINITION
 */

/*
 * PUBLIC DEFINITIONS
 */

/*
 * PUBLIC TYPES
 */

/*
 * PUBLIC FUNCTIONS
 */

void EEPROM_Write(uint32_t offset, const void * data, uint16_t size);
void EEPROM_Read(uint32_t offset, void * data, uint16_t size);

/*
 * EXTERN DECLARATIONS
 */

#endif //EEPROM_H
