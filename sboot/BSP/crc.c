#include "crc.h"


uint16_t Xmodem_crc16(uint8_t *data,uint16_t datalen)
{
    uint8_t i;
    uint16_t Crcinit = 0x0000;
    uint16_t Crcipoly = 0x1021;
    while(datalen--)
    {
        Crcinit = (*data<<8)^Crcinit;
        for(i=0;i<8;i++)
        {
            if(Crcinit&0x8000)
            {
                Crcinit = (Crcinit<<1)^Crcipoly;
            }
            else
            {
                Crcinit = (Crcinit<<1);
            }
        }
        data++;
    }
    return  Crcinit;
    
}
