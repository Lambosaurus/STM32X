
#include "ADC.h"

/*
 * PRIVATE DEFINITIONS
 */

#ifndef ADC_SAMPLETIME
#define ADC_SAMPLETIME	ADC_SAMPLETIME_79CYCLES_5
#endif

#define _ADC_SELECT(adc, channel) 	(adc->CHSELR = channel & ADC_CHANNEL_MASK)
#define _ADC_READ(adc)				(adc->DR)


#define TS_CAL1_AIN		(*((uint16_t*)0x1FF8007A))
#define TS_CAL2_AIN		(*((uint16_t*)0x1FF8007E))
#define TS_CAL1_DEG		30
#define TS_CAL2_DEG		130
#define TS_CAL_VREF		3000

#define VF_CAL_AIN		(*((uint16_t*)0x1FF80078))
#define VF_CAL_VREF		3000

/*
 * PRIVATE TYPES
 */

/*
 * PRIVATE PROTOTYPES
 */

static void ADC_Calibrate(void);
static void ADC_WaitForFlag(uint32_t flag);

/*
 * PRIVATE VARIABLES
 */

static ADC_HandleTypeDef gAdc;

/*
 * PUBLIC FUNCTIONS
 */


void ADC_Init(void)
{
	__HAL_RCC_ADC1_CLK_ENABLE();

	gAdc.Instance = ADC1;
	gAdc.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV4;
	gAdc.Init.Resolution = ADC_RESOLUTION_12B;
	gAdc.Init.DataAlign = ADC_DATAALIGN_RIGHT;
	gAdc.Init.ScanConvMode = ADC_SCAN_DIRECTION_FORWARD;
	gAdc.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
	gAdc.Init.LowPowerAutoWait = DISABLE;
	gAdc.Init.LowPowerAutoPowerOff = DISABLE;
	gAdc.Init.ContinuousConvMode = DISABLE;
	gAdc.Init.DiscontinuousConvMode = DISABLE;
	gAdc.Init.ExternalTrigConv = ADC_SOFTWARE_START;
	gAdc.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
	gAdc.Init.DMAContinuousRequests = DISABLE;
	gAdc.Init.Overrun = ADC_OVR_DATA_PRESERVED;
	gAdc.Init.LowPowerFrequencyMode = DISABLE;
	gAdc.Init.OversamplingMode = DISABLE;
	gAdc.Init.SamplingTime = ADC_SAMPLETIME;
	HAL_ADC_Init(&gAdc);

	ADC_Calibrate();

	__HAL_ADC_ENABLE(&gAdc);
	ADC_WaitForFlag(ADC_FLAG_RDY);
}

uint32_t ADC_Read(uint32_t channel)
{
	_ADC_SELECT(gAdc.Instance, channel);

	__HAL_ADC_CLEAR_FLAG(&gAdc, (ADC_FLAG_EOC | ADC_FLAG_EOS | ADC_FLAG_OVR));
	gAdc.Instance->CR |= ADC_CR_ADSTART;

	ADC_WaitForFlag(ADC_FLAG_EOC);

	return _ADC_READ(gAdc.Instance);
}

void ADC_Deinit(void)
{
	HAL_ADC_DeInit(&gAdc);
	__HAL_RCC_ADC1_CLK_DISABLE();
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
	ADC->CCR |= ADC_CCR_TSEN;
	int32_t ain = ADC_Read(ADC_CHANNEL_TEMPSENSOR);
	ADC->CCR &= ~ADC_CCR_TSEN;

	// The temp sensor is not ratiometric, so the vref must be adjusted for.
	ain = ain * ADC_VREF / TS_CAL_VREF;
	return ((ain - TS_CAL1_AIN) * (TS_CAL2_DEG - TS_CAL1_DEG) / (TS_CAL2_AIN - TS_CAL1_AIN)) + TS_CAL1_DEG;
}

uint32_t ADC_ReadVRef(void)
{
	ADC->CCR |= ADC_CCR_VREFEN;
	int32_t ain = ADC_Read(ADC_CHANNEL_VREFINT);
	ADC->CCR &= ~ADC_CCR_VREFEN;

	return (VF_CAL_VREF * (uint32_t)VF_CAL_AIN) / ain;
}

/*
 * PRIVATE FUNCTIONS
 */

static void ADC_WaitForFlag(uint32_t flag)
{
	while (!(gAdc.Instance->ISR & flag)) { }
}

static void ADC_Calibrate(void)
{
	// Note, ADC must be disabled for this to occurr
	gAdc.Instance->CR |= ADC_CR_ADCAL;
	while(gAdc.Instance->CR & ADC_CR_ADCAL);
}

/*
 * INTERRUPT ROUTINES
 */
