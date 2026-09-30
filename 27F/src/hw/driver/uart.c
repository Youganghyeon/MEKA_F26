/*
 * uart.c
 *
 *  Created on: 2020. 12. 8.
 *      Author: baram
 *
 *  채널 매핑
 *    _DEF_UART1 : USB CDC
 *    _DEF_UART2 : USART1 (DMA1_Channel1, PC 연결, PC4[tx], PA10[rx])
 *    _DEF_UART3 : USART2 (DMA1_Channel2, TEL 연결)
 *    _DEF_UART4 : USART3 (DMA1_Channel3, GPS 연결)
 *
 *  ※ UART_MAX_CH >= 4 가정
 */


#include "uart.h"
#include "cdc.h"
#include "qbuffer.h"


#ifdef _USE_HW_UART

#define UART_RX_BUF_LENGTH		256
#define UART_PRINTF_BUF_LENGTH	256
#define UART_TX_TIMEOUT_MS		100


typedef struct
{
	bool                is_open;
	uint32_t            baud;

	qbuffer_t           qbuffer;
	uint8_t             rx_buf[UART_RX_BUF_LENGTH];	// 채널별 독립 버퍼

	UART_HandleTypeDef *p_huart;
	DMA_HandleTypeDef  *p_hdma_rx;
	USART_TypeDef      *p_instance;
	IRQn_Type           dma_irq;
} uart_tbl_t;


// 다른 파일(stm32g4xx_it.c 등)에서 extern으로 쓸 수 있으니 static 아님
UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;
UART_HandleTypeDef huart3;
DMA_HandleTypeDef  hdma_usart1_rx;
DMA_HandleTypeDef  hdma_usart2_rx;
DMA_HandleTypeDef  hdma_usart3_rx;


static uart_tbl_t uart_tbl[UART_MAX_CH] =
{
	[_DEF_UART2] =
	{
		.p_huart    = &huart1,
		.p_hdma_rx  = &hdma_usart1_rx,
		.p_instance = USART1,
		.dma_irq    = DMA1_Channel1_IRQn,
	},
	[_DEF_UART3] =
	{
		.p_huart    = &huart2,
		.p_hdma_rx  = &hdma_usart2_rx,
		.p_instance = USART2,
		.dma_irq    = DMA1_Channel2_IRQn,
	},
	[_DEF_UART4] =
	{
		.p_huart    = &huart3,
		.p_hdma_rx  = &hdma_usart3_rx,
		.p_instance = USART3,
		.dma_irq    = DMA1_Channel3_IRQn,
	},
};


static bool uartOpenDma(uint8_t ch, uint32_t baud);
static void uartGpioInit(GPIO_TypeDef *port, uint32_t pin, uint32_t alternate);
static void uartDmaRxInit(UART_HandleTypeDef *huart, DMA_HandleTypeDef *hdma,
		DMA_Channel_TypeDef *channel, uint32_t request);




bool uartInit(void)
{
	for (int i=0; i<UART_MAX_CH; i++)
	{
		uart_tbl[i].is_open = false;
		uart_tbl[i].baud    = 115200;
	}

	return true;
}

bool uartOpen(uint8_t ch, uint32_t baud)
{
	bool ret = false;

	if (ch >= UART_MAX_CH)
	{
		return false;
	}

	switch(ch)
	{
	case _DEF_UART1:
		uart_tbl[ch].baud    = baud;
		uart_tbl[ch].is_open = true;
		ret = true;
		break;

	case _DEF_UART2:
	case _DEF_UART3:
	case _DEF_UART4:
		ret = uartOpenDma(ch, baud);
		break;
	}

	return ret;
}

static bool uartOpenDma(uint8_t ch, uint32_t baud)
{
	uart_tbl_t         *p_uart  = &uart_tbl[ch];
	UART_HandleTypeDef *p_huart = p_uart->p_huart;

	p_huart->Instance                    = p_uart->p_instance;
	p_huart->Init.BaudRate               = baud;
	p_huart->Init.WordLength             = UART_WORDLENGTH_8B;
	p_huart->Init.StopBits               = UART_STOPBITS_1;
	p_huart->Init.Parity                 = UART_PARITY_NONE;
	p_huart->Init.Mode                   = UART_MODE_TX_RX;
	p_huart->Init.HwFlowCtl              = UART_HWCONTROL_NONE;
	p_huart->Init.OverSampling           = UART_OVERSAMPLING_16;
	p_huart->Init.OneBitSampling         = UART_ONE_BIT_SAMPLE_DISABLE;
	p_huart->Init.ClockPrescaler         = UART_PRESCALER_DIV1;
	p_huart->AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;

	HAL_UART_DeInit(p_huart);

	qbufferCreate(&p_uart->qbuffer, p_uart->rx_buf, UART_RX_BUF_LENGTH);

	HAL_NVIC_SetPriority(p_uart->dma_irq, 0, 0);
	HAL_NVIC_EnableIRQ(p_uart->dma_irq);
	if (HAL_UART_Init(p_huart) != HAL_OK)
	{
		Error_Handler();
	}
	if (HAL_UARTEx_SetTxFifoThreshold(p_huart, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
	{
		Error_Handler();
	}
	if (HAL_UARTEx_SetRxFifoThreshold(p_huart, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
	{
		Error_Handler();
	}
	if (HAL_UARTEx_DisableFifoMode(p_huart) != HAL_OK)
	{
		Error_Handler();
	}

	if (HAL_UART_Receive_DMA(p_huart, p_uart->rx_buf, UART_RX_BUF_LENGTH) != HAL_OK)
	{
		HAL_NVIC_DisableIRQ(p_uart->dma_irq);
		HAL_UART_DeInit(p_huart);
		return false;
	}

	// DMA가 쓰고 있는 위치(in)에 읽기 위치(out)를 맞춤
	p_uart->qbuffer.in  = p_uart->qbuffer.len - p_uart->p_hdma_rx->Instance->CNDTR;
	p_uart->qbuffer.out = p_uart->qbuffer.in;

	p_uart->baud    = baud;
	p_uart->is_open = true;

	return true;
}

bool uartClose(uint8_t ch)
{
	if (ch >= UART_MAX_CH)
	{
		return false;
	}

	switch(ch)
	{
	case _DEF_UART1:
		break;

	case _DEF_UART2:
	case _DEF_UART3:
	case _DEF_UART4:
		if (uart_tbl[ch].is_open == true)
		{
			HAL_UART_DMAStop(uart_tbl[ch].p_huart);
			HAL_NVIC_DisableIRQ(uart_tbl[ch].dma_irq);
			HAL_UART_DeInit(uart_tbl[ch].p_huart);
		}
		break;
	}

	uart_tbl[ch].is_open = false;

	return true;
}

uint32_t uartAvailable(uint8_t ch)
{
	uint32_t ret = 0;

	if (ch >= UART_MAX_CH)
	{
		return 0;
	}

	switch(ch)
	{
	case _DEF_UART1:
		ret = cdcAvailable();
		break;

	case _DEF_UART2:
	case _DEF_UART3:
	case _DEF_UART4:
		if (uart_tbl[ch].is_open == true)
		{
			uart_tbl[ch].qbuffer.in = uart_tbl[ch].qbuffer.len - uart_tbl[ch].p_hdma_rx->Instance->CNDTR;
			ret = qbufferAvailable(&uart_tbl[ch].qbuffer);
		}
		break;
	}

	return ret;
}

uint8_t uartRead(uint8_t ch)
{
	uint8_t ret = 0;

	if (ch >= UART_MAX_CH)
	{
		return 0;
	}

	switch(ch)
	{
	case _DEF_UART1:
		ret = cdcRead();
		break;

	case _DEF_UART2:
	case _DEF_UART3:
	case _DEF_UART4:
		if (uart_tbl[ch].is_open == true)
		{
			qbufferRead(&uart_tbl[ch].qbuffer, &ret, 1);
		}
		break;
	}

	return ret;
}

uint32_t uartWrite(uint8_t ch, uint8_t *p_data, uint32_t length)
{
	uint32_t ret = 0;

	if (ch >= UART_MAX_CH)
	{
		return 0;
	}

	switch(ch)
	{
	case _DEF_UART1:
		ret = cdcWrite(p_data, length);
		break;

	case _DEF_UART2:
	case _DEF_UART3:
	case _DEF_UART4:
		if (uart_tbl[ch].is_open == true)
		{
			// HAL_UART_Transmit()의 길이 인자는 uint16_t
			if (HAL_UART_Transmit(uart_tbl[ch].p_huart, p_data, length, UART_TX_TIMEOUT_MS) == HAL_OK)
			{
				ret = length;
			}
		}
		break;
	}

	return ret;
}

uint32_t uartPrintf(uint8_t ch, char *fmt, ...)
{
	char     buf[UART_PRINTF_BUF_LENGTH];
	va_list  args;
	int      len;

	va_start(args, fmt);
	len = vsnprintf(buf, sizeof(buf), fmt, args);
	va_end(args);

	if (len <= 0)
	{
		return 0;
	}
	// vsnprintf는 잘리기 전 길이를 반환하므로 버퍼 크기로 제한
	if ((uint32_t)len >= sizeof(buf))
	{
		len = sizeof(buf) - 1;
	}

	return uartWrite(ch, (uint8_t *)buf, (uint32_t)len);
}

uint32_t uartGetBaud(uint8_t ch)
{
	uint32_t ret = 0;

	if (ch >= UART_MAX_CH)
	{
		return 0;
	}

	switch(ch)
	{
	case _DEF_UART1:
		ret = cdcGetBaud();
		break;

	case _DEF_UART2:
	case _DEF_UART3:
	case _DEF_UART4:
		ret = uart_tbl[ch].p_huart->Init.BaudRate;
		break;
	}

	return ret;
}




/* ------------------------------------------------------------------
 *  HAL MSP (클럭 / GPIO / DMA)
 * ------------------------------------------------------------------ */

static void uartGpioInit(GPIO_TypeDef *port, uint32_t pin, uint32_t alternate)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};

	GPIO_InitStruct.Pin       = pin;
	GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull      = GPIO_NOPULL;
	GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_LOW;
	GPIO_InitStruct.Alternate = alternate;
	HAL_GPIO_Init(port, &GPIO_InitStruct);
}

static void uartDmaRxInit(UART_HandleTypeDef *huart, DMA_HandleTypeDef *hdma,
		DMA_Channel_TypeDef *channel, uint32_t request)
{
	__HAL_RCC_DMAMUX1_CLK_ENABLE();
	__HAL_RCC_DMA1_CLK_ENABLE();

	hdma->Instance                 = channel;
	hdma->Init.Request             = request;
	hdma->Init.Direction           = DMA_PERIPH_TO_MEMORY;
	hdma->Init.PeriphInc           = DMA_PINC_DISABLE;
	hdma->Init.MemInc              = DMA_MINC_ENABLE;
	hdma->Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
	hdma->Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
	hdma->Init.Mode                = DMA_CIRCULAR;
	hdma->Init.Priority            = DMA_PRIORITY_LOW;
	if (HAL_DMA_Init(hdma) != HAL_OK)
	{
		Error_Handler();
	}

	__HAL_LINKDMA(huart, hdmarx, *hdma);
}

void HAL_UART_MspInit(UART_HandleTypeDef* uartHandle)
{
	RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

	if (uartHandle->Instance == USART1)
	{
		PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1;
		PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK2;
		if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
		{
			Error_Handler();
		}

		__HAL_RCC_USART1_CLK_ENABLE();
		__HAL_RCC_GPIOC_CLK_ENABLE();
		__HAL_RCC_GPIOA_CLK_ENABLE();

		/**USART1 GPIO Configuration
		 PC4     ------> USART1_TX
		 PA10    ------> USART1_RX
		 */
		uartGpioInit(PC_TX_GPIO_Port, PC_TX_Pin, GPIO_AF7_USART1);
		uartGpioInit(PC_RX_GPIO_Port, PC_RX_Pin, GPIO_AF7_USART1);

		uartDmaRxInit(uartHandle, &hdma_usart1_rx, DMA1_Channel1, DMA_REQUEST_USART1_RX);
	}
	else if (uartHandle->Instance == USART2)
	{
		PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART2;
		PeriphClkInit.Usart2ClockSelection = RCC_USART2CLKSOURCE_PCLK1;
		if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
		{
			Error_Handler();
		}

		__HAL_RCC_USART2_CLK_ENABLE();
		__HAL_RCC_GPIOA_CLK_ENABLE();
		__HAL_RCC_GPIOB_CLK_ENABLE();

		/**USART2 GPIO Configuration
		 PA15    ------> USART2_RX
		 PB3     ------> USART2_TX
		 */
		uartGpioInit(TEL_RX_GPIO_Port, TEL_RX_Pin, GPIO_AF7_USART2);
		uartGpioInit(TEL_TX_GPIO_Port, TEL_TX_Pin, GPIO_AF7_USART2);

		uartDmaRxInit(uartHandle, &hdma_usart2_rx, DMA1_Channel2, DMA_REQUEST_USART2_RX);
	}
	else if (uartHandle->Instance == USART3)
	{
		PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART3;
		PeriphClkInit.Usart3ClockSelection = RCC_USART3CLKSOURCE_PCLK1;
		if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
		{
			Error_Handler();
		}

		__HAL_RCC_USART3_CLK_ENABLE();
		__HAL_RCC_GPIOB_CLK_ENABLE();

		/**USART3 GPIO Configuration
		 PB10    ------> USART3_TX
		 PB11    ------> USART3_RX
		 */
		uartGpioInit(GPIOB, GPS_TX_Pin | GPS_RX_Pin, GPIO_AF7_USART3);

		uartDmaRxInit(uartHandle, &hdma_usart3_rx, DMA1_Channel3, DMA_REQUEST_USART3_RX);
	}
}

void HAL_UART_MspDeInit(UART_HandleTypeDef* uartHandle)
{
	if (uartHandle->Instance == USART1)
	{
		__HAL_RCC_USART1_CLK_DISABLE();

		HAL_GPIO_DeInit(PC_TX_GPIO_Port, PC_TX_Pin);
		HAL_GPIO_DeInit(PC_RX_GPIO_Port, PC_RX_Pin);

		HAL_DMA_DeInit(uartHandle->hdmarx);
	}
	else if (uartHandle->Instance == USART2)
	{
		__HAL_RCC_USART2_CLK_DISABLE();

		HAL_GPIO_DeInit(TEL_RX_GPIO_Port, TEL_RX_Pin);
		HAL_GPIO_DeInit(TEL_TX_GPIO_Port, TEL_TX_Pin);

		HAL_DMA_DeInit(uartHandle->hdmarx);
	}
	else if (uartHandle->Instance == USART3)
	{
		__HAL_RCC_USART3_CLK_DISABLE();

		HAL_GPIO_DeInit(GPIOB, GPS_TX_Pin | GPS_RX_Pin);

		HAL_DMA_DeInit(uartHandle->hdmarx);
	}
}


#endif
