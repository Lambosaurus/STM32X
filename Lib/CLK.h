#ifndef CLK_H
#define CLK_H

#include "STM32X.h"

/*
 * FUNCTIONAL TESTING
 * STM32L0: N
 * STM32F0: N
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

void CLK_Init(void);

void CLK_EnableLSO(void);
void CLK_DisableLSO(void);

#ifdef USB_ENABLE
void CLK_EnableUSBCLK(void);
void CLK_DisableUSBCLK(void);
#endif

static inline uint32_t CLK_GetHCLKFreq(void);
static inline uint32_t CLK_GetPCLKFreq(void);
static inline uint32_t CLK_GetLSOFreq(void);

/*
 * EXTERN DECLARATIONS
 */

#include "CLK.inl"

#endif //CLK_H
