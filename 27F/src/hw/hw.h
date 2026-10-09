/*
 * hw.h
 *
 *  Created on: Dec 6, 2020
 *      Author: baram
 */

#ifndef SRC_HW_HW_H_
#define SRC_HW_HW_H_


#include "hw_def.h"


#include "led.h"
#include "usb.h"
#include "uart.h"
#include "rtc.h"
#include "reset.h"
#include "flash.h"
#include "cli.h"
#include "button.h"
#include "gpio.h"
#include "cmd.h"
#include "cdc.h"
#include "adc.h"
#include "tim.h"
#include "spi.h"
#include "icm20948.h"
#include "LinearS.h"


void hwInit(void);


#endif /* SRC_HW_HW_H_ */
