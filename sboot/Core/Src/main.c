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
         if(WaitTimeCount.Wait_Cmd==30)//����ʱ������ת��������
         {
             Load_SYS_User(SYS_START_ADDR);
         }
     }
     else if(BootStructINFO.BootFlag==INTO_CMD)//���������
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
         if(BootStructINFO.Event_bit&EVENT_MSG_BIT)//������һ��״̬���Ŀ�ʼ��־λ���л���UPdataģʽ���������ʾ��Ϣ
         {
            printf("Warning:Cant quit before finished\r\n");
            printf("Please Use the XModem Dowload\r\n");
            BootStructINFO.Event_bit &= ~EVENT_MSG_BIT;
            BootStructINFO.Event_bit |= EVENT_C_BIT;
         }
         else if(BootStructINFO.Event_bit&EVENT_C_BIT)//��ת����C״̬
         {
             BootStructINFO.XmodeTimer++;
             if(BootStructINFO.XmodeTimer==2)
             {
                 BootStructINFO.XmodeTimer=0;
                 printf("C");
             }
         }
        else if(BootStructINFO.Event_bit&EVENT_FLASH_BIT)//����������ʹ����д��λ��ʼд������
        {
            printf("ready write %d pack\r\n",BootStructINFO.XmodeRecNum);
            HAL_Delay(3000);
            eraseflash(SYS_START_PAG,SYS_PAGE);         //�ȶ������򲿷ֵ�256-20ҳȫ������
            printf("Erase Done\r\n");
            BootStructINFO.Event_bit&=~EVENT_FLASH_BIT; //��д����ʧ�ܣ������ظ�����
            memset(Updatebuff,0,2048);                  //��updatebuf���㣬����Ҫ����������ݰ�������
            for(uint16_t i=0;i<BootStructINFO.XmodeRecPge-1;i++)//������������һ��forѭ��д������һҳ�������ҳ����Ϊ���һҳ�п��ܲ�����2048�����ݣ�����Ҫ�����жϣ�
            {
                norflash_read(Updatebuff,0x00000000+i*PAGE_SIZE,PAGE_SIZE);
                writeflash(SYS_START_ADDR+i*PAGE_SIZE,(uint32_t *)Updatebuff,PAGE_SIZE);
            }
            if(BootStructINFO.XmodeRceLstPN==0)//������յ�����������ݰ�Ϊ0����ǡ���Ƿ�����ҳ�ĵģ�128byte�����ݰ���
            {
                norflash_read(Updatebuff,0x00000000+(BootStructINFO.XmodeRecPge-1)*PAGE_SIZE,PAGE_SIZE);//ֱ��дһҳ
                writeflash(SYS_START_ADDR+(BootStructINFO.XmodeRecPge-1)*PAGE_SIZE,(uint32_t *)Updatebuff,PAGE_SIZE);
            }
            else//������������
            {
                norflash_read(Updatebuff,0x00000000+(BootStructINFO.XmodeRecPge-1)*PAGE_SIZE,BootStructINFO.XmodeRceLstPN*128);//��������ĵ�����д
                writeflash(SYS_START_ADDR+(BootStructINFO.XmodeRecPge-1)*PAGE_SIZE,(uint32_t *)Updatebuff,BootStructINFO.XmodeRceLstPN*128);
            }
            printf("Write Finished\r\n");//д���Ժ��ӡ
            HAL_Delay(2000);
            HAL_NVIC_SystemReset();//����ϵͳ
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
        uint8_t data_leng = 270-__HAL_DMA_GET_COUNTER(&hdma_usart1_rx);//���㱾�ν��յ����ַ�������
        if(BootStructINFO.BootFlag==WAIT_CMD)//����ڵ�aģʽ��
        {
            if(data_leng==1&&rx_buf[0]=='a')//���յ�a
            {
                BootStructINFO.BootFlag=INTO_CMD;//�����������̨ģʽ
                WaitTimeCount.Cmd_Count=49;
            }
        }
        else if(BootStructINFO.BootFlag==INTO_CMD)//����ڿ���̨ģʽ��
        {
            if(data_leng==1)
            {
                switch(rx_buf[0])//�յ������ַ�
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
        else if(BootStructINFO.BootFlag==UPDATE)                                                    //����Ǹ���ģʽ
        {
            if(rx_buf[0]==0x01&&data_leng==133)                                                     //�����ͷ��0x01��������Ϊ133����һ�����ݳ�����Ϊ��ȷ
            {
                BootStructINFO.Event_bit&=~EVENT_C_BIT;                                             //�ѷ�C�¼���־λ�����㣬ֹͣ��C
                BootStructINFO.XmodeCRC= Xmodem_crc16(&rx_buf[3],128);                              //��128�ֽ�����У��
                if(BootStructINFO.XmodeCRC==(rx_buf[131]*256+rx_buf[132]))                          //������ǵ�CRC�����ݰ�����CRCһ�£�����ȷ������һ����ȫ��ȷ������
                {
                    BootStructINFO.XmodeRecNum+=1;                                                  //������+1������Ľ������ǽ������ݰ��ĸ������������ͳ�ƽ���������
                    memcpy(&Updatebuff[0+128*((BootStructINFO.XmodeRecNum-1)%16)],&rx_buf[3],128);  //���м�����ݶθ����Ƶ�UpdaeBuff�У����������õ������СΪ2048,Ҳ����˵16�����ݲ�������һ����������
                    if((BootStructINFO.XmodeRecNum)%16==0)                                          //������յ�16�����ݣ����˿̻������Ѿ�����
                    {
                        BootStructINFO.XmodeRecPge+=1;                                              //���յ���ҳ��+1��Ҳ�Ƿ�����ڽ������ݰ���
                        norflash_write(Updatebuff,(BootStructINFO.XmodeRecPge-1)*2048,2048);        //�ѽ��յ�������д��Norflash�����Norflash��
                        
                    }
                    printf("\x06");                                                                 //����ACK
                }
            else
                {
                    printf("\x15");                                                                 //�������������ȷ��������ͷ���NACK   
                }
            }
            else if(data_leng==1&&rx_buf[0]==0x04)                                                  //������յ�����Ϊ1����Ϊ0x04�����ݣ�������Ѿ����һ��Э��ͨѶ��
            {
                printf("\x06");                                                                     //����һ��ACK��ְ
                if(BootStructINFO.XmodeRecNum%16!=0)                                                //����������ǽ��յ�����������������2048�������������ܻ������������������һ���ж�
                {
                    BootStructINFO.XmodeRceLstPN = BootStructINFO.XmodeRecNum%16;                   //�������Ѷ���������ݰ�������������������ڰ���
                    norflash_write(Updatebuff,BootStructINFO.XmodeRecPge*2048,(BootStructINFO.XmodeRceLstPN)*128);  //�Ѷ����������д��Norflash
                    BootStructINFO.XmodeRecPge+=1;                                                  //��Ϊ�������¿�һҳд�ģ����ﲻҪ����ҳ��+1
                }
                BootStructINFO.Event_bit|=EVENT_FLASH_BIT;                                          //ʹ��FLASH���±�־�¼�
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
