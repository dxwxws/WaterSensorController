#include "eeprom.h"

#define _nop_() __asm__("nop")

eeprom_cfg_t g_cfg; // 全局配置结构体变量

uint16_t g_t_air  = DEFAULT_T_AIR;
uint16_t g_t_full = DEFAULT_T_FULL;

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

    IAP_CONTR = 0x80; // 打开 IAP 读模式
    IAP_TPS   = 12;   // 设置 IAP 时序参数
    IAP_CMD   = 0x01; // 设置读命令
    IAP_ADDRL = addr;       // Set low byte of address
    IAP_ADDRH = addr >> 8;         // Set high byte of address
    IAP_TRIG  = 0x5A; // Trigger sequence part 1
    IAP_TRIG  = 0xA5; // Trigger sequence part 2
    _nop_();
    dat = IAP_DATA; // Read data
    IapIdle(); // Return to idle state

    return dat;
}
void IapErase(uint16_t addr)

{
    EA = 0;
    IAP_CONTR = 0x82;      // IAPEN=1, SWBS=1
    IAP_TPS   = 12;        // 11.0592 MHz
    IAP_CMD   = 0x03;      // ERASE
    IAP_ADDRH = addr >> 8;
    IAP_ADDRL = addr & 0xFF;
    IAP_TRIG  = 0x5A;
    IAP_TRIG  = 0xA5;
    _nop_();
    _nop_();
    _nop_();
    IAP_CONTR = 0;
    EA = 1;
}
void IapWrite(uint16_t addr, uint8_t dat)

{
    EA = 0;
    IAP_CONTR = 0x82;      // IAPEN=1, SWBS=1
    IAP_TPS   = 12;        // 11.0592 MHz
    IAP_CMD   = 0x02;      // WRITE
    IAP_ADDRH = addr >> 8;
    IAP_ADDRL = addr & 0xFF;
    IAP_DATA  = dat;
    IAP_TRIG  = 0x5A;
    IAP_TRIG  = 0xA5;
    _nop_();
    _nop_();
    _nop_();
    IAP_CONTR = 0;
    EA = 1;
}

// 初始化配置：检查EEPROM是否已初始化，如未初始化则写入默认值
void EEPROM_InitConfig(void)
{
    g_cfg.slave_id = IapRead(0x0000);
    g_cfg.baudrate = IapRead(0x0001);
    g_cfg.magic    = IapRead(0x0002);

    g_cfg.t_air  = IapRead(0x0003) | (IapRead(0x0004) << 8);
    g_cfg.t_full = IapRead(0x0005) | (IapRead(0x0006) << 8);

    if (g_cfg.magic != EEPROM_MAGIC_VALUE) {
        g_cfg.slave_id = DEFAULT_SLAVE_ID;
        g_cfg.baudrate = DEFAULT_BAUDRATE_INDEX;
        g_cfg.t_air    = DEFAULT_T_AIR;
        g_cfg.t_full   = DEFAULT_T_FULL;
        g_cfg.magic    = EEPROM_MAGIC_VALUE;
        EEPROM_SaveAll();
    } else {
        uint8_t need_save = 0;

        if (g_cfg.slave_id == 0 || g_cfg.slave_id > 247) {
            g_cfg.slave_id = DEFAULT_SLAVE_ID;
            need_save = 1;
        }

        if (g_cfg.baudrate > 7) {
            g_cfg.baudrate = DEFAULT_BAUDRATE_INDEX;
            need_save = 1;
        }

        if (g_cfg.t_air == 0xFFFF) {
            g_cfg.t_air = DEFAULT_T_AIR;
            need_save = 1;
        }

        if (g_cfg.t_full == 0xFFFF || g_cfg.t_full <= g_cfg.t_air) {
            g_cfg.t_full = DEFAULT_T_FULL;
            need_save = 1;
        }

        if (need_save) {
            EEPROM_SaveAll();
        }
    }

    // 同步运行时参数
    g_slave_id = g_cfg.slave_id;
    g_baudrate_index = g_cfg.baudrate;
    g_t_air = g_cfg.t_air;
    g_t_full = g_cfg.t_full;
}

void EEPROM_SaveAll(void)
{
    IapErase(0x0000);
    // 擦除后需要延迟等待操作完成
    {
        uint16_t i;
        for (i = 0; i < 100; i++);
    }

    IapWrite(0x0000, g_cfg.slave_id);
    IapWrite(0x0001, g_cfg.baudrate);
    IapWrite(0x0002, g_cfg.magic);

    IapWrite(0x0003, g_cfg.t_air & 0xFF);
    IapWrite(0x0004, g_cfg.t_air >> 8);

    IapWrite(0x0005, g_cfg.t_full & 0xFF);
    IapWrite(0x0006, g_cfg.t_full >> 8);
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
