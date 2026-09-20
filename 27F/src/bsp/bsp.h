/*
 * bsp.h
 *
 *  Created on: Dec 6, 2020
 *      Author: baram
 */

#ifndef SRC_BSP_BSP_H_
#define SRC_BSP_BSP_H_


#include "def.h"
#include "stm32g4xx_hal.h"

#define _USE_LOG_PRINT    1

#define DWT_START()   (DWT->CYCCNT = 0)
#define DWT_STOP()      ((float)DWT->CYCCNT / 168.0f)

#if _USE_LOG_PRINT
#define logPrintf(fmt, ...)     printf(fmt, ##__VA_ARGS__)
#else
#define logPrintf(fmt, ...)
#endif



void bspInit(void);

void delay(uint32_t ms);
uint32_t millis(void);
void bspDeinit(void);
void Error_Handler(void);

bool DWT_Delay_Init(void);
void DWT_Delay_us(uint32_t microseconds);

#endif /* SRC_BSP_BSP_H_ */
