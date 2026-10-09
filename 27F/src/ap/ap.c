/*
 * ap.c
 *
 *  Created on: Dec 6, 2020
 *      Author: baram
 */


#include "ap.h"
#include "sd_functions.h"
#include <stdio.h>

void cliBoot(cli_args_t *args);

#define SD_LOG_BUFFER_SIZE  4096U
#define SD_LOG_LINE_SIZE     128U

static char sd_log_filename[13];
static FIL sd_log_file;
static char sd_log_buffer[SD_LOG_BUFFER_SIZE + 1U];
static UINT sd_log_used = 0;
static bool sd_log_active = false;

static bool sdLogFlush(void)
{
	UINT bytes_written = 0;
	FRESULT result;

	if (!sd_log_active || sd_log_used == 0)
	{
		return sd_log_active;
	}

	result = f_write(&sd_log_file, sd_log_buffer, sd_log_used, &bytes_written);
	if (result == FR_OK && bytes_written == sd_log_used)
	{
		result = f_sync(&sd_log_file);
	}
	if (result != FR_OK || bytes_written != sd_log_used)
	{
		printf("[LOG] SD write failed: %d (%u/%u bytes)\r\n",
				result, bytes_written, sd_log_used);
		(void)f_close(&sd_log_file);
		sd_log_active = false;
		return false;
	}

	sd_log_used = 0;
	return true;
}

static void sdLogStart(void)
{
	static const char header[] = "ax_mg,ay_mg,az_mg,mx_centi_uT,my_centi_uT,mz_centi_uT\r\n";
	FILINFO file_info;
	UINT bytes_written = 0;
	FRESULT result = FR_NO_FILE;
	bool file_created = false;

	if (sd_mount() != FR_OK)
	{
		printf("[LOG] SD mount failed; logging disabled\r\n");
		return;
	}

	/* Find the first unused 8.3 filename and create it without overwrite. */
	for (uint16_t index = 1; index <= 9999; index++)
	{
		(void)snprintf(sd_log_filename, sizeof(sd_log_filename), "IMU%04u.CSV", (unsigned)index);
		result = f_stat(sd_log_filename, &file_info);
		if (result == FR_OK)
		{
			continue;
		}
		if (result != FR_NO_FILE)
		{
			break;
		}

		result = f_open(&sd_log_file, sd_log_filename, FA_CREATE_NEW | FA_WRITE);
		if (result == FR_OK)
		{
			file_created = true;
			break;
		}
		if (result != FR_EXIST)
		{
			break;
		}
	}
	if (!file_created)
	{
		printf("[LOG] Could not create a new log file: %d\r\n", result);
		(void)sd_unmount();
		return;
	}
	result = f_write(&sd_log_file, header, sizeof(header) - 1U, &bytes_written);
	if (result == FR_OK && bytes_written == sizeof(header) - 1U)
	{
		result = f_sync(&sd_log_file);
	}
	if (result != FR_OK || bytes_written != sizeof(header) - 1U)
	{
		printf("[LOG] Header write failed: %d\r\n", result);
		(void)f_close(&sd_log_file);
		(void)sd_unmount();
		return;
	}

	sd_log_used = 0;
	sd_log_active = true;
}

static void sdLogSample(const axises *accel, const axises *mag)
{
	char line[SD_LOG_LINE_SIZE];
	int length;
	int32_t ax_mg = (int32_t)(accel->x * 1000.0f);
	int32_t ay_mg = (int32_t)(accel->y * 1000.0f);
	int32_t az_mg = (int32_t)(accel->z * 1000.0f);
	int32_t mx_centi_uT = (int32_t)(mag->x * 100.0f);
	int32_t my_centi_uT = (int32_t)(mag->y * 100.0f);
	int32_t mz_centi_uT = (int32_t)(mag->z * 100.0f);

	if (!sd_log_active)
	{
		return;
	}

	length = snprintf(line, sizeof(line), "%ld,%ld,%ld,%ld,%ld,%ld\r\n",
			(long)ax_mg, (long)ay_mg, (long)az_mg,
			(long)mx_centi_uT, (long)my_centi_uT, (long)mz_centi_uT);
	if (length <= 0 || (size_t)length >= sizeof(line))
	{
		return;
	}

	if (sd_log_used + (UINT)length > SD_LOG_BUFFER_SIZE && !sdLogFlush())
	{
		return;
	}
	memcpy(&sd_log_buffer[sd_log_used], line, (size_t)length);
	sd_log_used += (UINT)length;

	if (sd_log_used >= SD_LOG_BUFFER_SIZE)
	{
		(void)sdLogFlush();
	}
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
	LinearSOpen(DEF_BL);

}
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
	sdLogStart();
	while(1)
	{
		//		if(Is20msFlag(DEF_TIM6) == true)
		//		{
		if(gpioPinRead(2) && Flag_1ms)
		{
			icm20948_accel_read_g(&my_accel);
			ak09916_mag_read_uT(&my_mag);
			sdLogSample(&my_accel, &my_mag);
			//    gx_rad = my_gyro.x * (3.14159265f / 180.0f);
			//    gy_rad = my_gyro.y * (3.14159265f / 180.0f);
			//    gz_rad = my_gyro.z * (3.14159265f / 180.0f);
			ax_g = my_accel.x;
			ay_g = my_accel.y;
			az_g = my_accel.z;
			Flag_1ms = false;
			adc_val = adcReceive(DEF_ADC1);
		}
		cliMain();
	}


}
