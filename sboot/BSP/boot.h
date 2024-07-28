#ifndef __BOOT_H
#define __BOOT_H

#include "main.h"

typedef void (*load_sys)(void);

__ASM void MSR_SP(uint32_t addr);

void Load_SYS_User(uint32_t addr);

void BOOT_Clear(void);


#endif
