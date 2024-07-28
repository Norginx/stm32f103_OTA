#include "boot.h"
#include "usart.h"
#include "spi.h"

load_sys Load_SYS;


__ASM void MSR_SP(uint32_t addr)//��ഫ�Σ�addr�ǵ�һ��������ֱֵ�Ӹ���r0�Ĵ���
{
    MSR MSP,r0//����spָ��
    BX r14  //�ָ��ֳ�
}


void Load_SYS_User(uint32_t addr)
{
    if((*(uint32_t *)addr>0x20000000-1)&&((*(uint32_t *)addr<(0x20000000+0x10000))))//����ָ���Ƿ��ں������䣨RAM�У�
    {
        MSR_SP(*(uint32_t *)addr);//sp��ת
        Load_SYS = (load_sys)*(uint32_t *)(addr+4);//��ַǿתΪ����������ת���Ǹ�λReset_Handler�ص�����
        BOOT_Clear();
        Load_SYS();
    }
}

void BOOT_Clear(void)//��ʼ��������
{
    HAL_UART_DeInit(&huart1);
    HAL_SPI_DeInit(&g_spi2_handler);
    HAL_DeInit();
}
