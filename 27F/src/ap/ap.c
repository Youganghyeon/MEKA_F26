/*
 * ap.c
 *
 *  Created on: Dec 6, 2020
 *      Author: baram
 */


#include "ap.h"
#include "sd_functions.h"
#include "sd_benchmark.h"

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
	spiOpen(_DEF_SPI1);
	spiOpen(_DEF_SPI2);
	icm20948_init();
	ak09916_init();
	adcOpen(DEF_ADC1);

}
axises my_gyro;
axises my_accel;
axises my_mag;

volatile uint8_t Flag = 0;

float sensor_us=0.0;
float compute = 0.0;

float gx_rad, gy_rad, gz_rad;
float ax_g, ay_g, az_g;          // 라디안이 아니라 g 단위 그대로 사용
float mx_uT, my_uT, mz_uT;       // 라디안이 아니라 uT 단위 그대로 사용

uint32_t adc_val;
bool     Flag_1ms= false;

void apMain(void)
{
	/* Run once at boot; keep the existing UART/ADC loop unchanged. */
	sdCardTest();
	sd_benchmark();
	while(1)
	{
		//		if(Is20msFlag(DEF_TIM6) == true)
		//		{
		if(gpioPinRead(2) && Flag_1ms)
		{
			icm20948_gyro_read_dps(&my_gyro);
			icm20948_accel_read_g(&my_accel);
			ak09916_mag_read_uT(&my_mag);
			//    gx_rad = my_gyro.x * (3.14159265f / 180.0f);
			//    gy_rad = my_gyro.y * (3.14159265f / 180.0f);
			//    gz_rad = my_gyro.z * (3.14159265f / 180.0f);
			ax_g = my_accel.x;
			ay_g = my_accel.y;
			az_g = my_accel.z;
			Flag_1ms = false;
		}
		adc_val = adcReceive(DEF_ADC1);
		//		}
		cliMain();
	}


}
