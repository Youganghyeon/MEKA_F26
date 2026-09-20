/*
 * spi.h
 *
 *  Created on: 2026. 2. 8.
 *      Author: yougang
 */

#ifndef SRC_COMMON_HW_INCLUDE_SPI_H_
#define SRC_COMMON_HW_INCLUDE_SPI_H_

#include "hw_def.h"

#ifdef _USE_HW_SPI

#define MAX_SPI_CH          HW_SPI_MAX_CH
#define _DEF_SPI1           HW_DEF_SPI1
#define _DEF_SPI2           HW_DEF_SPI2

typedef enum{
  IDLE = 0,
  Active,
  DONE,
} SPI_DMA_STATE;


void    spiInit(void);
bool    spiOpen(uint8_t ch);
bool    SPI_SendReceive_DMA(uint8_t ch, uint8_t *tx_data, uint8_t* rx_data, uint16_t length);
bool    SPI_SendReceive(uint8_t ch, uint8_t *tx_data, uint8_t* rx_data, uint16_t length);
void    spiRxCallbackRegister(uint8_t ch, void (*func)(void));
bool    IsSpiInit(uint8_t ch);
#endif



#endif /* SRC_COMMON_HW_INCLUDE_SPI_H_ */
