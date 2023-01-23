
/*
 * PRIVATE DEFINITIONS
 */

/*
 * INLINE FUNCTION DEFINITIONS
 */

// Init calls
static inline void GPIO_EnableInput(GPIO_t * gpio, uint32_t pin, GPIO_Pull_t pull)
{
	GPIO_Init(gpio, pin, GPIO_Mode_Input | pull);
}

static inline void GPIO_Deinit(GPIO_t * gpio, uint32_t pin)
{
	GPIO_Init(gpio, pin, GPIO_Mode_Analog);
}

static inline void GPIO_EnableOutput(GPIO_t * gpio, uint32_t pin, GPIO_State_t state)
{
	GPIO_Write(gpio, pin, state);
	GPIO_Init(gpio, pin, GPIO_Mode_Output);
}

// IO calls
static inline void GPIO_Set(GPIO_t * gpio, uint32_t pin)
{
	gpio->BSRR = (uint32_t)pin;
}

static inline void GPIO_Reset(GPIO_t * gpio, uint32_t pin)
{
#ifdef STM32F4
	gpio->BSRR = ((uint32_t)pin) << 16;
#else
	gpio->BRR = (uint32_t)pin;
#endif
}

static inline GPIO_State_t GPIO_Read(GPIO_t * gpio, uint32_t pin)
{
	return ((gpio->IDR & pin) > 0);
}

static inline uint32_t GPIO_ReadPort(GPIO_t * gpio, uint32_t pins)
{
	return gpio->IDR & pins;
}

