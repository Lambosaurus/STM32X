
#include "ADC.h"
#include "CLK.h"
#include "US.h"

#ifdef ADC_DMA_CH
#include "DMA.h"
#endif

/*
 * PRIVATE DEFINITIONS
 */

#if defined(STM32L0)

#define ADC_CLOCK_PRESCALAR			ADC_CLOCK_ASYNC_DIV4
#define ADC_CLOCK_FREQ				(16000000 / 4) // 16MHz HSI div 4.
#define ADC_SMPR_DEFAULT			ADC_SAMPLETIME_79CYCLES_5 // Gives ~ 20us sample time

#define TS_CAL1_AIN					(*((uint16_t*)0x1FF8007A))
#define TS_CAL2_AIN					(*((uint16_t*)0x1FF8007E))
#define TS_CAL1_DEG					30
#define TS_CAL2_DEG					130
#define TS_CAL_VREF					3000

#define VF_CAL_AIN					(*((uint16_t*)0x1FF80078))
#define VF_CAL_VREF					3000

#define ADC_Channel_Vref			ADC_Channel_17
#define ADC_Channel_Temp			ADC_Channel_18

// refer to HAL on this implementation
#define _ADC_CLOCK_PRESCALER(adcx, prescalar)         \
  do{                                                     \
		adcx->CFGR2 &= ~(ADC_CFGR2_CKMODE);                \
		MODIFY_REG(ADC_COMMON->CCR, ADC_CCR_PRESC, prescalar);   \
  } while(0)

#define ADC_CFGR1_CONTINUOUS		ADC_CONTINUOUS
#define ADC_CFGR1_DMACONTREQ		ADC_DMACONTREQ
#define ADC_CFGR1_AUTOOFF			__HAL_ADC_CFGR1_AUTOFF
#define ADC_CFGR1_AUTOWAIT			__HAL_ADC_CFGR1_AutoDelay

#define ADC_COMMON					ADC

#elif defined(STM32F0)

// This puts sample + conversion time at 18us.
#define ADC_CLOCK_PRESCALAR			ADC_CLOCK_ASYNC_DIV1
#define ADC_CLOCK_FREQ				14000000 // ADC Asynchronous clock is 14MHz
#define ADC_SMPR_DEFAULT			ADC_SAMPLETIME_239CYCLES_5 // Gives about 17us sample time.

#define TS_CAL1_AIN					(*((uint16_t*)0x1FFFF7B8))
#define TS_CAL2_AIN					(*((uint16_t*)0x1FFFF7C2))
#define TS_CAL1_DEG					30
#define TS_CAL2_DEG					110
#define TS_CAL_VREF					3300

#define VF_CAL_AIN					(*((uint16_t*)0x1FFFF7BA))
#define VF_CAL_VREF					3300

#define ADC_Channel_Vref			ADC_Channel_17
#define ADC_Channel_Temp			ADC_Channel_16

#define _ADC_CLOCK_PRESCALER(adcx, prescalar) MODIFY_REG(adcx->CFGR2, ADC_CFGR2_CKMODE, prescalar)
#define ADC_SMPR_SMPR				ADC_SMPR_SMP

#define ADC_FLAG_ALL				(ADC_FLAG_AWD | ADC_FLAG_OVR | ADC_FLAG_EOS | ADC_FLAG_RDY | ADC_FLAG_EOC | ADC_FLAG_EOSMP)

#ifndef ADC_COMMON
#define ADC_COMMON 					ADC1_COMMON
#endif

#elif defined(STM32G0) || defined(STM32WL)

#define ADC_CLOCK_PRESCALAR			ADC_CLOCK_ASYNC_DIV4
#define ADC_CLOCK_FREQ				(16000000 / 4) // 16MHz HSI div 4.
#define ADC_SMPR_DEFAULT			ADC_SAMPLETIME_79CYCLES_5 // Gives ~ 20us sample time
#define _ADC_SELECT(adc, channel) 	(adc->CHSELR = channel & ADC_CHANNEL_ID_BITFIELD_MASK)

// refer to HAL on this implementation
#define _ADC_CLOCK_PRESCALER(adcx, prescalar)         		\
  do{                                                       \
		adcx->CFGR2 &= ~(ADC_CFGR2_CKMODE);                 \
		MODIFY_REG(ADC_COMMON->CCR, ADC_CCR_PRESC, prescalar);\
  } while(0)

#define TS_CAL1_AIN					(*((uint16_t*)0x1FFF75A8))
#define TS_CAL2_AIN					(*((uint16_t*)0x1FFF75CA))
#define TS_CAL1_DEG					30
#define TS_CAL2_DEG					130
#define TS_CAL_VREF					3000

#define VF_CAL_AIN					(*((uint16_t*)0x1FFF75AA))
#define VF_CAL_VREF					3000

#define ADC_Channel_Vref			ADC_Channel_13
#define ADC_Channel_Temp			ADC_Channel_12

#define __HAL_RCC_ADC1_CLK_ENABLE	__HAL_RCC_ADC_CLK_ENABLE
#define __HAL_RCC_ADC1_CLK_DISABLE	__HAL_RCC_ADC_CLK_DISABLE
#define ADC_FLAG_ALL				(ADC_FLAG_OVR | ADC_FLAG_EOS | ADC_FLAG_RDY | ADC_FLAG_EOC | ADC_FLAG_EOSMP)
#define ADC_SMPR_SMPR				ADC_SMPR_SMP1_Msk
#define ADC_SMPR_SMP_Pos			ADC_SMPR_SMP1_Pos
#define ADC_SCANDIR(x)				ADC_SCAN_SEQ_MODE(x)

#define __HAL_ADC_ENABLE(adc)		((adc)->Instance->CR |= ADC_CR_ADEN)
#define __HAL_ADC_DISABLE(adc)		((adc)->Instance->CR |= ADC_CR_ADDIS)

#if defined(STM32WL)
#define ADC1						ADC
#else
#define ADC_COMMON					ADC
#endif

#endif


// There is a conversion time of 12.5 cycles. The .5 is wrapped into the constant.
#define ADC_CLKS(sample_clks)		(sample_clks + 13)
#define ADCx						ADC1

/*
 * PRIVATE TYPES
 */

/*
 * PRIVATE PROTOTYPES
 */

static void ADC_Calibrate(void);
static void ADC_WaitForFlag(uint32_t flag);
static uint32_t ADC_SelectSampleTime(uint32_t desired, uint32_t * frequency);
static void ADC_StopConversion(void);

/*
 * PRIVATE VARIABLES
 */

typedef struct {
	ADC_TypeDef * Instance;
} ADC_t;

static ADC_t gADC = {
	.Instance = ADC1
};

/*
 * PUBLIC FUNCTIONS
 */

void ADC_Init(void)
{
	CLK_EnableADCCLK();
	__HAL_RCC_ADC1_CLK_ENABLE();

#ifdef _ADC_CLOCK_PRESCALER
	_ADC_CLOCK_PRESCALER(ADCx, ADC_CLOCK_PRESCALAR);
#endif

#if ADC_CCR_LFMEN && !defined(STM32G0)
	// Disable the low power mode
	MODIFY_REG(ADC_COMMON->CCR, ADC_CCR_LFMEN, __HAL_ADC_CCR_LOWFREQUENCY(DISABLE));
#endif

#ifdef ADC_CR_ADVREGEN
	// Enable the voltage regulator
	if (HAL_IS_BIT_CLR(ADCx->CR, ADC_CR_ADVREGEN))
	{
		ADCx->CR |= ADC_CR_ADVREGEN;
		// Wait for regulator stability.
		US_Delay(20);
	}
#endif

	ADCx->CFGR1 = ADC_DATAALIGN_RIGHT
		| ADC_SCANDIR( ADC_SCAN_DIRECTION_FORWARD )
		| ADC_CFGR1_CONTINUOUS(DISABLE)
		| ADC_CFGR1_DMACONTREQ(DISABLE)
		| ADC_CFGR1_AUTOWAIT(DISABLE)
		| ADC_CFGR1_AUTOOFF(DISABLE)
		| ADC_OVR_DATA_PRESERVED
		| ADC_RESOLUTION_12B;

	ADCx->CFGR2 = 0;
	// Configure the default sampling rate
	MODIFY_REG(ADCx->SMPR, ADC_SMPR_SMPR, ADC_SMPR_DEFAULT);

	ADC_Calibrate();
	__HAL_ADC_ENABLE(&gADC);
	ADC_WaitForFlag(ADC_FLAG_RDY);
}

uint32_t ADC_SetFreq(uint32_t target)
{
	uint32_t actual;
	uint32_t smpr = ADC_SelectSampleTime(target, &actual);
	MODIFY_REG(ADC1->SMPR, ADC_SMPR_SMPR, smpr);
	return actual;
}

#if defined(STM32L0) || defined(STM32WL)
void ADC_SetOversampling(uint32_t ratio)
{
	if (ratio > 1)
	{
		// Find the power 2 of the requested oversampling ratio.
		uint32_t osr = 0;
		while (ratio > 1)
		{
			ratio >>= 1;
			osr++;
		}

		// We are configuring both the oversampling ratio, and the shift ratio.
		// This cancels out the oversampling - keeping all the ADC figures within ADC_MAX
		MODIFY_REG(ADC1->CFGR2,
				(ADC_CFGR2_OVSR | ADC_CFGR2_OVSS | ADC_CFGR2_TOVS | ADC_CFGR2_OVSE),
				  ADC_CFGR2_OVSE | ADC_TRIGGEREDMODE_SINGLE_TRIGGER
				| ((osr-1) << ADC_CFGR2_OVSR_Pos) // OVSR = 0 yields *2 oversampling ratio
				| (osr << ADC_CFGR2_OVSS_Pos)     // OVSS = 1 yields /2 bit shift
				);
	}
	else
	{
		// Disable oversampling.
		ADC1->CFGR2 &= ~ADC_CFGR2_OVSE;
	}
}
#endif

uint32_t ADC_Read(ADC_Channel_t channel)
{
	__HAL_ADC_CLEAR_FLAG(&gADC, (ADC_FLAG_EOC | ADC_FLAG_EOS | ADC_FLAG_OVR));

	ADCx->CHSELR = channel;

	// Put it back in single shot mode.
	MODIFY_REG( ADCx->CFGR1,
				ADC_CFGR1_DMACFG | ADC_CFGR1_CONT | ADC_CFGR1_DMAEN,
				ADC_CFGR1_CONTINUOUS(DISABLE) | ADC_CFGR1_DMACONTREQ(DISABLE)
			);

	ADCx->CR |= ADC_CR_ADSTART;

	ADC_WaitForFlag(ADC_FLAG_EOC);

	return ADCx->DR;
}

void ADC_Deinit(void)
{
	ADC_StopConversion();

	// at this point, the ADC_CR_ADSTART must not be set.
	__HAL_ADC_DISABLE(&gADC);
	while(ADCx->CR & ADC_CR_ADEN);

	ADCx->IER = 0;
	__HAL_ADC_CLEAR_FLAG(&gADC, ADC_FLAG_ALL);

#ifdef ADC_CR_ADVREGEN
	ADCx->CR &= ~ADC_CR_ADVREGEN;
#endif

	ADCx->CFGR1 = 0;
	ADCx->CFGR2 = 0;
	ADCx->SMPR = 0;
#if !(defined(STM32G0) || defined(STM32WL))
	ADCx->TR = 0;
#endif

	__HAL_RCC_ADC1_CLK_DISABLE();
	CLK_DisableADCCLK();
}

uint32_t AIN_AinToDivider(uint32_t ain, uint32_t rlow, uint32_t rhigh)
{
	return AIN_AinToMv(ain) * (rhigh + rlow) / rlow;
}

uint32_t AIN_AinToMv(uint32_t ain)
{
	return (ain * ADC_VREF) / ADC_MAX;
}

int32_t ADC_ReadDieTemp(void)
{
	ADC_COMMON->CCR |= ADC_CCR_TSEN;
	US_Delay(10);
	int32_t ain = ADC_Read(ADC_Channel_Temp);
	ADC_COMMON->CCR &= ~ADC_CCR_TSEN;

	// The temp sensor is not ratiometric, so the vref must be adjusted for.
	ain = ain * ADC_VREF / TS_CAL_VREF;
	return ((ain - TS_CAL1_AIN) * (TS_CAL2_DEG - TS_CAL1_DEG) / (TS_CAL2_AIN - TS_CAL1_AIN)) + TS_CAL1_DEG;
}

uint32_t ADC_ReadVRef(void)
{
	ADC_COMMON->CCR |= ADC_CCR_VREFEN;
	US_Delay(10);
	int32_t ain = ADC_Read(ADC_Channel_Vref);
	ADC_COMMON->CCR &= ~ADC_CCR_VREFEN;

	return (VF_CAL_VREF * (uint32_t)VF_CAL_AIN) / ain;
}

void ADC_ReadTempNoise(uint8_t * bfr, uint32_t size)
{
	ADC_COMMON->CCR |= ADC_CCR_TSEN;
	US_Delay(10);
	while (size--)
	{
		uint8_t b = 0;
		for (uint32_t i = 0; i < 8; i++)
		{
			b |= (ADC_Read(ADC_Channel_Temp) & 0x01) << i;
		}
		*bfr++ = b;
	}

	ADC_COMMON->CCR &= ~ADC_CCR_TSEN;
}

#ifdef ADC_DMA_CH

void ADC_Start(ADC_Channel_t channel, uint16_t * buffer, uint32_t count, bool circular, ADC_Callback_t callback)
{
	// These flags can cause the previous ADC sample to be sent via DMA if not cleared.
	__HAL_ADC_CLEAR_FLAG(&gADC, ADC_FLAG_ALL);

	// Select the channel
	ADCx->CHSELR = channel;

	// Enable DMA, with optional circular mode.
	MODIFY_REG( ADCx->CFGR1,
				ADC_CFGR1_DMACFG | ADC_CFGR1_CONT | ADC_CFGR1_DMAEN,
				ADC_CFGR1_CONTINUOUS(ENABLE) | ADC_CFGR1_DMACONTREQ(circular) | ADC_CFGR1_DMAEN
			);

	DMA_Flags_t flags = DMA_Dir_FromPeriph | DMA_MemSize_HalfWord | DMA_PeriphSize_Word;

	if (circular)
	{
		flags |= DMA_Mode_Circular;
	}

	DMA_Init(ADC_DMA_CH, (void*)&ADCx->DR, buffer, count, flags, (DMA_Callback_t)callback);
	ADCx->CR |= ADC_CR_ADSTART;
}

void ADC_Stop(void)
{
	ADC_StopConversion();
	DMA_Deinit(ADC_DMA_CH);
}

#endif // ADC_DMA_CH

/*
 * PRIVATE FUNCTIONS
 */

static void ADC_StopConversion(void)
{
	// Are we running?
	if (ADCx->CR & ADC_CR_ADSTART)
	{
		// Issue stop request
		ADCx->CR |= ADC_CR_ADSTP;
		// Wait for stop
		while(ADCx->CR & ADC_CR_ADSTART);
	}
}

static void ADC_WaitForFlag(uint32_t flag)
{
	while (!__HAL_ADC_GET_FLAG(&gADC, flag));
}

static void ADC_Calibrate(void)
{
	// Note, ADC must be disabled for this to occurr
	ADCx->CR |= ADC_CR_ADCAL;
	while(ADCx->CR & ADC_CR_ADCAL);
}

static uint32_t ADC_SelectSampleTime(uint32_t desired, uint32_t * frequency)
{
#if defined(STM32L0) || defined(STM32G0) || defined(STM32WL)
	const uint16_t sample_times[] = {
		// STM32L0 has 12.5 cycle conversion time.
		ADC_CLKS(1), 	// ADC_SAMPLETIME_1CYCLE_5
		ADC_CLKS(3), 	// ADC_SAMPLETIME_3CYCLES_5
		ADC_CLKS(7), 	// ADC_SAMPLETIME_7CYCLES_5
		ADC_CLKS(12), 	// ADC_SAMPLETIME_12CYCLES_5
		ADC_CLKS(19), 	// ADC_SAMPLETIME_19CYCLES_5
		ADC_CLKS(39), 	// ADC_SAMPLETIME_39CYCLES_5
		ADC_CLKS(79), 	// ADC_SAMPLETIME_79CYCLES_5
		ADC_CLKS(160), 	// ADC_SAMPLETIME_160CYCLES_5
	};
#elif defined(STM32F0)
	const uint16_t sample_times[] = {
		// STM32LF0 has 12.5 cycle conversion time.
		ADC_CLKS(1), 	// ADC_SAMPLETIME_1CYCLE_5
		ADC_CLKS(7), 	// ADC_SAMPLETIME_7CYCLES_5
		ADC_CLKS(13), 	// ADC_SAMPLETIME_13CYCLES_5
		ADC_CLKS(28), 	// ADC_SAMPLETIME_28CYCLES_5
		ADC_CLKS(41), 	// ADC_SAMPLETIME_41CYCLES_5
		ADC_CLKS(55), 	// ADC_SAMPLETIME_55CYCLES_5
		ADC_CLKS(71), 	// ADC_SAMPLETIME_71CYCLES_5
		ADC_CLKS(239), 	// ADC_SAMPLETIME_239CYCLES_5
	};
#endif

	uint32_t actual;
	uint32_t i;
	for (i = 0; i < LENGTH(sample_times); i++)
	{
		actual = ADC_CLOCK_FREQ / (uint32_t)sample_times[i];
		if (actual <= desired)
		{
			break;
		}
	}
	*frequency = actual;
	return i << ADC_SMPR_SMP_Pos;
}

/*
 * INTERRUPT ROUTINES
 */
