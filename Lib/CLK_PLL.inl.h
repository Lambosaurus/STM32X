#include "STM32X.h"

/*
 * This does the PLL solving for CLK.c
 * This should NOT be referenced in any other file.
 */

/*
 * PRIVATE DEFINITIONS
 */

// The multiplier can be overridden by the user.
#ifndef CLK_PLL_MUL
#if defined(STM32G0) || defined(STM32WL)
#define CLK_PLL_MUL				8
#else
#define CLK_PLL_MUL				4
#endif
#endif

#ifndef CLK_PLL_DIV
#define CLK_PLL_DIV				((CLK_PLL_SRC_FREQ * CLK_PLL_MUL) / CLK_SYSCLK_FREQ)
#endif


#if defined(STM32G0) || defined(STM32WL)
#define RCC_PLL_MULX_IS_VALID(x)	(x >= 8 && x <= 86)
#define RCC_PLL_MULX(x)				(x)
#endif


#ifdef RCC_PLL_MULX_IS_VALID
// Do we have generated definitions for PLL_MUL_X?
#if RCC_PLL_MULX_IS_VALID(CLK_PLL_MUL)
// Do we have a valid multiplier?
#define CLK_PLL_MUL_CFG			RCC_PLL_MULX(CLK_PLL_MUL)
#else
#error "Unavailable PLL multiplier"
#endif

#else // We do not have generated PLL_MUL - do a manual selection

// Select the PLL MUL/DIV registers (There must be a smarter way)
#if (CLK_PLL_MUL == 2)
#define CLK_PLL_MUL_CFG			RCC_PLL_MUL2
#elif (CLK_PLL_MUL == 3)
#define CLK_PLL_MUL_CFG			RCC_PLL_MUL3
#elif (CLK_PLL_MUL == 4)
#define CLK_PLL_MUL_CFG			RCC_PLL_MUL4
#elif (CLK_PLL_MUL == 5)
#define CLK_PLL_MUL_CFG			RCC_PLL_MUL5
#elif (CLK_PLL_MUL == 6)
#define CLK_PLL_MUL_CFG			RCC_PLL_MUL6
#elif (CLK_PLL_MUL == 7)
#define CLK_PLL_MUL_CFG			RCC_PLL_MUL7
#elif (CLK_PLL_MUL == 8)
#define CLK_PLL_MUL_CFG			RCC_PLL_MUL8
#elif (CLK_PLL_MUL == 9)
#define CLK_PLL_MUL_CFG			RCC_PLL_MUL9
#elif (CLK_PLL_MUL == 10)
#define CLK_PLL_MUL_CFG			RCC_PLL_MUL10
#elif (CLK_PLL_MUL == 11)
#define CLK_PLL_MUL_CFG			RCC_PLL_MUL11
#elif (CLK_PLL_MUL == 12)
#define CLK_PLL_MUL_CFG			RCC_PLL_MUL12
#elif (CLK_PLL_MUL == 13)
#define CLK_PLL_MUL_CFG			RCC_PLL_MUL13
#elif (CLK_PLL_MUL == 14)
#define CLK_PLL_MUL_CFG			RCC_PLL_MUL14
#elif (CLK_PLL_MUL == 15)
#define CLK_PLL_MUL_CFG			RCC_PLL_MUL15
#elif (CLK_PLL_MUL == 16)
#define CLK_PLL_MUL_CFG			RCC_PLL_MUL16
#elif (CLK_PLL_MUL == 32)
#define CLK_PLL_MUL_CFG			RCC_PLL_MUL32
#elif (CLK_PLL_MUL == 48)
#define CLK_PLL_MUL_CFG			RCC_PLL_MUL48
#else
#error "Unavailable PLL multiplier"
#endif

#endif



#ifdef RCC_PLL_DIVX_IS_VALID
// Do we have generated definitions for PLL_MUL_X?
#if RCC_PLL_DIVX_IS_VALID(CLK_PLL_DIV)
// Do we have a valid multiplier?
#define CLK_PLL_DIV_CFG			RCC_PLL_DIVX(CLK_PLL_DIV)
#else
#error "Unavailable PLL divider"
#endif

#else // We do not have generated PLL_DIV - do a manual selection

#if (CLK_PLL_DIV == 1)
#define CLK_PLL_DIV_CFG			RCC_PLL_DIV1
#elif (CLK_PLL_DIV == 2)
#define CLK_PLL_DIV_CFG			RCC_PLL_DIV2
#elif (CLK_PLL_DIV == 3)
#define CLK_PLL_DIV_CFG			RCC_PLL_DIV3
#elif (CLK_PLL_DIV == 4)
#define CLK_PLL_DIV_CFG			RCC_PLL_DIV4
#elif (CLK_PLL_DIV == 5)
#define CLK_PLL_DIV_CFG			RCC_PLL_DIV5
#elif (CLK_PLL_DIV == 6)
#define CLK_PLL_DIV_CFG			RCC_PLL_DIV6
#elif (CLK_PLL_DIV == 7)
#define CLK_PLL_DIV_CFG			RCC_PLL_DIV7
#elif (CLK_PLL_DIV == 8)
#define CLK_PLL_DIV_CFG			RCC_PLL_DIV8
#else
#error "Unavailable PLL divider"
#endif
#endif

#if (((CLK_PLL_SRC_FREQ * CLK_PLL_MUL) / CLK_PLL_DIV) != CLK_SYSCLK_FREQ)
#error "CLK_SYSCLK_FREQ not achieved"
#endif

/*
 * INLINE FUNCTION DEFINITIONS
 */

