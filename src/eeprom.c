#include "eeprom.h"

#define _nop_() __asm__("nop")

// 全局配置变量（RAM中）
uint8_t g_slave_id = DEFAULT_SLAVE_ID;
uint8_t g_baudrate_index = DEFAULT_BAUDRATE_INDEX;

void IapIdle()
{
    IAP_CONTR = 0x00; // Disable IAP
    IAP_CMD = 0x00;  // Clear command
    IAP_TRIG = 0x00; // Clear trigger
    IAP_ADDRH = 0x80; // 将地址设置到非EEPROM区域
    IAP_ADDRL = 0x00; // Clear address low byte
}

uint8_t IapRead(uint16_t addr)
{
    uint8_t dat;

    IAP_CONTR = 0x80; // Enable IAP with read mode
    IAP_TPS   = 12;   // Set IAP timing parameter
    IAP_CMD   = 0x01; // Set read command
    IAP_ADDRL = addr;       // Set low byte of address
    IAP_ADDRH = addr >> 8;         // Set high byte of address
    IAP_TRIG  = 0x5A; // Trigger sequence part 1
    IAP_TRIG  = 0xA5; // Trigger sequence part 2
    _nop_();
    dat = IAP_DATA; // Read data
    IapIdle(); // Return to idle state

    return dat;
}

void IapWrite(uint16_t addr, uint8_t dat)
{
    IAP_CONTR = 0x80; // Enable IAP with write mode
    IAP_TPS   = 12;   // Set IAP timing parameter
    IAP_CMD   = 0x02; // Set write command
    IAP_ADDRL = addr;       // Set low byte of address
    IAP_ADDRH = addr >> 8;         // Set high byte of address
    IAP_DATA  = dat;  // Load data to be written
    IAP_TRIG  = 0x5A; // Trigger sequence part 1
    IAP_TRIG  = 0xA5; // Trigger sequence part 2
    _nop_();
    IapIdle(); // Return to idle state
}

void IapErase(uint16_t addr)
{
    IAP_CONTR = 0x80; // Enable IAP with erase mode
    IAP_TPS   = 12;   // Set IAP timing parameter
    IAP_CMD   = 0x03; // Set erase command
    IAP_ADDRL = addr;       // Set low byte of address
    IAP_ADDRH = addr >> 8;         // Set high byte of address
    IAP_TRIG  = 0x5A; // Trigger sequence part 1
    IAP_TRIG  = 0xA5; // Trigger sequence part 2
    _nop_();
    IapIdle(); // Return to idle state
}

// 初始化配置：检查EEPROM是否已初始化，如未初始化则写入默认值
void EEPROM_InitConfig(void)
{
    uint8_t magic = IapRead(EEPROM_ADDR_MAGIC);
    
    if (magic != EEPROM_MAGIC_VALUE)
    {
        // 首次使用，写入默认配置
        IapWrite(EEPROM_ADDR_SLAVE_ID, DEFAULT_SLAVE_ID);
        IapWrite(EEPROM_ADDR_BAUDRATE, DEFAULT_BAUDRATE_INDEX);
        IapWrite(EEPROM_ADDR_MAGIC, EEPROM_MAGIC_VALUE);
    }
    
    // 读取配置到RAM
    g_slave_id = IapRead(EEPROM_ADDR_SLAVE_ID);
    g_baudrate_index = IapRead(EEPROM_ADDR_BAUDRATE);
    
    // 有效性检查
    if (g_slave_id == 0 || g_slave_id > 247)
        g_slave_id = DEFAULT_SLAVE_ID;
    
    if (g_baudrate_index > 7)
        g_baudrate_index = DEFAULT_BAUDRATE_INDEX;
}

// 保存从机地址
void EEPROM_SaveSlaveID(uint8_t id)
{
    if (id > 0 && id <= 247)  // Modbus有效地址范围
    {
        // 读取当前波特率配置
        uint8_t current_baudrate = IapRead(EEPROM_ADDR_BAUDRATE);
        uint8_t current_magic = IapRead(EEPROM_ADDR_MAGIC);
        
        // 擦除扇区（从地址0开始）
        IapErase(0x0000);
        
        // 重新写入所有配置
        IapWrite(EEPROM_ADDR_SLAVE_ID, id);
        IapWrite(EEPROM_ADDR_BAUDRATE, current_baudrate);
        IapWrite(EEPROM_ADDR_MAGIC, current_magic);
        
        g_slave_id = id;
    }
}

// 读取从机地址
uint8_t EEPROM_ReadSlaveID(void)
{
    return g_slave_id;
}

// 保存波特率索引
void EEPROM_SaveBaudrate(uint8_t index)
{
    if (index <= 7)  // 有效波特率索引：0-7
    {
        // 读取当前从机地址和魔法数字
        uint8_t current_id = IapRead(EEPROM_ADDR_SLAVE_ID);
        uint8_t current_magic = IapRead(EEPROM_ADDR_MAGIC);
        
        // 擦除扇区（从地址0开始）
        IapErase(0x0000);
        
        // 重新写入所有配置
        IapWrite(EEPROM_ADDR_SLAVE_ID, current_id);
        IapWrite(EEPROM_ADDR_BAUDRATE, index);
        IapWrite(EEPROM_ADDR_MAGIC, current_magic);
        
        g_baudrate_index = index;
    }
}

// 读取波特率索引
uint8_t EEPROM_ReadBaudrate(void)
{
    return g_baudrate_index;
}