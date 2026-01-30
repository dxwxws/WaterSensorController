#ifndef _18B20_H_
#define _18B20_H_

#include "reg52.h"
#include <stdint.h>

__sbit __at(0xB0+2) DQ; // P3.2 连接到 DS18B20 的数据线

void DS18B20_ConfigPort(void);
void DelayUs(uint16_t us);
uint8_t DS18B20_Init(void);      // 返回0表示成功，1表示失败
void DS18B20_WriteByte(uint8_t byte);
uint8_t DS18B20_ReadByte(void);
uint8_t Ds18b20ConvertTemp();    // Ds18b20温度转换，返回0表示成功
uint16_t Ds18b20ReadTemp();      // Ds18b20读取温度，返回0xFFFF表示失败

#endif // !_18B20_H_