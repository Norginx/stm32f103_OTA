#include "boot.h"
#include "usart.h"
#include "spi.h"

load_sys Load_SYS;


__ASM void MSR_SP(uint32_t addr)//汇编传参，addr是第一个变量，值直接给到r0寄存器
{
    MSR MSP,r0//设置sp指针
    BX r14  //恢复现场
}


void Load_SYS_User(uint32_t addr)
{
    if((*(uint32_t *)addr>0x20000000-1)&&((*(uint32_t *)addr<(0x20000000+0x10000))))//看看指针是否在合理区间（RAM中）
    {
        MSR_SP(*(uint32_t *)addr);//sp跳转
        Load_SYS = (load_sys)*(uint32_t *)(addr+4);//地址强转为函数，这里转的是复位Reset_Handler回调函数
        BOOT_Clear();
        Load_SYS();
    }
}

void BOOT_Clear(void)//初始化的清理
{
    HAL_UART_DeInit(&huart1);
    HAL_SPI_DeInit(&g_spi2_handler);
    HAL_DeInit();
}
