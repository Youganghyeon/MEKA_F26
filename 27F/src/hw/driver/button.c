/*
 * button.c
 *
 *  Created on: 2026. 3. 12.
 *      Author: yougang
 */
#include "button.h"
#include "cli.h"

typedef struct{
  GPIO_TypeDef* port;
  uint32_t pin;
  GPIO_PinState on_state;
} Button_tbl_t;

Button_tbl_t button_tbl[BUTTON_MAX_CH]=
    {
        {GPIOC, GPIO_PIN_13, GPIO_PIN_SET},
    };

#ifdef _USE_HW_CLI
static void cliButton(cli_args_t *args);
#endif

bool buttonInit(void)
{
  bool ret=true;
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  __HAL_RCC_GPIOC_CLK_ENABLE();

  GPIO_InitStruct.Pin = GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  for(int i=0; i<BUTTON_MAX_CH; i++)
  {
    GPIO_InitStruct.Pin = button_tbl[i].pin;
    HAL_GPIO_Init(button_tbl[i].port, &GPIO_InitStruct);
  }

#ifdef _USE_HW_CLI
  cliAdd("button", cliButton);
#endif

  return ret;
}


bool buttonGetPressed(uint8_t ch)
{
 bool ret=false;

 if(BUTTON_MAX_CH <= ch)
 {
   return false;
 }
 if(button_tbl[ch].on_state == HAL_GPIO_ReadPin(button_tbl[ch].port, button_tbl[ch].pin))
 {
   ret=true;
 }

 return ret;
}


#ifdef _USE_HW_CLI

void cliButton(cli_args_t *args)
{
  bool ret=false;

  if(args->argc==1 && args->isStr(0, "show") == true)
  {
    while(cliKeepLoop())
    {
      for(int i=0; i<BUTTON_MAX_CH; i++)
      {
        cliPrintf("%d", buttonGetPressed(i));
      }
      cliPrintf("\n");
      delay(100);
    }
    ret=true;
  }
  if(ret != true)
  {
    cliPrintf("button show\n");
  }
}
#endif
