/*
 * adc.c
 *
 *  Created on: 2026. 5. 29.
 *      Author: yougang
 */


#include "adc.h"

#ifdef _USE_HW_ADC
ADC_HandleTypeDef hadc1;

typedef struct{
  uint8_t            channel;
  ADC_HandleTypeDef* ADC_Handle;
  bool               isInit;
  bool               isOpen;
}ADC_tbl_t;

ADC_tbl_t ADC_tbl[ADC_MAX_CH] = {
    {DEF_ADC1, &hadc1, false, false},
};


void adcInit(void)
{
  for(int i=0; i<ADC_MAX_CH; i++)
  {
    ADC_tbl[i].isInit=true;
    ADC_tbl[i].isOpen=false;
   }
}


bool adcOpen(uint8_t ch)
{
  bool ret= false;
  ADC_tbl_t* p_adc = &ADC_tbl[ch];
  ADC_HandleTypeDef* p_handle = p_adc->ADC_Handle;
  ADC_MultiModeTypeDef multimode = {0};
  ADC_ChannelConfTypeDef sConfig = {0};

  switch(ch)
  {
    case DEF_ADC1:
      /** Common config */
      p_handle->Instance = ADC1;
      p_handle->Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
      p_handle->Init.Resolution = ADC_RESOLUTION_12B;
      p_handle->Init.DataAlign = ADC_DATAALIGN_RIGHT;
      p_handle->Init.GainCompensation = 0;
      p_handle->Init.ScanConvMode = ADC_SCAN_DISABLE;
      p_handle->Init.EOCSelection = ADC_EOC_SINGLE_CONV;
      p_handle->Init.LowPowerAutoWait = DISABLE;
      p_handle->Init.ContinuousConvMode = DISABLE;
      p_handle->Init.NbrOfConversion = 1;
      p_handle->Init.DiscontinuousConvMode = DISABLE;
      p_handle->Init.ExternalTrigConv = ADC_SOFTWARE_START;
      p_handle->Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
      p_handle->Init.DMAContinuousRequests = DISABLE;
      p_handle->Init.Overrun = ADC_OVR_DATA_PRESERVED;
      p_handle->Init.OversamplingMode = DISABLE;

      if (HAL_ADC_Init(p_handle) == HAL_OK)
      {
        ret=true;
      }
      else
      {
        ret= false;
      }

      /** Configure the ADC multi-mode */
      multimode.Mode = ADC_MODE_INDEPENDENT;
      if (HAL_ADCEx_MultiModeConfigChannel(p_handle, &multimode) != HAL_OK)
      {
        ret= false;
      }

      /** Configure Regular Channel */
      sConfig.Channel = ADC_CHANNEL_1;
      sConfig.Rank = ADC_REGULAR_RANK_1;
      sConfig.SamplingTime = ADC_SAMPLETIME_2CYCLES_5;
      sConfig.SingleDiff = ADC_SINGLE_ENDED;
      sConfig.OffsetNumber = ADC_OFFSET_NONE;
      sConfig.Offset = 0;
      if (HAL_ADC_ConfigChannel(p_handle, &sConfig) != HAL_OK)
      {
        ret= false;
      }
      p_adc->isOpen=ret;

      break;
    default:
      break;
  }
    return ret;
}

uint32_t adcReceive(uint8_t ch)
{
  uint32_t adcVal=0;
  switch(ch)
  {
    case DEF_ADC1:
      if(HAL_ADC_Start(ADC_tbl[ch].ADC_Handle) == HAL_OK)
      {
        if (HAL_ADC_PollForConversion(ADC_tbl[ch].ADC_Handle, 10) == HAL_OK)
        {
          adcVal = HAL_ADC_GetValue(&hadc1);
        }
      }

      break;
    default:
      break;
  }
  HAL_ADC_Stop(ADC_tbl[ch].ADC_Handle);
  return adcVal;
}


void HAL_ADC_MspInit(ADC_HandleTypeDef* adcHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};
  if(adcHandle->Instance==ADC1)
  {
  /* USER CODE BEGIN ADC1_MspInit 0 */

  /* USER CODE END ADC1_MspInit 0 */

  /** Initializes the peripherals clocks
  */
    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC12;
    PeriphClkInit.Adc12ClockSelection = RCC_ADC12CLKSOURCE_SYSCLK;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
    {
      Error_Handler();
    }

    /* ADC1 clock enable */
    __HAL_RCC_ADC12_CLK_ENABLE();

    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**ADC1 GPIO Configuration
    PA0     ------> ADC1_IN1
    PA1     ------> ADC1_IN2
    PA2     ------> ADC1_IN3
    PA3     ------> ADC1_IN4
    */
    GPIO_InitStruct.Pin = FL_ADC_Pin|FR_ADC_Pin|BL_ADC_Pin|BR_ADC_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* USER CODE BEGIN ADC1_MspInit 1 */

  /* USER CODE END ADC1_MspInit 1 */
  }
}

void HAL_ADC_MspDeInit(ADC_HandleTypeDef* adcHandle)
{

  if(adcHandle->Instance==ADC1)
  {
  /* USER CODE BEGIN ADC1_MspDeInit 0 */

  /* USER CODE END ADC1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_ADC12_CLK_DISABLE();

    /**ADC1 GPIO Configuration
    PA0     ------> ADC1_IN1
    PA1     ------> ADC1_IN2
    PA2     ------> ADC1_IN3
    PA3     ------> ADC1_IN4
    */
    HAL_GPIO_DeInit(GPIOA, FL_ADC_Pin|FR_ADC_Pin|BL_ADC_Pin|BR_ADC_Pin);

  /* USER CODE BEGIN ADC1_MspDeInit 1 */

  /* USER CODE END ADC1_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

#endif
