// #include "STC8G.h"
#include "reg52.h"
#include "delay.h"
#include "cap.h"

// P5.4 = RC 控制脚（充放电）
// P5.5 = RC 检测脚（电压门限检测）

// typedef struct {    // 水位点结构体
//     uint16_t t;
//     uint8_t  level; // 0~100%
// } wl_point_t;

// const wl_point_t wl_table[] = { // 水位点表
//     {  2,   0 },  // 空气
//     {  6,   5 },  // 刚接触
//     { 24,  50 },  // 半浸
//     { 40, 100 },  // 全浸
// };

void cap_init(void)  // 初始化电容测量模块
{
    // P5.4 推挽输出（初始用于放电）
    // P5M0 |=  0x10;
    // P5M1 &= ~0x10;

    // P5.5 输入
    // P5M0 &= ~0x20;
    // P5M1 &= ~0x20;
    P5M0 = 0x10; 
    P5M1 = 0x20; 

    // 不再初始化定时器，改用软件计数测量
}

uint16_t cap_measure(void)  // 测量一次电容充电时间，单位：相对值
{
    uint16_t count = 0;

    // 1. 放电阶段：P5.4 输出 0
    P5M0 = 0x10; 
    P5M1 = 0x20; 

    // P5M0 |=  0x10;   // 推挽输出
    // P5M1 &= ~0x10;
    P54  = 0;
    delay_us(50);

    // 2. P5.5 输入（检测脚）
    // P5M0 &= ~0x20;
    // P5M1 &= ~0x20;

   // 3. 开始充电：P5.4 改为高阻输入（开漏）
    P5M0 = 0x00; 
    P5M1 = 0x30; 

    // P5M0 &= ~0x10;
    // P5M1 |=  0x10;

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
    const uint16_t t_air  = 7;   // 空气中的计数值
    const uint16_t t_full = 98;  // 完全浸入水中的计数值

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
