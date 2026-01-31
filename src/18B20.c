#include "18B20.h"

// SDCC内联汇编nop指令
#define NOP() __asm nop __endasm

void DS18B20_ConfigPort(void)
{
    P3M0 = 0x0c; //设置 P3.2 为开漏输出，P3.3为推挽输出，用于输出电磁阀控制。
    P3M1 = 0x04; 
    DQ = 1;    //初始化总线为高电平
}

// 微秒级延时（假设11.0592MHz晶振）
// 对于11.0592MHz，1机器周期 = 12/11.0592MHz ≈ 1.085us
void DelayUs(uint16_t us)
{
    // 每次循环约1us
    while (us--)
    {
        NOP();
    }
}

uint8_t DS18B20_Init(void)
{
    uint8_t ack = 0;
    uint16_t timeout = 200;
    
    DQ = 1;              // 拉高总线
    DelayUs(5);         // 延时5微秒
    DQ = 0;              // 拉低总线
    DelayUs(480);       // 延时480微秒
    DQ = 1;              // 释放总线
    DelayUs(60);        // 延时60微秒
    
    // 检测从机是否有响应（存在从机会拉低总线）
    while (DQ && timeout--)
    {
        DelayUs(1);
    }
    
    if (timeout == 0)
    {
        return 1; // 初始化失败，没有检测到DS18B20
    }
    
    ack = !DQ; // 如果DQ为低，表示有响应
    DelayUs(420);       // 延时420微秒，等待复位完成
    
    return ack ? 0 : 1; // 返回0表示成功，1表示失败
}

void DS18B20_WriteByte(uint8_t byte)
{
    uint8_t i;
    for (i = 0; i < 8; i++)
    {
        DQ = 0;              // 拉低总线启动时隙
        DelayUs(2);          // 延时2微秒
        
        if (byte & 0x01)     // 写1
        {
            DQ = 1;          // 快速释放总线
        }
        // 写0则保持拉低
        
        DelayUs(60);         // 等待时隙结束
        DQ = 1;              // 确保释放总线
        DelayUs(2);          // 恢复时间
        
        byte >>= 1;          // 移到下一位
    }
}
uint8_t DS18B20_ReadByte(void)
{
    uint8_t byte = 0;
    uint8_t i;

    for (i = 0; i < 8; i++)
    {
        DQ = 0;              // 拉低总线启动读时隙
        DelayUs(2);          // 延时2微秒
        DQ = 1;              // 释放总线
        DelayUs(12);         // 等待12us后采样
        
        if (DQ)              // 读取数据位
        {
            byte |= (1 << i); // 设置对应位
        }
        
        DelayUs(50);         // 等待时隙结束
    }

    return byte;
}
uint8_t Ds18b20ConvertTemp()
{
    if (DS18B20_Init())
    {
        return 1; // 初始化失败
    }
    DS18B20_WriteByte(0xCC); // 跳过ROM
    DS18B20_WriteByte(0x44); // 温度转换命令
    return 0; // 成功
}
uint16_t Ds18b20ReadTemp()
{
    uint8_t tempL, tempH;
    uint16_t temp;
    
    if (DS18B20_Init())
    {
        return 0xFFFF; // 初始化失败，返回错误值
    }
    
    DS18B20_WriteByte(0xCC); // 跳过ROM
    DS18B20_WriteByte(0xBE); // 读暂存器命令
    tempL = DS18B20_ReadByte();
    tempH = DS18B20_ReadByte();
    temp = (tempH << 8) | tempL;
    return temp;
}