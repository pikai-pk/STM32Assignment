/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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
#define SW4_Pin GPIO_PIN_4
#define SW4_GPIO_Port GPIOE
#define BUZZER_PWM_Pin GPIO_PIN_4
#define BUZZER_PWM_GPIO_Port GPIOB
#define SER_Pin GPIO_PIN_8
#define SER_GPIO_Port GPIOC
#define DISEN_Pin GPIO_PIN_9
#define DISEN_GPIO_Port GPIOC
#define DISLK_Pin GPIO_PIN_8
#define DISLK_GPIO_Port GPIOA
#define SCK_Pin GPIO_PIN_11
#define SCK_GPIO_Port GPIOA
#define A3_Pin GPIO_PIN_12
#define A3_GPIO_Port GPIOA
#define A0_Pin GPIO_PIN_15
#define A0_GPIO_Port GPIOA
#define LED1_Pin GPIO_PIN_8
#define LED1_GPIO_Port GPIOE
#define LED2_Pin GPIO_PIN_9
#define LED2_GPIO_Port GPIOE
#define LED3_Pin GPIO_PIN_10
#define LED3_GPIO_Port GPIOE
#define LED4_Pin GPIO_PIN_11
#define LED4_GPIO_Port GPIOE
#define LED5_Pin GPIO_PIN_12
#define LED5_GPIO_Port GPIOE
#define LED6_Pin GPIO_PIN_13
#define LED6_GPIO_Port GPIOE
#define LED7_Pin GPIO_PIN_14
#define LED7_GPIO_Port GPIOE
#define LED8_Pin GPIO_PIN_15
#define LED8_GPIO_Port GPIOE
#define SW1_Pin GPIO_PIN_1
#define SW1_GPIO_Port GPIOE
#define A1_Pin GPIO_PIN_10
#define A1_GPIO_Port GPIOC
#define A2_Pin GPIO_PIN_11
#define A2_GPIO_Port GPIOC
#define KEY1_Pin SW1_Pin
#define KEY1_GPIO_Port SW1_GPIO_Port
#define KEY2_Pin SW4_Pin
#define KEY2_GPIO_Port SW4_GPIO_Port

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
