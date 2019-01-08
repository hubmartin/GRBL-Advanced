/*
  USART.c - USART implementation
  Part of STM32F4_HAL

  Copyright (c)	2017 Patrick F.

  STM32F4_HAL is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.
  STM32F4_HAL is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  You should have received a copy of the GNU General Public License
  along with STM32F4_HAL.  If not, see <http://www.gnu.org/licenses/>.
*/
#include <stdio.h>
#include "USART.h"
#include "FIFO_USART.h"


static uint8_t FifoInit = 0;


void Usart_Init(USART_TypeDef *usart, uint32_t baud)
{
	USART_InitTypeDef USART_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;

	if(!FifoInit) {
		// Initialize fifo once
		FifoUsart_Init();
		FifoInit = 1;
	}

	if(usart == USART1) {
		/* Enable GPIO clock */
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
		RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);

		GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_7);
		GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_7);

		/* Configure USART Tx RX as alternate function push-pull */
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_Level_2;
		GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
		GPIO_Init(GPIOA, &GPIO_InitStructure);

		USART_OverSampling8Cmd(USART1, ENABLE);

		USART_InitStructure.USART_BaudRate = baud;
		USART_InitStructure.USART_WordLength = USART_WordLength_8b;
		USART_InitStructure.USART_StopBits = USART_StopBits_1;
		USART_InitStructure.USART_Parity = USART_Parity_No;
		USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
		USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

		/* USART configuration */
		USART_Init(USART1, &USART_InitStructure);

		NVIC_InitTypeDef NVIC_InitStructure;

		/* Enable the USARTx Interrupt */
		NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
		NVIC_Init(&NVIC_InitStructure);

	} else if(usart == USART2) {
		/* Enable GPIO clock */
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
		RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);

		GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_7);
		GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_7);

		/* Configure USART Tx as alternate function push-pull */
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_Level_2;
		GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
		GPIO_Init(GPIOA, &GPIO_InitStructure);

		/* Enable the USART OverSampling by 8 */
		USART_OverSampling8Cmd(USART2, ENABLE);

		USART_InitStructure.USART_BaudRate = baud;
		USART_InitStructure.USART_WordLength = USART_WordLength_8b;
		USART_InitStructure.USART_StopBits = USART_StopBits_1;
		USART_InitStructure.USART_Parity = USART_Parity_No;
		USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
		USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

		/* USART configuration */
		USART_Init(USART2, &USART_InitStructure);

		NVIC_InitTypeDef NVIC_InitStructure;

		/* Enable the USARTx Interrupt */
		NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
		NVIC_Init(&NVIC_InitStructure);

	}

	/* Enable the Receive interrupt*/
	USART_ITConfig(usart, USART_IT_RXNE, ENABLE);

	/* Enable USART */
	USART_Cmd(usart, ENABLE);
}

void Usart_Put(USART_TypeDef *usart, char c)
{
	if(usart == STDOUT)
		FifoUsart_Insert(STDOUT_NUM, USART_DIR_TX, c);

	// Enable sending via interrupt
	Usart_TxInt(usart, true);
}

void Usart_Write(USART_TypeDef *usart, uint8_t *data, uint8_t len)
{
	uint8_t i = 0;

	while(len--)
	{
		if(usart == STDOUT)
			FifoUsart_Insert(STDOUT_NUM, USART_DIR_TX, data[i++]);
	}

	// Enable sending via interrupt
	Usart_TxInt(usart, true);
}

void Usart_TxInt(USART_TypeDef *usart, bool enable)
{
	if(enable)
	{
		USART_ITConfig(usart, USART_IT_TXE, ENABLE);
	}
	else
	{
		USART_ITConfig(usart, USART_IT_TXE, DISABLE);
	}
}

void Usart_RxInt(USART_TypeDef *usart, bool enable)
{
	if(enable)
	{
		USART_ITConfig(usart, USART_IT_RXNE, ENABLE);
	}
	else
	{
		USART_ITConfig(usart, USART_IT_RXNE, DISABLE);
	}
}

