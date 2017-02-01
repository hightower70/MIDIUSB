/*****************************************************************************/
/* MIDI port handling on STM32F103                                           */
/*                                                                           */
/* Copyright (C) 2017 Laszlo Arvai                                           */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the GNU General Public License.  See the LICENSE file for details.     */
/*****************************************************************************/

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include "stm32f1xx_hal.h"
#include <halMIDIPort.h>
#include <midiInput.h>

/*****************************************************************************/
/* Module global variables                                                   */
/*****************************************************************************/
static UART_HandleTypeDef l_uart_handle;
static DMA_HandleTypeDef l_uart_tx_dma_handle;

/*****************************************************************************/
/* Function implementation                                                   */
/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
/// @brief Initializes MIDI Port 
void halMIDIPortInitialize(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;

	// USART Initialize
	__USART2_CLK_ENABLE();
	
  /**USART2 GPIO Configuration    
  PA2     ------> USART2_TX
  PA3     ------> USART2_RX 
  */
	GPIO_InitStruct.Pin = GPIO_PIN_2;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = GPIO_PIN_3;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_INPUT;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);	

	l_uart_handle.Instance = USART2;
	l_uart_handle.Init.BaudRate = 31250;
	l_uart_handle.Init.WordLength = UART_WORDLENGTH_8B;
	l_uart_handle.Init.StopBits = UART_STOPBITS_1;
	l_uart_handle.Init.Parity = UART_PARITY_NONE;
	l_uart_handle.Init.Mode = UART_MODE_TX_RX;
	HAL_UART_Init(&l_uart_handle);

  /* DMA controller clock enable */
	__HAL_RCC_DMA1_CLK_ENABLE();

	// Peripheral DMA initiaize
	l_uart_tx_dma_handle.Instance = DMA1_Channel7;
	l_uart_tx_dma_handle.Init.Direction = DMA_MEMORY_TO_PERIPH;
	l_uart_tx_dma_handle.Init.PeriphInc = DMA_PINC_DISABLE;
	l_uart_tx_dma_handle.Init.MemInc = DMA_MINC_ENABLE;
	l_uart_tx_dma_handle.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
	l_uart_tx_dma_handle.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
	l_uart_tx_dma_handle.Init.Mode = DMA_NORMAL;
	l_uart_tx_dma_handle.Init.Priority = DMA_PRIORITY_MEDIUM;
	HAL_DMA_Init(&l_uart_tx_dma_handle);

	__HAL_LINKDMA(&l_uart_handle, hdmatx, l_uart_tx_dma_handle);

	// Peripheral interrupt initialize
	HAL_NVIC_SetPriority(USART2_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(USART2_IRQn);
	
	// DMA interrupt initialize
	HAL_NVIC_SetPriority(DMA1_Channel7_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(DMA1_Channel7_IRQn);	
	
	// enable receiver input
	__HAL_USART_ENABLE_IT(&l_uart_handle, USART_IT_RXNE); 
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Send buffer content over MIDI interface
/// @param in_buffer Buffer containing data to send
/// @param in_buffer_length Number of bytes to send
void halMIDIPortSendBuffer(uint8_t* in_buffer, uint16_t in_buffer_length)
{
	// check if transmitter is idle
	if (!halMIDIIsTransmitterEmpty())
		return;
	
	// Start transmission
	HAL_UART_Transmit_DMA(&l_uart_handle, in_buffer, in_buffer_length);
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Returns transmitter state
/// @retval Returns true when transmitter is ready to send new block of data
bool halMIDIIsTransmitterEmpty(void)
{
	return (l_uart_handle.State == HAL_USART_STATE_READY || l_uart_handle.State == HAL_USART_STATE_BUSY_RX);
}

///////////////////////////////////////////////////////////////////////////////
/// @param UART transmit DMA IRQ handler
void DMA1_Channel7_IRQHandler(void)
{
	HAL_DMA_IRQHandler(&l_uart_tx_dma_handle);
}

///////////////////////////////////////////////////////////////////////////////
/// @param UART IRQ handler
void USART2_IRQHandler(void)
{
	uint32_t tmp1 = 0;
	uint32_t tmp2 = 0;
	uint8_t received_data;

	// Override HAL original interrupt handler and handle incoming characters individually
	// All other interrupt sources will be handled by the HAL interrupt handler function
	// Check for received character interrupt
	tmp1 = __HAL_UART_GET_FLAG(&l_uart_handle, UART_FLAG_RXNE);
	tmp2 = __HAL_UART_GET_IT_SOURCE(&l_uart_handle, UART_IT_RXNE);
	if ((tmp1 != RESET) && (tmp2 != RESET))
	{
		// get received byte
		received_data = (uint8_t)(l_uart_handle.Instance->DR & (uint8_t)0x00FF);

		// process received byte
		midiInputDataReceived(received_data);
	}

  // HAL interrupt handler
	HAL_UART_IRQHandler(&l_uart_handle);
}
