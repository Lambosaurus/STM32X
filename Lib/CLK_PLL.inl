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
#define CLK_PLL_MUL				4
#endif

#ifndef CLK_PLL_DIV
#define CLK_PLL_DIV				((CLK_PLL_SRC_FREQ * CLK_PLL_MUL) / CLK_SYSCLK_FREQ)
#endif

// Select the PLL MUL/DIV registers (There must be a smarter way)
#if (CLK_PLL_MUL == 2)
#define CLK_PLL_MUL_CFG			RCC_PLL_MUL2
#elif (CLK_PLL_MUL == 3)
#define CLK_PLL_MUL_CFG			RCC_PLL_MUL3
#elif (CLK_PLL_MUL == 4)
#define CLK_PLL_MUL_CFG			RCC_PLL_MUL4
#elif (CLK_PLL_MUL == 6)
#define CLK_PLL_MUL_CFG			RCC_PLL_MUL6
#elif (CLK_PLL_MUL == 8)
#define CLK_PLL_MUL_CFG			RCC_PLL_MUL8
#elif (CLK_PLL_MUL == 12)
#define CLK_PLL_MUL_CFG			RCC_PLL_MUL12
#elif (CLK_PLL_MUL == 16)
#define CLK_PLL_MUL_CFG			RCC_PLL_MUL16
#elif (CLK_PLL_MUL == 32)
#define CLK_PLL_MUL_CFG			RCC_PLL_MUL32
#elif (CLK_PLL_MUL == 48)
#define CLK_PLL_MUL_CFG			RCC_PLL_MUL48
#else
#error "Unavailable PLL multiplier"
#endif

#if (CLK_PLL_DIV == 1)
#define CLK_PLL_DIV_CFG			RCC_PLL_DIV1
#elif (CLK_PLL_DIV == 2)
#define CLK_PLL_DIV_CFG			RCC_PLL_DIV2
#elif (CLK_PLL_DIV == 3)
#define CLK_PLL_DIV_CFG			RCC_PLL_DIV3
#elif (CLK_PLL_DIV == 4)
#define CLK_PLL_DIV_CFG			RCC_PLL_DIV4
#else
#error "Unavailable PLL divider"
#endif

#if (((CLK_PLL_SRC_FREQ * CLK_PLL_MUL) / CLK_PLL_DIV) != CLK_SYSCLK_FREQ)
#error "CLK_SYSCLK_FREQ not achieved"
#endif

/*
 * INLINE FUNCTION DEFINITIONS
 */

