
/*
 * PRIVATE DEFINITIONS
 */

/*
 * INLINE FUNCTION DEFINITIONS
 */

static inline uint32_t TIM_GetCounter(TIM_t * tim)
{
	return tim->Instance->CNT;
}
