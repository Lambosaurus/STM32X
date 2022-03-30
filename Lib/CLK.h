#ifndef CLK_H
#define CLK_H

#include "STM32X.h"

/*
 * FUNCTIONAL TESTING
 * STM32L0: Y
 * STM32F0: N
 */

/*
 * PUBLIC DEFINITIONS
 */

#ifndef CLK_SYSCLK_FREQ
#define CLK_SYSCLK_FREQ			32000000
#endif

/*
 * PUBLIC TYPES
 */

/*
 * PUBLIC FUNCTIONS
 */

// HCLK & PCLK included
void CLK_InitSYSCLK(void);

void CLK_EnableLSO(void);
void CLK_DisableLSO(void);

void CLK_EnableADCCLK(void);
void CLK_DisableADCCLK(void);

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

#include "CLK.inl.h"

#endif //CLK_H
