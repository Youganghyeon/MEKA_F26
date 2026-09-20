/*
 * def.h
 *
 *  Created on: Dec 6, 2020
 *      Author: baram
 */

#ifndef SRC_COMMON_DEF_H_
#define SRC_COMMON_DEF_H_


#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#define _DEF_LED1           0
#define _DEF_LED2           1
#define _DEF_LED3           2
#define _DEF_LED4           3



#define _DEF_BUTTON1        0
#define _DEF_BUTTON2        1
#define _DEF_BUTTON3        2
#define _DEF_BUTTON4        3


#define _DEF_INPUT             0
#define _DEF_INPUT_PULL_UP     1
#define _DEF_INPUT_PULL_DOWN   2
#define _DEF_OUTPUT            3
#define _DEF_OUTPUT_PULL_UP    4
#define _DEF_OUTPUT_PULL_DOWN  5


#define _DEF_LOW      0
#define _DEF_HIGH     1



#define Button_Pin GPIO_PIN_13
#define Button_GPIO_Port GPIOC
#define FL_ADC_Pin GPIO_PIN_0
#define FL_ADC_GPIO_Port GPIOA
#define FR_ADC_Pin GPIO_PIN_1
#define FR_ADC_GPIO_Port GPIOA
#define BL_ADC_Pin GPIO_PIN_2
#define BL_ADC_GPIO_Port GPIOA
#define BR_ADC_Pin GPIO_PIN_3
#define BR_ADC_GPIO_Port GPIOA
#define IMU_SCK_Pin GPIO_PIN_5
#define IMU_SCK_GPIO_Port GPIOA
#define IMU_MISO_Pin GPIO_PIN_6
#define IMU_MISO_GPIO_Port GPIOA
#define IMU_MOSI_Pin GPIO_PIN_7
#define IMU_MOSI_GPIO_Port GPIOA
#define PC_TX_Pin GPIO_PIN_4
#define PC_TX_GPIO_Port GPIOC
#define IMU_CS_Pin GPIO_PIN_0
#define IMU_CS_GPIO_Port GPIOB
#define GPS_TX_Pin GPIO_PIN_10
#define GPS_TX_GPIO_Port GPIOB
#define GPS_RX_Pin GPIO_PIN_11
#define GPS_RX_GPIO_Port GPIOB
#define SD_CS_Pin GPIO_PIN_12
#define SD_CS_GPIO_Port GPIOB
#define SD_SCK_Pin GPIO_PIN_13
#define SD_SCK_GPIO_Port GPIOB
#define SD_MISO_Pin GPIO_PIN_14
#define SD_MISO_GPIO_Port GPIOB
#define SD_MOSI_Pin GPIO_PIN_15
#define SD_MOSI_GPIO_Port GPIOB
#define Led_Pin GPIO_PIN_6
#define Led_GPIO_Port GPIOC
#define PC_RX_Pin GPIO_PIN_10
#define PC_RX_GPIO_Port GPIOA
#define TEL_RX_Pin GPIO_PIN_15
#define TEL_RX_GPIO_Port GPIOA
#define TEL_TX_Pin GPIO_PIN_3
#define TEL_TX_GPIO_Port GPIOB



#define constrain(amt,low,high) ((amt)<(low)?(low):((amt)>(high)?(high):(amt)))

#ifndef max
#define max(a,b) (((a) > (b)) ? (a) : (b))
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif


#ifndef map
#define map(value, in_min, in_max, out_min, out_max) ((value - in_min) * (out_max - out_min) / (in_max - in_min) + out_min)
#endif
typedef struct
{
  uint8_t version[32];
  uint8_t name[32];
} firm_version_t;

typedef struct
{
  uint8_t version[32];
  uint8_t name[32];
} boot_version_t;
#define MAGIC_NUMBER      0x5555AAAA

#endif /* SRC_COMMON_DEF_H_ */
