#include "crc16.h"

uint16_t Modbus_CRC16(uint8_t *buf, uint16_t len)
{
    uint16_t crc = 0xFFFF;
    uint16_t i;

    while (len--)
    {
        crc ^= *buf++;
        for (i = 0; i < 8; i++)
        {
            if (crc & 0x0001)
                crc = (crc >> 1) ^ 0xA001;
            else 
                crc >>= 1;
        }
    }
    return crc;
}