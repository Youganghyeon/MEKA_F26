/*
 * hw.c
 *
 *  Created on: Dec 6, 2020
 *      Author: baram
 */


#include "hw.h"

__attribute((section(".b_version"))) boot_version_t boot_ver={
        "B260327R1",
        "STM32F411"
};

void hwInit(void)
{
  bspInit();
  gpioInit();
  rtcInit();
  resetInit();
  ledInit();
  usbInit();
  uartInit();
  cliInit();
  buttonInit();
  cdcInit();
  usbBegin(USB_CDC_MODE);
//  spiBegin(_DEF_SPI1);

}
