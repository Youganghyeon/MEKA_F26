/*
 * tim.c
 */

#include "tim.h"

#ifdef _USE_HW_TIMER

TIM_HandleTypeDef htim6;

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
  TIM_HandleTypeDef* htim;
  bool               isInit;
  bool               isOpen;
  TIMER_Flag_tbl     flag;
} TIMER_tbl;

static TIMER_tbl timer_tbl[TIM_MAX_CH] = {
  { &htim6, false, false, {false, false, false, false, 0, 0, 0} },
};

static bool timerOpen(uint8_t ch);


void timInit(void)
{
  for (int i = 0; i < TIM_MAX_CH; i++) {
    timer_tbl[i].isInit = true;
    timer_tbl[i].isOpen = false;
  }
}

bool istimOpen(uint8_t ch)
{
  if (ch >= TIM_MAX_CH) return false;
  return timer_tbl[ch].isOpen;
}

bool timOpen(uint8_t ch)
{
  if (ch >= TIM_MAX_CH) return false;
  return timerOpen(ch);
}

static bool timerOpen(uint8_t ch)
{
  switch (ch) {
    case HW_DEF_TIM6:
      MX_TIM6_Init();                         // PSC=143, ARR=999 -> 1kHz
      timer_tbl[ch].isOpen = true;
      return (HAL_TIM_Base_Start_IT(&htim6) == HAL_OK);

    default:
      return false;
  }
}

bool timDeinit(uint8_t ch)
{
  if (ch >= TIM_MAX_CH) return false;
  timer_tbl[ch].isOpen = false;
  return (HAL_TIM_Base_Stop_IT(timer_tbl[ch].htim) == HAL_OK);
}

bool timPsc(uint8_t ch, uint32_t psc)
{
  if (ch >= TIM_MAX_CH) return false;
  __HAL_TIM_SET_PRESCALER(timer_tbl[ch].htim, psc);
  return true;
}


/* ---- Flag helpers ---- */
#define DEF_FLAG_FUNCS(NAME, FIELD)                          \
bool Is##NAME##Flag(uint8_t ch)                              \
{                                                            \
  if (ch >= TIM_MAX_CH) return false;                        \
  return timer_tbl[ch].flag.FIELD;                           \
}                                                            \
bool clear##NAME##Flag(uint8_t ch)                           \
{                                                            \
  if (ch >= TIM_MAX_CH) return false;                        \
  timer_tbl[ch].flag.FIELD = false;                          \
  return true;                                               \
}

DEF_FLAG_FUNCS(1ms,    Flag_1ms)
DEF_FLAG_FUNCS(20ms,   Flag_20ms)
DEF_FLAG_FUNCS(100ms,  Flag_100ms)
DEF_FLAG_FUNCS(1000ms, Flag_1000ms)


void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  if (htim->Instance == TIM6)
  {
    TIMER_Flag_tbl* f = &timer_tbl[HW_DEF_TIM6].flag;

    f->Flag_1ms = true;

    if (++f->counter_20ms   >= 20)   { f->counter_20ms   = 0; f->Flag_20ms   = true; }
    if (++f->counter_100ms  >= 100)  { f->counter_100ms  = 0; f->Flag_100ms  = true; }
    if (++f->counter_1000ms >= 1000) { f->counter_1000ms = 0; f->Flag_1000ms = true; }
  }
}


/* ---- CubeMX 생성부 ---- */
void MX_TIM6_Init(void)
{
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  htim6.Instance               = TIM6;
  htim6.Init.Prescaler         = 143;
  htim6.Init.CounterMode       = TIM_COUNTERMODE_UP;
  htim6.Init.Period            = 999;
  htim6.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim6) != HAL_OK) { Error_Handler(); }

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode     = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim6, &sMasterConfig) != HAL_OK) { Error_Handler(); }
}

void HAL_TIM_Base_MspInit(TIM_HandleTypeDef* tim_baseHandle)
{
  if (tim_baseHandle->Instance == TIM6)
  {
    __HAL_RCC_TIM6_CLK_ENABLE();
    HAL_NVIC_SetPriority(TIM6_DAC_IRQn, 0, 0);   // 칩에 따라 TIM6_IRQn일 수 있음
    HAL_NVIC_EnableIRQ(TIM6_DAC_IRQn);
  }
}

void HAL_TIM_Base_MspDeInit(TIM_HandleTypeDef* tim_baseHandle)
{
  if (tim_baseHandle->Instance == TIM6)
  {
    __HAL_RCC_TIM6_CLK_DISABLE();
    HAL_NVIC_DisableIRQ(TIM6_DAC_IRQn);
  }
}

#endif
