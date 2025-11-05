/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define led0_Pin GPIO_PIN_13
#define led0_GPIO_Port GPIOC
#define key0_Pin GPIO_PIN_0
#define key0_GPIO_Port GPIOA
#define SPI1_CS_Pin GPIO_PIN_4
#define SPI1_CS_GPIO_Port GPIOA
#define spi_scl_Pin GPIO_PIN_13
#define spi_scl_GPIO_Port GPIOB
#define spi_mosi_Pin GPIO_PIN_15
#define spi_mosi_GPIO_Port GPIOB
#define ec11_A_Pin GPIO_PIN_8
#define ec11_A_GPIO_Port GPIOA
#define ec11_B_Pin GPIO_PIN_9
#define ec11_B_GPIO_Port GPIOA
#define ec11_Key_Pin GPIO_PIN_10
#define ec11_Key_GPIO_Port GPIOA
#define dht11_Pin GPIO_PIN_15
#define dht11_GPIO_Port GPIOA
#define spi_blk_Pin GPIO_PIN_6
#define spi_blk_GPIO_Port GPIOB
#define spi_dc_Pin GPIO_PIN_7
#define spi_dc_GPIO_Port GPIOB
#define spi_res_Pin GPIO_PIN_8
#define spi_res_GPIO_Port GPIOB
#define spi_cs_Pin GPIO_PIN_9
#define spi_cs_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
