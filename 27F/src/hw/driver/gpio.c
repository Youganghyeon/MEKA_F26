/*
 * gpio.c
 *
 *  Created on: 2026. 3. 12.
 *      Author: yougang
 */

#include "gpio.h"

#ifdef _USE_HW_GPIO
#include "cli.h"


typedef struct{
	GPIO_TypeDef* port;
	uint32_t pin;
	uint8_t  mode;
	GPIO_PinState on_state;
	GPIO_PinState off_state;
	bool init_value;
} Gpio_tbl_t;

Gpio_tbl_t gpio_tbl[GPIO_MAX_CH]=
{
		//   {GPIOB, GPIO_PIN_9, _DEF_INPUT_PULL_UP, GPIO_PIN_SET, GPIO_PIN_RESET, 1},           // SD CARD
		//  {GPIOA, GPIO_PIN_2, _DEF_OUTPUT, GPIO_PIN_SET, GPIO_PIN_RESET, _DEF_LOW  },         // BKP HIGH : Light,      LOW: No Light
		//  {GPIOA, GPIO_PIN_3, _DEF_OUTPUT, GPIO_PIN_SET, GPIO_PIN_RESET, _DEF_HIGH },         // CS  HIGH : NO Select,  LOW : Select
		//  {GPIOA, GPIO_PIN_4, _DEF_OUTPUT, GPIO_PIN_SET, GPIO_PIN_RESET, _DEF_HIGH },         // DC  HIGH : DATA     ,  LOW : Command
		//  {GPIOB, GPIO_PIN_1, _DEF_OUTPUT, GPIO_PIN_SET, GPIO_PIN_RESET, _DEF_LOW },          // RST LOW:ACTIVE
};

#ifdef _USE_HW_CLI
static void cliGpio(cli_args_t *args);
#endif

bool gpioInit(void)
{
	bool ret=true;
	for(int i=0; i<GPIO_MAX_CH; i++)
	{
		gpioPinMode(i,gpio_tbl[i].mode);
		gpioPinWrite(i, gpio_tbl[i].init_value);
	}

#ifdef _USE_HW_CLI
	cliAdd("gpio", cliGpio);
#endif

	return ret;
}

bool gpioPinMode(uint8_t ch, uint8_t mode)
{
	bool ret=true;
	if(ch>=GPIO_MAX_CH) return false;
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOF_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	switch(mode)
	{
	case _DEF_INPUT:
		GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		break;
	case _DEF_INPUT_PULL_UP:
		GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
		GPIO_InitStruct.Pull = GPIO_PULLUP;
		break;
	case _DEF_INPUT_PULL_DOWN:
		GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
		GPIO_InitStruct.Pull = GPIO_PULLDOWN;
		break;
	case _DEF_OUTPUT:
		GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		break;
	case _DEF_OUTPUT_PULL_UP:
		GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
		GPIO_InitStruct.Pull = GPIO_PULLUP;
		break;
	case _DEF_OUTPUT_PULL_DOWN:
		GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
		GPIO_InitStruct.Pull = GPIO_PULLDOWN;
		break;
	}
	GPIO_InitStruct.Pin = gpio_tbl[ch].pin;
	HAL_GPIO_Init(gpio_tbl[ch].port, &GPIO_InitStruct);
	return ret;
}


bool gpioPinRead(uint8_t ch)
{
	bool ret=false;
	if(ch>=GPIO_MAX_CH) return false;
	if(HAL_GPIO_ReadPin(gpio_tbl[ch].port, gpio_tbl[ch].pin)==gpio_tbl[ch].on_state)
	{
		ret=true;
	}
	return ret;
}

bool gpioPinWrite(uint8_t ch, bool value)
{
	bool ret=true;
	if(ch>=GPIO_MAX_CH) return false;
	if(value == true)
	{
		HAL_GPIO_WritePin(gpio_tbl[ch].port, gpio_tbl[ch].pin, gpio_tbl[ch].on_state);
	}
	else
	{
		HAL_GPIO_WritePin(gpio_tbl[ch].port, gpio_tbl[ch].pin, gpio_tbl[ch].off_state);

	}
	return ret;
}

bool gpioPinToggle(uint8_t ch)
{
	bool ret=true;
	if(ch>=GPIO_MAX_CH) return false;
	HAL_GPIO_TogglePin(gpio_tbl[ch].port, gpio_tbl[ch].pin);
	return ret;
}

#ifdef _USE_HW_CLI
static void cliGpio(cli_args_t *args)
{
	bool ret=false;
	if(args->argc == 1 && args->isStr(0,"show"))
	{
		while(cliKeepLoop())
		{
			for(int i=0; i<GPIO_MAX_CH; i++)
			{
				cliPrintf("%d \n", i);
			}
			cliPrintf("\n");
			delay(100);
		}
		ret=true;
	}
	if(args->argc == 2 && args->isStr(0,"read"))
	{
		uint8_t ch;
		ch = args->getData(1);

		while(cliKeepLoop()>0)
		{
			cliPrintf("%d \n", gpioPinRead(ch));
			delay(100);
		}
		ret=true;
	}
	else if(args->argc == 3 && args->isStr(0,"write"))
	{
		uint8_t ch;
		uint8_t value;

		ch= args->getData(1);
		value=args->getData(2);

		gpioPinWrite(ch, value);
		cliPrintf("gpioPinWrite[%d] : %d \n", ch, value);
		delay(100);

		ret= true;
	}
	if(ret != true)
	{
		cliPrintf("gpio show \n");
		cliPrintf("gpio read ch[0~%d] \n", GPIO_MAX_CH-1);
		cliPrintf("gpio write ch[0~%d] 0:1\n", GPIO_MAX_CH-1);
	}
}
#endif
#endif
