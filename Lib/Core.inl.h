
/*
 * PRIVATE DEFINITIONS
 */

extern volatile uint32_t gTicks;

/*
 * INLINE FUNCTION DEFINITIONS
 */

static inline uint32_t CORE_GetTick(void)
{
	return gTicks;
}
