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
	cliOpen(_DEF_UART1, 115200);
	//uartOpen(_DEF_UART2, 115200);
}

void apMain(void)
{
	while(1)
	{
	 cliMain();
	}
}
