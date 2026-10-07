/*
 * spi.c
 *
 *  Created on: 2026. 2. 8.
 *      Author: yougang
 */


#include "spi.h"
#include "sd_spi.h"

#ifdef _USE_HW_SPI
SPI_HandleTypeDef hspi1;
SPI_HandleTypeDef hspi2;
DMA_HandleTypeDef hdma_spi1_rx;
DMA_HandleTypeDef hdma_spi1_tx;
DMA_HandleTypeDef hdma_spi2_rx;
DMA_HandleTypeDef hdma_spi2_tx;

typedef struct{
	SPI_HandleTypeDef* spi_handle;
	DMA_HandleTypeDef* dma_tx;
	DMA_HandleTypeDef* dma_rx;
	bool isInit;
	bool isReceived;
	void (*txfunc)(void);
	void (*rxfunc)(void);

}Spi_tbl_t;


Spi_tbl_t spi_tbl[MAX_SPI_CH];


void spiInit(void)
{
	for(int i=0; i<MAX_SPI_CH; i++)
	{
		spi_tbl[i].isInit = false;
	}
}


bool spiOpen(uint8_t ch)
{
	bool ret=false;
	SPI_HandleTypeDef* p_spi_handle;
	switch(ch)
	{
	case _DEF_SPI1:
		__HAL_RCC_DMA1_CLK_ENABLE();
		HAL_NVIC_SetPriority(DMA1_Channel4_IRQn, 0, 0);
		HAL_NVIC_EnableIRQ(DMA1_Channel4_IRQn);
		HAL_NVIC_SetPriority(DMA1_Channel6_IRQn, 0, 0);
		HAL_NVIC_EnableIRQ(DMA1_Channel6_IRQn);


		spi_tbl[ch].spi_handle = &hspi1;
		p_spi_handle = spi_tbl[ch].spi_handle;
		p_spi_handle->Instance = SPI1;
		p_spi_handle->Init.Mode = SPI_MODE_MASTER;
		p_spi_handle->Init.Direction = SPI_DIRECTION_2LINES;
		p_spi_handle->Init.DataSize = SPI_DATASIZE_8BIT;
		p_spi_handle->Init.CLKPolarity = SPI_POLARITY_HIGH;
		p_spi_handle->Init.CLKPhase = SPI_PHASE_2EDGE;
		p_spi_handle->Init.NSS = SPI_NSS_SOFT;
		p_spi_handle->Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_64;//8
		p_spi_handle->Init.FirstBit = SPI_FIRSTBIT_MSB;
		p_spi_handle->Init.TIMode = SPI_TIMODE_DISABLE;
		p_spi_handle->Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
		p_spi_handle->Init.CRCPolynomial = 10;
		if (HAL_SPI_Init(p_spi_handle) != HAL_OK)
		{
			Error_Handler();
		}
		else
		{
			ret=true;
		}
		break;
	case _DEF_SPI2:
		spi_tbl[ch].spi_handle = &hspi2;
		p_spi_handle = spi_tbl[ch].spi_handle;
		p_spi_handle->Instance = SPI2;
		p_spi_handle->Init.Mode = SPI_MODE_MASTER;
		p_spi_handle->Init.Direction = SPI_DIRECTION_2LINES;
		p_spi_handle->Init.DataSize = SPI_DATASIZE_8BIT;
		/* SD cards require SPI mode 0 during initialization. */
		p_spi_handle->Init.CLKPolarity = SPI_POLARITY_LOW;
		p_spi_handle->Init.CLKPhase = SPI_PHASE_1EDGE;
		p_spi_handle->Init.NSS = SPI_NSS_SOFT;
		/* Start SD initialization at the slowest divider; speed up after init. */
		p_spi_handle->Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_256;
		p_spi_handle->Init.FirstBit = SPI_FIRSTBIT_MSB;
		p_spi_handle->Init.TIMode = SPI_TIMODE_DISABLE;
		p_spi_handle->Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
		p_spi_handle->Init.CRCPolynomial = 10;
		__HAL_RCC_DMA2_CLK_ENABLE();
		 HAL_NVIC_SetPriority(DMA2_Channel1_IRQn, 0, 0);
		  HAL_NVIC_EnableIRQ(DMA2_Channel1_IRQn);
		  /* DMA2_Channel2_IRQn interrupt configuration */
		  HAL_NVIC_SetPriority(DMA2_Channel2_IRQn, 0, 0);
		  HAL_NVIC_EnableIRQ(DMA2_Channel2_IRQn);

		if (HAL_SPI_Init(p_spi_handle) != HAL_OK)
		{
			Error_Handler();
		}
		else
		{
			ret=true;
		}
		break;
	default:
		break;
	}
	spi_tbl[ch].isInit = true;
	return ret;
}

bool spiSetBaudPrescaler(uint8_t ch, uint32_t prescaler)
{
	if (ch >= MAX_SPI_CH || !spi_tbl[ch].isInit || spi_tbl[ch].spi_handle == NULL)
	{
		return false;
	}

	SPI_HandleTypeDef *p_spi_handle = spi_tbl[ch].spi_handle;
	if (HAL_SPI_DeInit(p_spi_handle) != HAL_OK)
	{
		return false;
	}

	p_spi_handle->Init.BaudRatePrescaler = prescaler;
	return HAL_SPI_Init(p_spi_handle) == HAL_OK;
}


bool SPI_SendReceive(uint8_t ch, uint8_t *tx_data, uint8_t* rx_data, uint16_t length)
{
	bool ret=false;
	HAL_StatusTypeDef status;
	status=HAL_SPI_TransmitReceive(spi_tbl[ch].spi_handle, tx_data, rx_data, length, 100);
	if(status == HAL_OK)
	{
		ret=true;
	}
	return ret;
}


bool SPI_SendReceive_DMA(uint8_t ch, uint8_t *tx_data, uint8_t* rx_data, uint16_t length)
{
	bool ret=false;
	HAL_StatusTypeDef status;
	status=HAL_SPI_TransmitReceive_DMA(spi_tbl[ch].spi_handle, tx_data, rx_data, length);
	if(status == HAL_OK)
	{
		ret=true;
	}
	return ret;
}

bool IsSpiInit(uint8_t ch)
{
	return spi_tbl[ch].isInit;
}

void spiRxCallbackRegister(uint8_t ch, void (*func)(void))
{
	if(func != NULL)
	{
		spi_tbl[ch].rxfunc = func;
	}
}

void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi)
{
	/* Prevent unused argument(s) compilation warning */
	if (hspi != NULL && hspi->Instance == SPI2)
	{
		SD_SPI_OnTxRxComplete(hspi);
	}
			for(int i=0; i<MAX_SPI_CH; i++)
			{
				if(spi_tbl[i].spi_handle == NULL) continue;
				if(hspi->Instance == spi_tbl[i].spi_handle->Instance)
				{
					if (spi_tbl[i].rxfunc != NULL) spi_tbl[i].rxfunc();
				}
			}
}

void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
{
	SD_SPI_OnTxComplete(hspi);
}
void HAL_SPI_MspInit(SPI_HandleTypeDef* spiHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(spiHandle->Instance==SPI1)
  {
  /* USER CODE BEGIN SPI1_MspInit 0 */

  /* USER CODE END SPI1_MspInit 0 */
    /* SPI1 clock enable */
    __HAL_RCC_SPI1_CLK_ENABLE();

    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**SPI1 GPIO Configuration
    PA5     ------> SPI1_SCK
    PA6     ------> SPI1_MISO
    PA7     ------> SPI1_MOSI
    */
    GPIO_InitStruct.Pin = IMU_SCK_Pin|IMU_MISO_Pin|IMU_MOSI_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF5_SPI1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* SPI1 DMA Init */
    /* SPI1_RX Init */
    hdma_spi1_rx.Instance = DMA1_Channel4;
    hdma_spi1_rx.Init.Request = DMA_REQUEST_SPI1_RX;
    hdma_spi1_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_spi1_rx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_spi1_rx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_spi1_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_spi1_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_spi1_rx.Init.Mode = DMA_NORMAL;
    hdma_spi1_rx.Init.Priority = DMA_PRIORITY_LOW;
    if (HAL_DMA_Init(&hdma_spi1_rx) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(spiHandle,hdmarx,hdma_spi1_rx);

    /* SPI1_TX Init */
    hdma_spi1_tx.Instance = DMA1_Channel6;
    hdma_spi1_tx.Init.Request = DMA_REQUEST_SPI1_TX;
    hdma_spi1_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
    hdma_spi1_tx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_spi1_tx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_spi1_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_spi1_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_spi1_tx.Init.Mode = DMA_NORMAL;
    hdma_spi1_tx.Init.Priority = DMA_PRIORITY_LOW;
    if (HAL_DMA_Init(&hdma_spi1_tx) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(spiHandle,hdmatx,hdma_spi1_tx);

  /* USER CODE BEGIN SPI1_MspInit 1 */

  /* USER CODE END SPI1_MspInit 1 */
  }
  else if(spiHandle->Instance==SPI2)
  {
  /* USER CODE BEGIN SPI2_MspInit 0 */

  /* USER CODE END SPI2_MspInit 0 */
    /* SPI2 clock enable */
    __HAL_RCC_SPI2_CLK_ENABLE();

    __HAL_RCC_GPIOB_CLK_ENABLE();
    /**SPI2 GPIO Configuration
    PB13     ------> SPI2_SCK
    PB14     ------> SPI2_MISO
    PB15     ------> SPI2_MOSI
    */
    GPIO_InitStruct.Pin = SD_SCK_Pin|SD_MISO_Pin|SD_MOSI_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF5_SPI2;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* SPI2 DMA Init */
    /* SPI2_RX Init */
    hdma_spi2_rx.Instance = DMA2_Channel1;
    hdma_spi2_rx.Init.Request = DMA_REQUEST_SPI2_RX;
    hdma_spi2_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_spi2_rx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_spi2_rx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_spi2_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_spi2_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_spi2_rx.Init.Mode = DMA_NORMAL;
    hdma_spi2_rx.Init.Priority = DMA_PRIORITY_HIGH;
    if (HAL_DMA_Init(&hdma_spi2_rx) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(spiHandle,hdmarx,hdma_spi2_rx);

    /* SPI2_TX Init */
    hdma_spi2_tx.Instance = DMA2_Channel2;
    hdma_spi2_tx.Init.Request = DMA_REQUEST_SPI2_TX;
    hdma_spi2_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
    hdma_spi2_tx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_spi2_tx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_spi2_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_spi2_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_spi2_tx.Init.Mode = DMA_NORMAL;
    hdma_spi2_tx.Init.Priority = DMA_PRIORITY_HIGH;
    if (HAL_DMA_Init(&hdma_spi2_tx) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(spiHandle,hdmatx,hdma_spi2_tx);

  /* USER CODE BEGIN SPI2_MspInit 1 */

  /* USER CODE END SPI2_MspInit 1 */
  }
}

void HAL_SPI_MspDeInit(SPI_HandleTypeDef* spiHandle)
{

  if(spiHandle->Instance==SPI1)
  {
  /* USER CODE BEGIN SPI1_MspDeInit 0 */

  /* USER CODE END SPI1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_SPI1_CLK_DISABLE();

    /**SPI1 GPIO Configuration
    PA5     ------> SPI1_SCK
    PA6     ------> SPI1_MISO
    PA7     ------> SPI1_MOSI
    */
    HAL_GPIO_DeInit(GPIOA, IMU_SCK_Pin|IMU_MISO_Pin|IMU_MOSI_Pin);

    /* SPI1 DMA DeInit */
    HAL_DMA_DeInit(spiHandle->hdmarx);
    HAL_DMA_DeInit(spiHandle->hdmatx);
  /* USER CODE BEGIN SPI1_MspDeInit 1 */

  /* USER CODE END SPI1_MspDeInit 1 */
  }
  else if(spiHandle->Instance==SPI2)
  {
  /* USER CODE BEGIN SPI2_MspDeInit 0 */

  /* USER CODE END SPI2_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_SPI2_CLK_DISABLE();

    /**SPI2 GPIO Configuration
    PB13     ------> SPI2_SCK
    PB14     ------> SPI2_MISO
    PB15     ------> SPI2_MOSI
    */
    HAL_GPIO_DeInit(GPIOB, SD_SCK_Pin|SD_MISO_Pin|SD_MOSI_Pin);

    /* SPI2 DMA DeInit */
    HAL_DMA_DeInit(spiHandle->hdmarx);
    HAL_DMA_DeInit(spiHandle->hdmatx);
  /* USER CODE BEGIN SPI2_MspDeInit 1 */

  /* USER CODE END SPI2_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

#endif
