/*
 * ap.c
 *
 *  Created on: Dec 6, 2020
 *      Author: baram
 */


#include "ap.h"

void cliBoot(cli_args_t *args);

//cmd_t cmd;

void apInit(void)
{
	uartOpen(_DEF_UART1, 115200);
	uartOpen(_DEF_UART2, 115200);
	uartOpen(_DEF_UART3, 115200);
	uartOpen(_DEF_UART4, 115200);
	adcOpen(DEF_ADC1);
}

uint32_t adc_val;

void apMain(void)
{
	while(1)
	{
		adc_val = adcReceive(DEF_ADC1);
		if(uartAvailable(_DEF_UART1) >0)
		{
			uartPrintf(_DEF_UART1, "%x \n", uartRead(_DEF_UART1));
		}
		if(uartAvailable(_DEF_UART2) >0)
		{
			uartPrintf(_DEF_UART2, "%x \n",uartRead(_DEF_UART2));
		}
		if(uartAvailable(_DEF_UART3) >0)
		{
			uartPrintf(_DEF_UART3, "%x \n",uartRead(_DEF_UART3));
		}
		if(uartAvailable(_DEF_UART4) >0)
		{
			uartPrintf(_DEF_UART4, "%x \n",uartRead(_DEF_UART4));
		}
	}
}
