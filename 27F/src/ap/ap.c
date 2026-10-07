/*
 * ap.c
 *
 *  Created on: Dec 6, 2020
 *      Author: baram
 */


#include "ap.h"
#include "sd_functions.h"
#include <stdio.h>
#include <string.h>

void cliBoot(cli_args_t *args);

static void sdCardTest(void)
{
	static const char test_text[] = "SPI2 DMA SD card test\r\n";
	char readback[64];
	UINT bytes_read = 0;
	int result;

	printf("\r\n[SD] Test start\r\n");
	result = sd_mount();
	if (result != FR_OK)
	{
		printf("[SD] Mount failed: %d\r\n", result);
		return;
	}

	result = sd_write_file("SD_TEST.TXT", test_text);
	if (result != FR_OK)
	{
		printf("[SD] Write failed: %d\r\n", result);
	}
	else if (sd_read_file("SD_TEST.TXT", readback, sizeof(readback), &bytes_read) != FR_OK)
	{
		printf("[SD] Read failed\r\n");
	}
	else if (strcmp(readback, test_text) != 0)
	{
		printf("[SD] Verify failed (%u bytes): %s\r\n", bytes_read, readback);
	}
	else
	{
		printf("[SD] PASS: wrote and read %u bytes\r\n", bytes_read);
	}

	(void)sd_unmount();
}

//cmd_t cmd;

void apInit(void)
{
	cliOpen(_DEF_UART1, 115200);
	uartOpen(_DEF_UART2, 115200);
	uartOpen(_DEF_UART3, 115200);
	uartOpen(_DEF_UART4, 115200);
//	istimOpen(DEF_TIM6);
	spiOpen(_DEF_SPI2);
	adcOpen(DEF_ADC1);
}

uint32_t adc_val;

void apMain(void)
{
	/* Run once at boot; keep the existing UART/ADC loop unchanged. */
	sdCardTest();

	while(1)
	{
//		if(Is20msFlag(DEF_TIM6) == true)
//		{
		  adc_val = adcReceive(DEF_ADC1);
//		}
		  cliMain();
	}


}
