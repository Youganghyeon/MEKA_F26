/*
 * gpio.h
 *
 *  Created on: 2026. 3. 12.
 *      Author: yougang
 */

#ifndef SRC_COMMON_HW_INCLUDE_GPIO_H_
#define SRC_COMMON_HW_INCLUDE_GPIO_H_

#include "hw_def.h"

#ifdef _USE_HW_GPIO

#define GPIO_MAX_CH      HW_GPIO_MAX_CH

bool gpioInit(void);
bool gpioPinMode(uint8_t ch, uint8_t mode);
bool gpioPinRead(uint8_t ch);
bool gpioPinWrite(uint8_t ch, bool value);
bool gpioPinToggle(uint8_t ch);


#endif

#endif /* SRC_COMMON_HW_INCLUDE_GPIO_H_ */
