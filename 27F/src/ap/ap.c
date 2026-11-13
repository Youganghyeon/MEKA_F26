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
#define ADC_CHANNEL_COUNT      4U
#define SAMPLE_INTERVAL_MS   10U

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
	static const char header[] = "Time_ms,Type,ax_g,ay_g,az_g,FL_adc,FR_adc,BL_adc,BR_adc\r\n";
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

static void floatToParts3(float value, const char **sign, uint32_t *whole, uint32_t *fraction)
{
	int32_t scaled = (int32_t)(value * 1000.0f + (value >= 0.0f ? 0.5f : -0.5f));
	uint32_t magnitude = (scaled < 0) ? (uint32_t)(-scaled) : (uint32_t)scaled;

	*sign = (scaled < 0) ? "-" : "";
	*whole = magnitude / 1000U;
	*fraction = magnitude % 1000U;
}

static void sdLogAppendRecord(const char *line, int length)
{
	if (!sd_log_active || length <= 0 || (size_t)length >= SD_LOG_LINE_SIZE)
	{
		return;
	}

	if (sd_log_used + (UINT)length > SD_LOG_BUFFER_SIZE && !sdLogFlush())
	{
		return;
	}
	memcpy(&sd_log_buffer[sd_log_used], line, (size_t)length);
	sd_log_used += (UINT)length;

	/* Flush before the next record would run out of room. */
	if (sd_log_used >= SD_LOG_BUFFER_SIZE - SD_LOG_LINE_SIZE)
	{
		(void)sdLogFlush();
	}
}

static void sdLogLinearSample(const uint32_t adc_values[ADC_CHANNEL_COUNT])
{
	char line[SD_LOG_LINE_SIZE];
	int length;

	if (!sd_log_active)
	{
		return;
	}
	length = snprintf(line, sizeof(line), "%lu,ADC,,,,%lu,%lu,%lu,%lu\r\n",
			(unsigned long)HAL_GetTick(),
			(unsigned long)adc_values[0], (unsigned long)adc_values[1],
			(unsigned long)adc_values[2], (unsigned long)adc_values[3]);
	sdLogAppendRecord(line, length);
}

static void sdLogImuSample(const axises *accel)
{
	char line[SD_LOG_LINE_SIZE];
	const char *ax_sign, *ay_sign, *az_sign;
	uint32_t ax_whole, ax_fraction, ay_whole, ay_fraction, az_whole, az_fraction;
	int length;

	if (!sd_log_active)
	{
		return;
	}
	floatToParts3(accel->x, &ax_sign, &ax_whole, &ax_fraction);
	floatToParts3(accel->y, &ay_sign, &ay_whole, &ay_fraction);
	floatToParts3(accel->z, &az_sign, &az_whole, &az_fraction);

	length = snprintf(line, sizeof(line), "%lu,IMU,%s%lu.%03lu,%s%lu.%03lu,%s%lu.%03lu,,,,\r\n",
			(unsigned long)HAL_GetTick(),
			ax_sign, (unsigned long)ax_whole, (unsigned long)ax_fraction,
			ay_sign, (unsigned long)ay_whole, (unsigned long)ay_fraction,
			az_sign, (unsigned long)az_whole, (unsigned long)az_fraction);
	sdLogAppendRecord(line, length);
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
	if (!adcOpenAllChannels())
	{
		printf("[ADC] Failed to open ADC1 channels PA0-PA3\r\n");
	}

}
axises my_accel;
uint32_t adc_values[ADC_CHANNEL_COUNT];

volatile uint8_t Flag = 0;

float sensor_us=0.0;
float compute = 0.0;

float gx_rad, gy_rad, gz_rad;
float ax_g, ay_g, az_g;          // 라디안이 아니라 g 단위 그대로 사용
float mx_uT, my_uT, mz_uT;       // 라디안이 아니라 uT 단위 그대로 사용

uint32_t previousMillis = 0;
bool     Flag_1ms = false; /* Still referenced by the SysTick handler. */

void apMain(void)
{
	sdLogStart();
	while(1)
	{
		uint32_t currentMillis = HAL_GetTick();
		if ((uint32_t)(currentMillis - previousMillis) >= SAMPLE_INTERVAL_MS)
		{
			previousMillis = currentMillis;
			if (adcReadAllChannels(adc_values))
			{
				sdLogLinearSample(adc_values);
			}
		}

		/* ADC logging is independent; log IMU only while PB1 is high. */
		if (gpioPinRead(2))
		{
			icm20948_accel_read_g(&my_accel);
			sdLogImuSample(&my_accel);
			ax_g = my_accel.x;
			ay_g = my_accel.y;
			az_g = my_accel.z;
		}
		cliMain();
	}


}
