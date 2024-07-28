#include "flash_ctrl.h"



uint8_t eraseflash(uint16_t addr,uint16_t nbPages)
{
    unsigned int pageError = 0;
    uint32_t pageAddress = 0x08000000+addr*2048;
    FLASH_EraseInitTypeDef eraseInit;
    eraseInit.TypeErase = FLASH_TYPEERASE_PAGES;
    eraseInit.PageAddress = pageAddress;
    eraseInit.Banks = FLASH_BASE;
    eraseInit.NbPages = nbPages;
    HAL_FLASH_Unlock();
    if(HAL_FLASHEx_Erase(&eraseInit,&pageError) != HAL_OK)
    {
        HAL_FLASH_Lock();
        return 1;
    }
    HAL_FLASH_Lock();
    return 0;

}


void writeflash(uint32_t saddr,uint32_t *wdata,uint32_t wnum)
{
    HAL_FLASH_Unlock();
    while(wnum)
    {
        HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD,saddr,*wdata);
        wnum -=4;
        saddr+=4;
        wdata++;
    }
    HAL_FLASH_Lock();
}
