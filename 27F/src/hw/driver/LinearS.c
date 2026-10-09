#include "LinearS.h"
#include "adc.h"


#define LINEAR_CH_MAX           4U      // 사용하는 ADC 채널 수에 맞게 수정

#define LINEAR_ADC_MAX          4095ULL
#define LINEAR_ADC_REF_MV       3300ULL

#define LINEAR_SENSOR_MAX_MV    5000ULL
#define LINEAR_SENSOR_MAX_MM    75ULL

#define LINEAR_R_TOP            1000ULL
#define LINEAR_R_BOTTOM         1800ULL

// distance = adc * (Vref/ADC_MAX) * ((R1+R2)/R2) * (MAX_MM/MAX_MV)
// 곱셈을 먼저, 나눗셈은 마지막 한 번만 해서 누적 오차를 없앰
#define LINEAR_NUM  (LINEAR_ADC_REF_MV * (LINEAR_R_TOP + LINEAR_R_BOTTOM) * LINEAR_SENSOR_MAX_MM)
#define LINEAR_DEN  (LINEAR_ADC_MAX * LINEAR_R_BOTTOM * LINEAR_SENSOR_MAX_MV)

static bool isInit[LINEAR_CH_MAX] = { false, };

bool LinearSOpen(uint8_t ch)
{
	if (ch >= LINEAR_CH_MAX)
	{
		return false;
	}

	if (adcOpen(ch))
	{
		isInit[ch] = true;
	}

	return isInit[ch];
}

uint16_t LinearGetValue(uint8_t ch)
{
	uint64_t adc_val;
	uint64_t distance;

	if (ch >= LINEAR_CH_MAX || !isInit[ch])
	{
		return 0;
	}

	adc_val = adcReceive(ch);

	// ADC 범위 보호
	if (adc_val > LINEAR_ADC_MAX)
	{
		adc_val = LINEAR_ADC_MAX;
	}

	// 반올림 포함 한 번에 계산 (최대 약 2.8e12, uint64_t 필요)
	distance = (adc_val * LINEAR_NUM + (LINEAR_DEN / 2ULL)) / LINEAR_DEN;

	// 센서 풀스케일 초과 보호 (Vref/저항 오차로 75mm 넘는 경우 방지)
	if (distance > LINEAR_SENSOR_MAX_MM)
	{
		distance = LINEAR_SENSOR_MAX_MM;
	}

	return (uint16_t)distance;
}
