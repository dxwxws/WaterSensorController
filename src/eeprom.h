#ifndef EEPROM_H
#define EEPROM_H
#include "stdint.h"
#include "reg52.h"

// EEPROM 存储地址定义
#define EEPROM_ADDR_SLAVE_ID    0x0000   // 从机地址
#define EEPROM_ADDR_BAUDRATE    0x0001   // 波特率索引
#define EEPROM_ADDR_MAGIC       0x0002   // 魔法数字，用于判断是否首次使用
#define EEPROM_MAGIC_VALUE      0x50     // 魔法数字值

// 默认配置
#define DEFAULT_SLAVE_ID        0x01
#define DEFAULT_BAUDRATE_INDEX  6        // 默认115200bps

/*********本地常量声明******************/
/*********本地变量声明******************/
void IapIdle(void);
uint8_t IapRead(uint16_t addr);
void IapWrite(uint16_t addr, uint8_t dat);
void IapErase(uint16_t addr);

// 配置读写函数
void EEPROM_SaveSlaveID(uint8_t id);
uint8_t EEPROM_ReadSlaveID(void);
void EEPROM_SaveBaudrate(uint8_t index);
uint8_t EEPROM_ReadBaudrate(void);
void EEPROM_InitConfig(void);

extern uint8_t g_slave_id;
extern uint8_t g_baudrate_index;

#endif