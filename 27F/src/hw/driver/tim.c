/*
 * tim.c
 *
 *  Created on: 2026. 2. 8.
 *      Author: yougang
 */


#include "tim.h"

#ifdef _USE_HW_TIM


// ---------------------------------------------------------------------

// ---------------------------------------------------------------------
TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim5;
TIM_HandleTypeDef htim7;

// ---------------------------------------------------------------------
// 구
// ---------------------------------------------------------------------
typedef struct {
  uint32_t           channel;
  TIM_HandleTypeDef* htim;
  bool               isInit;
  bool               isOpen;
} TIM_Base_tbl;

typedef struct {
  volatile bool     Flag_1ms;
  volatile bool     Flag_20ms;
  volatile bool     Flag_100ms;
  volatile bool     Flag_1000ms;
  volatile uint32_t counter_20ms;
  volatile uint32_t counter_100ms;
  volatile uint32_t counter_1000ms;
} TIMER_Flag_tbl;

typedef struct {
  uint32_t freq;
  uint32_t max_output;
  bool     isInit;
} PWM_Status_tbl;

typedef struct {
  TIM_Base_tbl   tim;
  TIMER_Flag_tbl timerFlag;
} TIMER_tbl;

typedef struct {
  TIM_Base_tbl   tim;
  PWM_Status_tbl pwmStatus;
} PWM_tbl;


static PWM_tbl pwm_tbl[PWM_MAX_CH] = {
    {{TIM_CHANNEL_4, &htim3, false, false}, {0, 0, false}},
    {{TIM_CHANNEL_1, &htim5, false, false}, {0, 0, false}},
    {{TIM_CHANNEL_2, &htim5, false, false}, {0, 0, false}},
    {{TIM_CHANNEL_3, &htim5, false, false}, {0, 0, false}},
    {{TIM_CHANNEL_4, &htim5, false, false}, {0, 0, false}},
};

static TIMER_tbl timer_tbl[TIMER_MAX_CH] = {
    {{0xFF, &htim7, false, false}, {false, false, 0}},
};

static bool pwmOpen   (uint8_t hw_ch);
static bool timerOpen (uint8_t hw_ch);
static bool pwmPsc    (uint8_t hw_ch, uint32_t psc);
static bool timerPsc  (uint8_t hw_ch, uint32_t psc);


void timInit(void)
{
  for (int i = 0; i < PWM_MAX_CH; i++) {
    pwm_tbl[i].tim.isInit = true;
    pwm_tbl[i].tim.isOpen = false;
  }
  for (int i = 0; i < TIMER_MAX_CH; i++) {
    timer_tbl[i].tim.isInit = true;
    timer_tbl[i].tim.isOpen = false;
  }
}

bool istimOpen(uint8_t ch)
{
  uint8_t idx = TIM_GET_INDEX(ch);
  if (TIM_IS_TIMER(ch))
  {
    if (idx >= TIMER_MAX_CH)
    {
      return false;
    }
    return timer_tbl[ch].tim.isOpen;
  }
  else
  {
    if (idx >= PWM_MAX_CH)
    {
      return false;
    }
    return pwm_tbl[ch].tim.isOpen;
  }
}

bool timOpen(uint8_t ch)
{
  uint8_t idx = TIM_GET_INDEX(ch);
  if (TIM_IS_TIMER(ch)) {
    if (idx >= TIMER_MAX_CH) return false;
    return timerOpen(idx);
  } else {
    if (idx >= PWM_MAX_CH) return false;
    return pwmOpen(idx);
  }
}

static bool pwmOpen(uint8_t hw_ch)
{
  TIM_ClockConfigTypeDef  sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig      = {0};
  TIM_OC_InitTypeDef      sConfigOC          = {0};

  TIM_Base_tbl*      p_tim    = &pwm_tbl[hw_ch].tim;
  TIM_HandleTypeDef* p_handle = p_tim->htim;

  switch (hw_ch) {
    case HW_DEF_TIM3_CH4:
      p_handle->Instance                 = TIM3;
      p_handle->Init.Prescaler           = 1000 - 1;
      p_handle->Init.CounterMode         = TIM_COUNTERMODE_UP;
      p_handle->Init.Period              = 21 - 1;
      p_handle->Init.ClockDivision       = TIM_CLOCKDIVISION_DIV1;
      p_handle->Init.AutoReloadPreload   = TIM_AUTORELOAD_PRELOAD_DISABLE;
      if (HAL_TIM_Base_Init(p_handle) != HAL_OK)          { Error_Handler(); }
      sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
      if (HAL_TIM_ConfigClockSource(p_handle, &sClockSourceConfig) != HAL_OK) { Error_Handler(); }
      if (HAL_TIM_PWM_Init(p_handle) != HAL_OK)           { Error_Handler(); }
      sMasterConfig.MasterOutputTrigger  = TIM_TRGO_RESET;
      sMasterConfig.MasterSlaveMode      = TIM_MASTERSLAVEMODE_DISABLE;
      if (HAL_TIMEx_MasterConfigSynchronization(p_handle, &sMasterConfig) != HAL_OK) { Error_Handler(); }
      sConfigOC.OCMode     = TIM_OCMODE_PWM1;
      sConfigOC.Pulse      = 10;
      sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
      sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
      if (HAL_TIM_PWM_ConfigChannel(p_handle, &sConfigOC, p_tim->channel) != HAL_OK) { Error_Handler(); }
      HAL_TIM_MspPostInit(p_handle);
      p_tim->isOpen = true;
      HAL_TIM_PWM_Start(p_tim->htim, p_tim->channel);
      break;

    case HW_DEF_TIM5_CH1:
    case HW_DEF_TIM5_CH2:
    case HW_DEF_TIM5_CH3:
    case HW_DEF_TIM5_CH4:

      if (!p_handle->Instance) {
        p_handle->Instance               = TIM5;
        p_handle->Init.Prescaler         = 0;
        p_handle->Init.CounterMode       = TIM_COUNTERMODE_UP;
        p_handle->Init.Period            = 41999;
        p_handle->Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;
        p_handle->Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
        if (HAL_TIM_Base_Init(p_handle) != HAL_OK)       { Error_Handler(); }
        sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
        if (HAL_TIM_ConfigClockSource(p_handle, &sClockSourceConfig) != HAL_OK) { Error_Handler(); }
        if (HAL_TIM_PWM_Init(p_handle) != HAL_OK)        { Error_Handler(); }
        sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
        sMasterConfig.MasterSlaveMode     = TIM_MASTERSLAVEMODE_DISABLE;
        if (HAL_TIMEx_MasterConfigSynchronization(p_handle, &sMasterConfig) != HAL_OK) { Error_Handler(); }
      }

      sConfigOC.OCMode     = TIM_OCMODE_PWM1;
      sConfigOC.Pulse      = 0;
      sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
      sConfigOC.OCFastMode = TIM_OCFAST_ENABLE;
      if (HAL_TIM_PWM_ConfigChannel(p_handle, &sConfigOC, p_tim->channel) != HAL_OK) { Error_Handler(); }
      HAL_TIM_MspPostInit(p_handle);
      p_tim->isOpen = true;
      HAL_TIM_PWM_Start(p_tim->htim, p_tim->channel);
      break;

    default:
      return false;
  }
  return true;
}


static bool timerOpen(uint8_t hw_ch)
{
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  TIM_Base_tbl*      p_tim    = &timer_tbl[hw_ch].tim;
  TIM_HandleTypeDef* p_handle = p_tim->htim;

  switch (hw_ch) {
    case HW_DEF_TIM7:
      p_handle->Instance               = TIM7;
      p_handle->Init.Prescaler         = 41999;
      p_handle->Init.CounterMode       = TIM_COUNTERMODE_UP;
      p_handle->Init.Period            = 1;
      p_handle->Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
      if (HAL_TIM_Base_Init(p_handle) != HAL_OK) { Error_Handler(); }
      sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
      sMasterConfig.MasterSlaveMode     = TIM_MASTERSLAVEMODE_DISABLE;
      if (HAL_TIMEx_MasterConfigSynchronization(p_handle, &sMasterConfig) != HAL_OK) { Error_Handler(); }
      p_tim->isOpen = true;
      HAL_TIM_Base_Start_IT(p_handle);
      break;

    default:
      return false;
  }
  return true;
}


bool timDeinit(uint8_t ch)
{
  uint8_t idx = TIM_GET_INDEX(ch);
  TIM_HandleTypeDef* p_handle;

  if (TIM_IS_TIMER(ch)) {
    if (idx >= TIMER_MAX_CH) return false;
    p_handle = timer_tbl[idx].tim.htim;
    timer_tbl[idx].tim.isOpen = false;
  } else {
    if (idx >= PWM_MAX_CH) return false;
    p_handle = pwm_tbl[idx].tim.htim;
    pwm_tbl[idx].tim.isOpen = false;
  }

  return (HAL_TIM_Base_Stop(p_handle) == HAL_OK);
}


bool timPsc(uint8_t ch, uint32_t psc)
{
  uint8_t idx = TIM_GET_INDEX(ch);
  if (TIM_IS_TIMER(ch)) {
    return timerPsc(idx, psc);
  } else {
    return pwmPsc(idx, psc);
  }
}

static bool pwmPsc(uint8_t hw_ch, uint32_t psc)
{
  if (hw_ch >= PWM_MAX_CH) return false;

  switch (hw_ch) {
    case HW_DEF_TIM3_CH4:
      TIM3->PSC = psc;
      break;
    case HW_DEF_TIM5_CH1:
    case HW_DEF_TIM5_CH2:
    case HW_DEF_TIM5_CH3:
    case HW_DEF_TIM5_CH4:
      TIM5->PSC = psc;
      break;
    default:
      return false;
  }
  return true;
}

static bool timerPsc(uint8_t hw_ch, uint32_t psc)
{
  if (hw_ch >= TIMER_MAX_CH) return false;

  switch (hw_ch) {
    case HW_DEF_TIM7:
      TIM7->PSC = psc;
      break;
    default:
      return false;
  }
  return true;
}

bool pwmChange(uint8_t ch, uint32_t ccr)
{
  bool ret=false;
  uint8_t idx =0;
  if(!(TIM_IS_TIMER(ch)))
  {
    idx = TIM_GET_INDEX(ch);
    if (idx < PWM_MAX_CH)
    {
      ret=true;
    }
  }
  if(ret==true)
  {
    TIM_Base_tbl* p_tim = &pwm_tbl[idx].tim;
    __HAL_TIM_SET_COMPARE(p_tim->htim, p_tim->channel, ccr);
  }
  return ret;
}

bool pwmStart(uint8_t ch)
{
  if (TIM_IS_TIMER(ch)) return false;

  uint8_t idx = TIM_GET_INDEX(ch);
  if (idx >= PWM_MAX_CH) return false;

  TIM_Base_tbl* p_tim = &pwm_tbl[idx].tim;
  return (HAL_TIM_PWM_Start(p_tim->htim, p_tim->channel) == HAL_OK);
}

bool pwmStop(uint8_t ch)
{
  if (TIM_IS_TIMER(ch)) return false;

  uint8_t idx = TIM_GET_INDEX(ch);
  if (idx >= PWM_MAX_CH) return false;

  TIM_Base_tbl* p_tim = &pwm_tbl[idx].tim;
  return (HAL_TIM_PWM_Stop(p_tim->htim, p_tim->channel) == HAL_OK);
}


bool Is1msFlag(uint8_t ch)
{
  if (!TIM_IS_TIMER(ch)) return false;

  uint8_t idx = TIM_GET_INDEX(ch);
  if (idx >= TIMER_MAX_CH) return false;

  return timer_tbl[idx].timerFlag.Flag_1ms;
}

bool Is20msFlag(uint8_t ch)
{
  if (!TIM_IS_TIMER(ch)) return false;

  uint8_t idx = TIM_GET_INDEX(ch);
  if (idx >= TIMER_MAX_CH) return false;

  return timer_tbl[idx].timerFlag.Flag_20ms;
}


bool Is100msFlag(uint8_t ch)
{
  if (!TIM_IS_TIMER(ch)) return false;

  uint8_t idx = TIM_GET_INDEX(ch);
  if (idx >= TIMER_MAX_CH) return false;

  return timer_tbl[idx].timerFlag.Flag_100ms;
}


bool Is1000msFlag(uint8_t ch)
{
  if (!TIM_IS_TIMER(ch)) return false;

  uint8_t idx = TIM_GET_INDEX(ch);
  if (idx >= TIMER_MAX_CH) return false;

  return timer_tbl[idx].timerFlag.Flag_1000ms;
}


bool clear1msFlag(uint8_t ch)
{
  if (!TIM_IS_TIMER(ch)) return false;

  uint8_t idx = TIM_GET_INDEX(ch);
  if (idx >= TIMER_MAX_CH) return false;

  timer_tbl[idx].timerFlag.Flag_1ms = false;
  return true;
}

bool clear20msFlag(uint8_t ch)
{
  if (!TIM_IS_TIMER(ch)) return false;

  uint8_t idx = TIM_GET_INDEX(ch);
  if (idx >= TIMER_MAX_CH) return false;

  timer_tbl[idx].timerFlag.Flag_20ms = false;
  return true;
}

bool clear100msFlag(uint8_t ch)
{
  if (!TIM_IS_TIMER(ch)) return false;

  uint8_t idx = TIM_GET_INDEX(ch);
  if (idx >= TIMER_MAX_CH) return false;

  timer_tbl[idx].timerFlag.Flag_100ms = false;
  return true;
}

bool clear1000msFlag(uint8_t ch)
{
  if (!TIM_IS_TIMER(ch)) return false;

  uint8_t idx = TIM_GET_INDEX(ch);
  if (idx >= TIMER_MAX_CH) return false;

  timer_tbl[idx].timerFlag.Flag_1000ms = false;
  return true;
}
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* Prevent unused argument(s) compilation warning */
  if(htim->Instance == timer_tbl[HW_DEF_TIM7].tim. htim->Instance)
  {
    timer_tbl[HW_DEF_TIM7].timerFlag.Flag_1ms = true;

    timer_tbl[HW_DEF_TIM7].timerFlag.counter_20ms = (timer_tbl[HW_DEF_TIM7].timerFlag.counter_20ms+1)%21;
    if(timer_tbl[HW_DEF_TIM7].timerFlag.counter_20ms == 20)
    {
      timer_tbl[HW_DEF_TIM7].timerFlag.Flag_20ms = true;
    }

    timer_tbl[HW_DEF_TIM7].timerFlag.counter_100ms = (timer_tbl[HW_DEF_TIM7].timerFlag.counter_100ms+1)%101;
    if(timer_tbl[HW_DEF_TIM7].timerFlag.counter_100ms == 100)
    {
      timer_tbl[HW_DEF_TIM7].timerFlag.Flag_100ms = true;
    }

    timer_tbl[HW_DEF_TIM7].timerFlag.counter_1000ms = (timer_tbl[HW_DEF_TIM7].timerFlag.counter_1000ms+1)%1001;
    if(timer_tbl[HW_DEF_TIM7].timerFlag.counter_1000ms == 1000)
    {
      timer_tbl[HW_DEF_TIM7].timerFlag.Flag_1000ms = true;
    }

  }

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_TIM_PeriodElapsedCallback could be implemented in the user file
   */
}


void HAL_TIM_Base_MspInit(TIM_HandleTypeDef* tim_baseHandle)
{

  if(tim_baseHandle->Instance==TIM3)
  {
    /* USER CODE BEGIN TIM3_MspInit 0 */

    /* USER CODE END TIM3_MspInit 0 */
    /* TIM3 clock enable */
    __HAL_RCC_TIM3_CLK_ENABLE();
    /* USER CODE BEGIN TIM3_MspInit 1 */

    /* USER CODE END TIM3_MspInit 1 */
  }
  else if(tim_baseHandle->Instance==TIM5)
  {
    /* USER CODE BEGIN TIM5_MspInit 0 */

    /* USER CODE END TIM5_MspInit 0 */
    /* TIM5 clock enable */
    __HAL_RCC_TIM5_CLK_ENABLE();
    /* USER CODE BEGIN TIM5_MspInit 1 */

    /* USER CODE END TIM5_MspInit 1 */
  }
  else if(tim_baseHandle->Instance==TIM7)
  {
    /* USER CODE BEGIN TIM7_MspInit 0 */

    /* USER CODE END TIM7_MspInit 0 */
    /* TIM7 clock enable */
    __HAL_RCC_TIM7_CLK_ENABLE();

    /* TIM7 interrupt Init */
    HAL_NVIC_SetPriority(TIM7_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(TIM7_IRQn);
    /* USER CODE BEGIN TIM7_MspInit 1 */

    /* USER CODE END TIM7_MspInit 1 */
  }
}
void HAL_TIM_MspPostInit(TIM_HandleTypeDef* timHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(timHandle->Instance==TIM3)
  {
    /* USER CODE BEGIN TIM3_MspPostInit 0 */

    /* USER CODE END TIM3_MspPostInit 0 */
    __HAL_RCC_GPIOB_CLK_ENABLE();
    /**TIM3 GPIO Configuration
    PB1     ------> TIM3_CH4
     */
    GPIO_InitStruct.Pin = GPIO_PIN_1;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF2_TIM3;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* USER CODE BEGIN TIM3_MspPostInit 1 */

    /* USER CODE END TIM3_MspPostInit 1 */
  }
  else if(timHandle->Instance==TIM5)
  {
    /* USER CODE BEGIN TIM5_MspPostInit 0 */

    /* USER CODE END TIM5_MspPostInit 0 */

    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**TIM5 GPIO Configuration
    PA0-WKUP     ------> TIM5_CH1
    PA1     ------> TIM5_CH2
    PA2     ------> TIM5_CH3
    PA3     ------> TIM5_CH4
     */
    GPIO_InitStruct.Pin = ESC_1_Pin|ESC_2_Pin|ESC_3_Pin|ESC_4_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF2_TIM5;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* USER CODE BEGIN TIM5_MspPostInit 1 */

    /* USER CODE END TIM5_MspPostInit 1 */
  }

}

void HAL_TIM_Base_MspDeInit(TIM_HandleTypeDef* tim_baseHandle)
{

  if(tim_baseHandle->Instance==TIM3)
  {
    /* USER CODE BEGIN TIM3_MspDeInit 0 */

    /* USER CODE END TIM3_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_TIM3_CLK_DISABLE();
    /* USER CODE BEGIN TIM3_MspDeInit 1 */

    /* USER CODE END TIM3_MspDeInit 1 */
  }
  else if(tim_baseHandle->Instance==TIM5)
  {
    /* USER CODE BEGIN TIM5_MspDeInit 0 */

    /* USER CODE END TIM5_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_TIM5_CLK_DISABLE();
    /* USER CODE BEGIN TIM5_MspDeInit 1 */

    /* USER CODE END TIM5_MspDeInit 1 */
  }
  else if(tim_baseHandle->Instance==TIM7)
  {
    /* USER CODE BEGIN TIM7_MspDeInit 0 */

    /* USER CODE END TIM7_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_TIM7_CLK_DISABLE();

    /* TIM7 interrupt Deinit */
    HAL_NVIC_DisableIRQ(TIM7_IRQn);
    /* USER CODE BEGIN TIM7_MspDeInit 1 */

    /* USER CODE END TIM7_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

#endif
