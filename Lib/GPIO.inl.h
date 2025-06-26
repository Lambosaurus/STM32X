
/*
 * PRIVATE DEFINITIONS
 */

/*
 * INLINE FUNCTION DEFINITIONS
 */

// Init calls
static inline void GPIO_EnableInput(GPIO_Pin_t pins, GPIO_Pull_t pull)
{
	GPIO_Init(pins, GPIO_Mode_Input | pull);
}

static inline void GPIO_Deinit(GPIO_Pin_t pins)
{
	GPIO_Init(pins, GPIO_Mode_Analog);
}

static inline void GPIO_EnableOutput(GPIO_Pin_t pins, GPIO_State_t state)
{
	GPIO_Write(pins, state);
	GPIO_Init(pins, GPIO_Mode_Output);
}

#ifdef GPIO_USE_IRQS
static inline void EXTI_IRQHandler(uint32_t n)
{
	if (__HAL_GPIO_EXTI_GET_IT(1 << n) != RESET)
	{
		__HAL_GPIO_EXTI_CLEAR_IT(1 << n);
		GPIO_IRQHandler(n);
	}
}
#endif
