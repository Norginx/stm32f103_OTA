/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
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
#include "stm32f1xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */
#define FLASH_START     0x08000000                                              //单片机FLASH起始运行地址
#define PAGE_SIZE     2048                                                    //页大小，zet6是2048一页
#define FLASH_PAGE_NUM  256                                                     //一共有256页
#define BOOT_PAGE_NUM   20                                                      //boot占20页
#define SYS_PAGE        256-BOOT_PAGE_NUM                                     //其余是主系统占用
#define SYS_START_PAG   BOOT_PAGE_NUM                                        //主系统起始是从第几页开始的
#define SYS_START_ADDR  FLASH_START+SYS_START_PAG*2048                          //主系统的开始地址0800A000
/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */
#define WAIT_CMD    1
#define UPDATA_SYS  2
#define INTO_CMD    4
#define UPDATE      5

#define EVENT_C_BIT         1
#define EVENT_MSG_BIT       2
#define EVENT_FLASH_BIT     4
/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
