#ifndef __CRC16_H__
#define __CRC16_H__

#include <stdint.h>

uint16_t Modbus_CRC16(uint8_t *buf, uint16_t len);

#endif 