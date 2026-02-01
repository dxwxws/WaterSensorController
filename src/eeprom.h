#ifndef EEPROM_H
#define EEPROM_H
#include "stdint.h"
#include "reg52.h"

// EEPROM 存储地址定义
#define EEPROM_ADDR_SLAVE_ID    0x0000   // 从机地址
#define EEPROM_ADDR_BAUDRATE    0x0001   // 波特率索引
#define EEPROM_ADDR_MAGIC       0x0002   // 魔法数字，用于判断是否首次使用

#define EEPROM_ADDR_T_AIR_L     0x0003  // 空气中的计数值低字节
#define EEPROM_ADDR_T_AIR_H     0x0004  // 空气中的计数值高字节
#define EEPROM_ADDR_T_FULL_L    0x0005  // 满水位计数值低字节
#define EEPROM_ADDR_T_FULL_H    0x0006  // 满水位计数值高字节

#define DEFAULT_T_AIR           7
#define DEFAULT_T_FULL          98

#define EEPROM_MAGIC_VALUE      0x50     // 魔法数字值

// 默认配置
#define DEFAULT_SLAVE_ID        0x01
#define DEFAULT_BAUDRATE_INDEX  6        // 默认115200bps

typedef struct { 
    uint8_t slave_id; 
    uint8_t baudrate; 
    uint8_t magic; 
    uint16_t t_air; 
    uint16_t t_full; 
} eeprom_cfg_t;

extern eeprom_cfg_t g_cfg;

/***************************/
void IapIdle(void);
uint8_t IapRead(uint16_t addr);     // 读取EEPROM数据
void IapWrite(uint16_t addr, uint8_t dat);      // 写入EEPROM数据
void IapErase(uint16_t addr);       // 擦除EEPROM扇区
void EEPROM_SaveAll(void);    // 保存所有配置到EEPROM
/***************************/

// 配置读写函数
void EEPROM_SaveSlaveID(uint8_t id);    // 保存从机地址
void EEPROM_SaveBaudrate(uint8_t index);    // 保存波特率索引
void EEPROM_InitConfig(void);       // 初始化配置

extern uint8_t g_slave_id;      // 全局从机地址变量
extern uint8_t g_baudrate_index;    // 全局波特率索引变量
extern uint16_t g_t_air;
extern uint16_t g_t_full;

#endif