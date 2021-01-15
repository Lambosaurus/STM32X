
/*
 * PRIVATE DEFINITIONS
 */

/*
 * INLINE FUNCTION DEFINITIONS
 */

static inline void GPIO_Set(GPIO_TypeDef * gpio, uint32_t pin)
{
	gpio->BSRR = (uint32_t)pin;
}

static inline void GPIO_Reset(GPIO_TypeDef * gpio, uint32_t pin)
{
	gpio->BRR = (uint32_t)pin;
}

static inline bool GPIO_Read(GPIO_TypeDef * gpio, uint32_t pin)
{
	return ((gpio->IDR & pin) > 0);
}

static inline uint32_t GPIO_ReadPort(GPIO_TypeDef * gpio, uint32_t pins)
{
	return gpio->IDR & pins;
}
