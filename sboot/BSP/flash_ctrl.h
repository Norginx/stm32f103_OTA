#ifndef __flash_ctrl_h
#define __flash_ctrl_h
#include "main.h"



uint8_t eraseflash(uint16_t addr,uint16_t nbPages);
void writeflash(uint32_t saddr,uint32_t *wdata,uint32_t wnum);

#endif
