/*
 * adc.c
 *
 *  Created on: 2026. 5. 29.
 *      Author: yougang
 */


#include "adc.h"

#ifdef _USE_HW_ADC
ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;

typedef struct{
  uint8_t            channel;
  ADC_HandleTypeDef* ADC_Handle;
  DMA_HandleTypeDef* DMA_Handle;
  bool               isInit;
  bool               isOpen;
}ADC_tbl_t;

ADC_tbl_t ADC_tbl[ADC_MAX_CH] = {
    {DEF_ADC1, &hadc1, &hdma_adc1, false, false},
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
  switch(ch)
  {
    case DEF_ADC1:
      __HAL_RCC_DMA2_CLK_ENABLE();
      HAL_NVIC_SetPriority(DMA2_Stream4_IRQn, 0, 0);
      HAL_NVIC_EnableIRQ(DMA2_Stream4_IRQn);

      ADC_ChannelConfTypeDef sConfig = {0};
      p_handle->Instance = ADC1;
      p_handle->Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV8;
      p_handle->Init.Resolution = ADC_RESOLUTION_12B;
      p_handle->Init.ScanConvMode = DISABLE;
      p_handle->Init.ContinuousConvMode = ENABLE;
      p_handle->Init.DiscontinuousConvMode = DISABLE;
      p_handle->Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
      p_handle->Init.ExternalTrigConv = ADC_SOFTWARE_START;
      p_handle->Init.DataAlign = ADC_DATAALIGN_RIGHT;
      p_handle->Init.NbrOfConversion = 1;
      p_handle->Init.DMAContinuousRequests = ENABLE;
      p_handle->Init.EOCSelection = ADC_EOC_SINGLE_CONV;

      if (HAL_ADC_Init(p_handle) == HAL_OK)
      {
        ret=true;
      }
      else
      {
        ret= false;
      }
      sConfig.Channel = ADC_CHANNEL_8;
      sConfig.Rank = 1;
      sConfig.SamplingTime = ADC_SAMPLETIME_480CYCLES;
      if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
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

bool adcReceive_DMA(uint8_t ch, uint32_t* des_buf, uint32_t length)
{
  bool ret=false;
  HAL_StatusTypeDef status;
  switch(ch)
  {
    case DEF_ADC1:
      status = HAL_ADC_Start_DMA(ADC_tbl[ch].ADC_Handle, des_buf, length);
      break;
    default:
      break;
  }
  if(status == HAL_OK)
  {
    ret= true;
  }
  return ret;
}



void HAL_ADC_MspInit(ADC_HandleTypeDef* adcHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(adcHandle->Instance==ADC1)
  {
  /* USER CODE BEGIN ADC1_MspInit 0 */

  /* USER CODE END ADC1_MspInit 0 */
    /* ADC1 clock enable */
    __HAL_RCC_ADC1_CLK_ENABLE();

    __HAL_RCC_GPIOB_CLK_ENABLE();
    /**ADC1 GPIO Configuration
    PB0     ------> ADC1_IN8
    */
    GPIO_InitStruct.Pin = GPIO_PIN_0;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* ADC1 DMA Init */
    /* ADC1 Init */
    hdma_adc1.Instance = DMA2_Stream4;
    hdma_adc1.Init.Channel = DMA_CHANNEL_0;
    hdma_adc1.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_adc1.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_adc1.Init.MemInc = DMA_MINC_DISABLE;
    hdma_adc1.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    hdma_adc1.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
    hdma_adc1.Init.Mode = DMA_CIRCULAR;
    hdma_adc1.Init.Priority = DMA_PRIORITY_LOW;  // DMA 점유 우선순서.... dma.c는 dma 인터럽트 처리 우선순위
    hdma_adc1.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    if (HAL_DMA_Init(&hdma_adc1) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(adcHandle,DMA_Handle,hdma_adc1);

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
    __HAL_RCC_ADC1_CLK_DISABLE();

    /**ADC1 GPIO Configuration
    PB0     ------> ADC1_IN8
    */
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_0);

    /* ADC1 DMA DeInit */
    HAL_DMA_DeInit(adcHandle->DMA_Handle);
  /* USER CODE BEGIN ADC1_MspDeInit 1 */

  /* USER CODE END ADC1_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

#endif


