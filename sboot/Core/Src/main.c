/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "dma.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdio.h"
#include "crc.h"
#include "flash_ctrl.h"
#include "boot.h"
#include "norflash.h"
#include "string.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
uint8_t rx_buf[270];
uint8_t Updatebuff[PAGE_SIZE];
struct BOOT_Struct {
    uint8_t BootFlag;
    uint8_t Event_bit;
    uint32_t XmodeTimer;
    uint32_t XmodeRecNum;
    uint32_t XmodeCRC;
    uint32_t XmodeRecPge;
    uint32_t XmodeRceLstPN;
}BootStructINFO;

struct Wait_struct{
    uint8_t Wait_Cmd;
    uint8_t Cmd_Count;
}WaitTimeCount;

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
  HAL_UARTEx_ReceiveToIdle_DMA(&huart1,rx_buf,270);
  BootStructINFO.BootFlag = WAIT_CMD;
  norflash_init();
  printf("Press a within 3 seconds to enter sboot\r\n");
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
     HAL_Delay(100);
     if(BootStructINFO.BootFlag==WAIT_CMD)
     {
         WaitTimeCount.Wait_Cmd++;
         if(WaitTimeCount.Wait_Cmd%10==0)
         {
             printf("Press a within %1d seconds to enter sboot\r\n",3-(WaitTimeCount.Wait_Cmd/10));
         }
         if(WaitTimeCount.Wait_Cmd==30)//倒计时结束跳转到主分区
         {
             Load_SYS_User(SYS_START_ADDR);
         }
     }
     else if(BootStructINFO.BootFlag==INTO_CMD)//输出命令行
     {
         WaitTimeCount.Cmd_Count++;
         if(WaitTimeCount.Cmd_Count%50==0)
         {
             WaitTimeCount.Cmd_Count=0;
             printf("/***********************************************************/\r\n");
             printf("[A] Update the System\r\n");
             printf("[B] Run into the MainSys\r\n");
             printf("[C] Restart the Sysyem\r\n");
             printf("\r\n\r\n\r\n\r\n");
         }

     }
     else if(BootStructINFO.BootFlag==UPDATE)
     {
         if(BootStructINFO.Event_bit&EVENT_MSG_BIT)//这里是一个状态机的开始标志位，切换到UPdata模式后，先输出提示信息
         {
            printf("Warning:Cant quit before finished\r\n");
            printf("Please Use the XModem Dowload\r\n");
            BootStructINFO.Event_bit &= ~EVENT_MSG_BIT;
            BootStructINFO.Event_bit |= EVENT_C_BIT;
         }
         else if(BootStructINFO.Event_bit&EVENT_C_BIT)//跳转到发C状态
         {
             BootStructINFO.XmodeTimer++;
             if(BootStructINFO.XmodeTimer==2)
             {
                 BootStructINFO.XmodeTimer=0;
                 printf("C");
             }
         }
        else if(BootStructINFO.Event_bit&EVENT_FLASH_BIT)//如果接收完成使能了写入位开始写入数据
        {
            printf("ready write %d pack\r\n",BootStructINFO.XmodeRecNum);
            HAL_Delay(3000);
            eraseflash(SYS_START_PAG,SYS_PAGE);         //先对主程序部分的256-20页全部擦除
            printf("Erase Done\r\n");
            BootStructINFO.Event_bit&=~EVENT_FLASH_BIT; //把写入先失能，避免重复触发
            memset(Updatebuff,0,2048);                  //把updatebuf清零，后续要用这个做数据搬运桥梁
            for(uint16_t i=0;i<BootStructINFO.XmodeRecPge-1;i++)//这里我们先用一个for循环写入除最后一页外的其他页（因为最后一页有可能并不满2048个数据，所以要单独判断）
            {
                norflash_read(Updatebuff,0x00000000+i*PAGE_SIZE,PAGE_SIZE);
                writeflash(SYS_START_ADDR+i*PAGE_SIZE,(uint32_t *)Updatebuff,PAGE_SIZE);
            }
            if(BootStructINFO.XmodeRceLstPN==0)//如果接收的余出来的数据包为0即，恰好是放满整页的的（128byte的数据包）
            {
                norflash_read(Updatebuff,0x00000000+(BootStructINFO.XmodeRecPge-1)*PAGE_SIZE,PAGE_SIZE);//直接写一页
                writeflash(SYS_START_ADDR+(BootStructINFO.XmodeRecPge-1)*PAGE_SIZE,(uint32_t *)Updatebuff,PAGE_SIZE);
            }
            else//如果有余出来的
            {
                norflash_read(Updatebuff,0x00000000+(BootStructINFO.XmodeRecPge-1)*PAGE_SIZE,BootStructINFO.XmodeRceLstPN*128);//把余出来的单独读写
                writeflash(SYS_START_ADDR+(BootStructINFO.XmodeRecPge-1)*PAGE_SIZE,(uint32_t *)Updatebuff,BootStructINFO.XmodeRceLstPN*128);
            }
            printf("Write Finished\r\n");//写完以后打印
            HAL_Delay(2000);
            HAL_NVIC_SystemReset();//重启系统
        }
     }
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
    if(huart->Instance==USART1)
    {
        uint8_t data_leng = 270-__HAL_DMA_GET_COUNTER(&hdma_usart1_rx);//计算本次接收到的字符串个数
        if(BootStructINFO.BootFlag==WAIT_CMD)//如果在等a模式下
        {
            if(data_leng==1&&rx_buf[0]=='a')//接收到a
            {
                BootStructINFO.BootFlag=INTO_CMD;//进入输出控制台模式
                WaitTimeCount.Cmd_Count=49;
            }
        }
        else if(BootStructINFO.BootFlag==INTO_CMD)//如果在控制台模式下
        {
            if(data_leng==1)
            {
                switch(rx_buf[0])//收到以下字符
                {
                    case 'A':{BootStructINFO.BootFlag=UPDATE;BootStructINFO.Event_bit |= EVENT_MSG_BIT;}break;
                    case 'B':Load_SYS_User(SYS_START_ADDR);break;
                    case 'C':HAL_NVIC_SystemReset();break;
                    default:printf("cmd erro\r\n");break;
                }
            }
            else
            {
                printf("cmd erro\r\n");
            }
        }
        else if(BootStructINFO.BootFlag==UPDATE)                                                    //如果是更新模式
        {
            if(rx_buf[0]==0x01&&data_leng==133)                                                     //如果包头是0x01，包长度为133，则一包数据初步认为正确
            {
                BootStructINFO.Event_bit&=~EVENT_C_BIT;                                             //把发C事件标志位给清零，停止发C
                BootStructINFO.XmodeCRC= Xmodem_crc16(&rx_buf[3],128);                              //对128字节数据校验
                if(BootStructINFO.XmodeCRC==(rx_buf[131]*256+rx_buf[132]))                          //如果我们的CRC和数据包给的CRC一致，可以确认这是一包完全正确的数据
                {
                    BootStructINFO.XmodeRecNum+=1;                                                  //接收数+1，这里的接收数是接收数据包的个数，方便后期统计接收数据量
                    memcpy(&Updatebuff[0+128*((BootStructINFO.XmodeRecNum-1)%16)],&rx_buf[3],128);  //把中间的数据段给复制到UpdaeBuff中（这里我设置的数组大小为2048,也就是说16包数据才能填满一个缓冲区）
                    if((BootStructINFO.XmodeRecNum)%16==0)                                          //如果接收到16包数据，即此刻缓冲区已经满了
                    {
                        BootStructINFO.XmodeRecPge+=1;                                              //接收到的页数+1，也是方便后期进行数据搬运
                        norflash_write(Updatebuff,(BootStructINFO.XmodeRecPge-1)*2048,2048);        //把接收到的数据写入Norflash里（外置Norflash）
                        
                    }
                    printf("\x06");                                                                 //发送ACK
                }
            else
                {
                    printf("\x15");                                                                 //如果不是数据正确的情况，就发送NACK   
                }
            }
            else if(data_leng==1&&rx_buf[0]==0x04)                                                  //如果接收到长度为1，且为0x04的数据，则表明已经完成一次协议通讯了
            {
                printf("\x06");                                                                     //发送一个ACK高职
                if(BootStructINFO.XmodeRecNum%16!=0)                                                //这里，由于我们接收到的数据量不可能是2048的整数倍，可能会多出几包数据这里进行一个判断
                {
                    BootStructINFO.XmodeRceLstPN = BootStructINFO.XmodeRecNum%16;                   //如果这里把多出来的数据包数量存起来，方便后期搬运
                    norflash_write(Updatebuff,BootStructINFO.XmodeRecPge*2048,(BootStructINFO.XmodeRceLstPN)*128);  //把多出来的数据写入Norflash
                    BootStructINFO.XmodeRecPge+=1;                                                  //因为我们是新开一页写的，这里不要忘记页数+1
                }
                BootStructINFO.Event_bit|=EVENT_FLASH_BIT;                                          //使能FLASH更新标志事件
            }
            else
            {
                printf("\x15");
            }
        }
        else
        {
            printf("\x15");
        }
        memset(rx_buf,0,270);
        HAL_UARTEx_ReceiveToIdle_DMA(&huart1,(uint8_t *)rx_buf,270);
    }
}


void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    printf("error now\r\n");
}


/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
