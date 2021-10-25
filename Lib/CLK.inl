
/*
 * PRIVATE DEFINITIONS
 */

/*
 * INLINE FUNCTION DEFINITIONS
 */

static inline uint32_t CLK_GetHCLKFreq(void)
{
	return CLK_SYSCLK_FREQ;
}

static inline uint32_t CLK_GetPCLKFreq(void)
{
	return CLK_SYSCLK_FREQ;
}

static inline uint32_t CLK_GetLSOFreq(void)
{
#ifdef CLK_USE_LSE
	return CLK_LSE_FREQ;
#else
	return LSI_VALUE;
#endif
}

