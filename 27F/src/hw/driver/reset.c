/*
 * reset.c
 *
 *  Created on: 2020. 12. 9.
 *      Author: baram
 */


#include "reset.h"
#include "rtc.h"
#include "cli.h"

#ifdef _USE_HW_RESET

static uint32_t reset_count = 0;

static void cliReset(cli_args_t* args);

bool resetInit(void)
{
	bool ret = true;
#ifdef _USE_HW_CLI
	cliAdd("Reset", cliReset);

#endif
#if 1
	// 만약 Reset 핀이 눌렸다면
	//
	if (RCC->CSR & (1<<26))
	{
		rtcBackupRegWrite(1, rtcBackupRegRead(1) + 1);
		delay(500);
		reset_count = rtcBackupRegRead(1);
	}

	rtcBackupRegWrite(1, 0);
#endif

	return ret;
}

uint32_t resetGetCount(void)
{
	return reset_count;
}

void cliReset(cli_args_t* args)
{
	bool ret = false;
	if(args->argc==1 && args->isStr(0, "show") == true)
	{
		while(cliKeepLoop())
		{
			cliPrintf("%d", resetGetCount());
			cliPrintf("\n");
			delay(100);
		}
		ret=true;
	}
	if(ret != true)
	{
		cliPrintf("reset show\n");
	}
}

#endif
