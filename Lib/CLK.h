#ifndef CLK_H
#define CLK_H

#include "STM32X.h"

/*
 * FUNCTIONAL TESTING
 * STM32L0: Y
 * STM32F0: Y
 * STM32G0: Y
 * STM32WL: Y
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

void CLK_EnableRNGCLK(void);
void CLK_DisableRNGCLK(void);

#ifdef USB_ENABLE
void CLK_EnableUSBCLK(void);
void CLK_DisableUSBCLK(void);
#endif

static inline uint32_t CLK_GetHCLKFreq(void);
static inline uint32_t CLK_GetPCLKFreq(void);
static inline uint32_t CLK_GetLSOFreq(void);


// Helper function for selecting a peripheral prescalar.
// Inputs: 	Source clock frequency
//			Minimum divider
//			Maximum divider
// 			Destination frequency - this shall not be exceeded
// Returns: The prescalar index. (index = 0 is the minimum prescalar. index = 1 is minimum x2)
//          Destination frequency is modified to be the actual frequency.
uint32_t CLK_SelectPrescalar(uint32_t src_freq, uint32_t div_min, uint32_t div_max, uint32_t * dst_freq);


/*
 * EXTERN DECLARATIONS
 */

#include "CLK.inl.h"

#endif //CLK_H
