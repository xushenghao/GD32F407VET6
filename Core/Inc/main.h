/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file main.h
 * @brief          : Header for main.c file.
 *                   This file contains the common defines of the application.
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2024 STMicroelectronics.
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
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <rtthread.h>
#include <tim.h>
#include "lib.h"
#include "bsp.h"
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
#define WKUP_Pin GPIO_PIN_0
#define WKUP_GPIO_Port GPIOA
#define SPI1_CS_Pin GPIO_PIN_4
#define SPI1_CS_GPIO_Port GPIOA
#define LED_Pin GPIO_PIN_2
#define LED_GPIO_Port GPIOB
#define TMC_DIAG_Pin GPIO_PIN_10
#define TMC_DIAG_GPIO_Port GPIOD
#define TMC_DIR_Pin GPIO_PIN_11
#define TMC_DIR_GPIO_Port GPIOD
#define TMC_STEP_Pin GPIO_PIN_12
#define TMC_STEP_GPIO_Port GPIOD
#define TMC_MS2_Pin GPIO_PIN_13
#define TMC_MS2_GPIO_Port GPIOD
#define TMC_MS1_Pin GPIO_PIN_14
#define TMC_MS1_GPIO_Port GPIOD
#define TMC_EN_Pin GPIO_PIN_15
#define TMC_EN_GPIO_Port GPIOD
#define OLED_SDA_Pin GPIO_PIN_7
#define OLED_SDA_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
