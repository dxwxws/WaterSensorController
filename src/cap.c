// #include "STC8G.h"
#include "reg52.h"
#include "delay.h"
#include "cap.h"
#include "eeprom.h"

extern eeprom_cfg_t g_cfg;

void cap_init(void)  // 初始化电容测量模块
{
    // P5.4 推挽输出（初始用于放电）
    // P5.5 输入
    
    P5M0 = 0x10; 
    P5M1 = 0x20; 

    // 不再初始化定时器，改用软件计数测量
}

uint16_t cap_measure(void)  // 测量一次电容充电时间，单位：相对值
{
    uint16_t count = 0;

    // 1. 放电阶段：P5.4 输出 0
    P5M0 = 0x10; 
    P5M1 = 0x20;    // 推挽输出

    P54  = 0;
    delay_us(50);

    // 2. P5.5 输入（检测脚）

   // 3. 开始充电：P5.4 改为高阻输入（开漏）
    P5M0 = 0x00; 
    P5M1 = 0x30; 

    // 4. 软件计数等待 P5.5 变为 1（改用软件计数，避免定时器冲突）
    while (P55 == 0)
    {
        count++;
        if (count >= 0xFFFE) {
            return 0xFFFF;   // 超时
        }
    }

    return count;
}

uint8_t water_level_get_percent(uint16_t t) // 根据时间获取水位百分比
{
    const uint16_t t_air  = g_cfg.t_air;   // 空气中的计数值
    const uint16_t t_full = g_cfg.t_full;  // 完全浸入水中的计数值

    if (t <= t_air)  return 0;
    if (t >= t_full) return 100;

    return (uint32_t)(t - t_air) * 100 / (t_full - t_air);
}

uint16_t cap_measure_avg(uint8_t n) // 测量 n 次并取平均
{
    uint32_t sum = 0;
    for (uint8_t i = 0; i < n; i++)
        sum += cap_measure();
    return sum / n;
}
