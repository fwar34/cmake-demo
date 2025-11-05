/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file    usart.c
 * @brief   This file provides code for the configuration
 *          of the USART instances.
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2025 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "usart.h"

/* USER CODE BEGIN 0 */
#include "protocol.h"
#include "receive_buffer.h"
#include <stdarg.h>
#include <stdio.h>
/* USER CODE END 0 */

UART_HandleTypeDef huart2;
DMA_HandleTypeDef hdma_usart2_rx;

/* USART2 init function */

void MX_USART2_UART_Init(void)
{

    /* USER CODE BEGIN USART2_Init 0 */

    /* USER CODE END USART2_Init 0 */

    /* USER CODE BEGIN USART2_Init 1 */

    /* USER CODE END USART2_Init 1 */
    huart2.Instance = USART2;
    huart2.Init.BaudRate = 115200;
    huart2.Init.WordLength = UART_WORDLENGTH_8B;
    huart2.Init.StopBits = UART_STOPBITS_1;
    huart2.Init.Parity = UART_PARITY_NONE;
    huart2.Init.Mode = UART_MODE_TX_RX;
    huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart2.Init.OverSampling = UART_OVERSAMPLING_16;
    if (HAL_UART_Init(&huart2) != HAL_OK) {
        Error_Handler();
    }
    /* USER CODE BEGIN USART2_Init 2 */

    /* USER CODE END USART2_Init 2 */
}

void HAL_UART_MspInit(UART_HandleTypeDef *uartHandle)
{

    GPIO_InitTypeDef GPIO_InitStruct = {0};
    if (uartHandle->Instance == USART2) {
        /* USER CODE BEGIN USART2_MspInit 0 */

        /* USER CODE END USART2_MspInit 0 */
        /* USART2 clock enable */
        __HAL_RCC_USART2_CLK_ENABLE();

        __HAL_RCC_GPIOA_CLK_ENABLE();
        /**USART2 GPIO Configuration
        PA2     ------> USART2_TX
        PA3     ------> USART2_RX
        */
        GPIO_InitStruct.Pin = GPIO_PIN_2;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

        GPIO_InitStruct.Pin = GPIO_PIN_3;
        GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

        /* USART2 DMA Init */
        /* USART2_RX Init */
        hdma_usart2_rx.Instance = DMA1_Channel6;
        hdma_usart2_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
        hdma_usart2_rx.Init.PeriphInc = DMA_PINC_DISABLE;
        hdma_usart2_rx.Init.MemInc = DMA_MINC_ENABLE;
        hdma_usart2_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
        hdma_usart2_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
        hdma_usart2_rx.Init.Mode = DMA_NORMAL;
        hdma_usart2_rx.Init.Priority = DMA_PRIORITY_LOW;
        if (HAL_DMA_Init(&hdma_usart2_rx) != HAL_OK) {
            Error_Handler();
        }

        __HAL_LINKDMA(uartHandle, hdmarx, hdma_usart2_rx);

        /* USART2 interrupt Init */
        HAL_NVIC_SetPriority(USART2_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(USART2_IRQn);
        /* USER CODE BEGIN USART2_MspInit 1 */

        /* USER CODE END USART2_MspInit 1 */
    }
}

void HAL_UART_MspDeInit(UART_HandleTypeDef *uartHandle)
{

    if (uartHandle->Instance == USART2) {
        /* USER CODE BEGIN USART2_MspDeInit 0 */

        /* USER CODE END USART2_MspDeInit 0 */
        /* Peripheral clock disable */
        __HAL_RCC_USART2_CLK_DISABLE();

        /**USART2 GPIO Configuration
        PA2     ------> USART2_TX
        PA3     ------> USART2_RX
        */
        HAL_GPIO_DeInit(GPIOA, GPIO_PIN_2 | GPIO_PIN_3);

        /* USART2 DMA DeInit */
        HAL_DMA_DeInit(uartHandle->hdmarx);

        /* USART2 interrupt Deinit */
        HAL_NVIC_DisableIRQ(USART2_IRQn);
        /* USER CODE BEGIN USART2_MspDeInit 1 */

        /* USER CODE END USART2_MspDeInit 1 */
    }
}

/* USER CODE BEGIN 1 */
static volatile uint8_t uart_tx_complete = 1; // 初始化为发送完成状态

// 串口发送完成回调函数
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart == &huart2) {
        uart_tx_complete = 1; // 标记发送完成
    }
}

void UartPrintf(const char *fmt, ...)
{
    static char message[256];
    va_list args;
    va_start(args, fmt);
    vsnprintf(message, sizeof(message), fmt, args);
    HAL_UART_Transmit(&huart2, (uint8_t *)message, strlen(message),
                      HAL_MAX_DELAY);
    va_end(args);
}

// 通过串口发送
void UartPrint(const char *msg, uint32_t msgLen)
{
    HAL_UART_Transmit(&huart2, (uint8_t *)msg, msgLen, HAL_MAX_DELAY);
}

// 安全的发送函数
void send_data_safely(char *data, uint16_t size)
{
    // while (!uart_tx_complete); // 等待上一次发送完成
    // uart_tx_complete = 0; // 标记开始新的发送
    // HAL_UART_Transmit_IT(&huart2, (uint8_t*)data, size);
    HAL_UART_Transmit(&huart2, (uint8_t *)data, size, HAL_MAX_DELAY);
}

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
    if (huart == &huart2) {
        GetReceiveBuffer()->dataLen = Size;
        GetReceiveBuffer()->consumePointer = GetReceiveBuffer()->data;
        Decode(GetProtocolDecoder(), GetReceiveBuffer());
        HAL_UARTEx_ReceiveToIdle_DMA(&huart2, GetReceiveBuffer()->data,
                                     GetReceiveBuffer()->capacity);
    }
}

// 使用方法
// 将这个函数放到usart.c文件的用户代码区域中，然后在main.c文件中调用printf函数即可。main.c文件需要包含stdio.h头文件。
int _write(int fd, char *pBuffer, int size)
{
    UNUSED(fd);
    // while (!uart_tx_complete);
    // uart_tx_complete = 0; // 标记开始新的发送
    // 避免串口发送过程中的死循环，加入超时机制
    const uint32_t timeout = 100000; // 超时周期，假设最多等待 100,000 次
    uint32_t timeout_counter = 0;

    for (int i = 0; i < size; i++) {
        // 等待直到串口的数据寄存器空
        while ((USART2->SR & 0x40) == 0) // 修改成你的串口
        {
            timeout_counter++;
            if (timeout_counter >= timeout) {
                // 如果超过超时限制，可以跳出并返回错误，或者做其他处理
                // uart_tx_complete = 1;
                return -1; // 返回错误
            }
        }
        USART2->DR = (uint8_t)pBuffer[i]; // 写入数据寄存器，发送字符
    }

    // uart_tx_complete = 1;
    return size; // 返回成功发送的字符数量
}

/* USER CODE END 1 */
