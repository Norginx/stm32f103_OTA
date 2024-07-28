#ifndef _OLED_H
#define _OLED_H
#include "main.h"


#define DATA    1
#define CMD     0

extern uint8_t back_button;

void IIC_Start(void);
void IIC_Stop(void);
uint8_t Wait_ACK(void);
void ACK(void);
void NACK(void);
void sent_byte(uint8_t byte);
uint8_t read_byte(uint8_t ack);

void OLED_ShowString(uint8_t x,uint8_t y,uint8_t *chr,uint8_t size1,uint8_t mode);
void oled_init(void);
void oled_display_on(void);
void oled_drawpoint(uint8_t x,uint8_t y,uint8_t t);
void oled_clear(void);
void oled_write(uint8_t data ,uint8_t mode);
void oled_refresh(void);
void OLED_ShowChar(uint8_t x,uint8_t y,uint8_t chr,uint8_t size1,uint8_t mode);
void OLED_ShowNum(uint8_t x,uint8_t y,uint32_t num,uint8_t len,uint8_t size1,uint8_t mode);
#endif
