#include "uart.h"
#include "18B20.h"
#include "modbus.h"
#include "eeprom.h"
#include "cap.h"

// DS18B20 状态机
typedef enum {
    DS18B20_IDLE,           // 空闲状态
    DS18B20_CONVERTING,     // 正在转换温度
    DS18B20_READY           // 转换完成，可以读取
} DS18B20_State;

volatile DS18B20_State ds18b20_state = DS18B20_IDLE;
volatile uint16_t ds18b20_timer = 0;  // 转换计时器（ms）
uint16_t g_temperature = 0;            // 存储温度值（全局变量）

// 水位测量定时器（减少测量频率，避免阻塞）
volatile uint16_t cap_measure_timer = 0;
#define CAP_MEASURE_INTERVAL_MS  250  // 每250ms测量一次水位（更快响应）

// 水位滤波器（滑动平均）
#define WATER_LEVEL_FILTER_SIZE  8  // 8点滤波，总响应时间2秒
uint16_t water_level_filter[WATER_LEVEL_FILTER_SIZE] = {0};
uint8_t water_level_filter_index = 0;

//--------------------- 定时器0初始化 ---------------------
void Timer0_Init_1ms(void)        //1毫秒@11.0592MHz
{
    AUXR |= 0x80;             //定时器时钟1T模式
    TMOD &= 0xF0;             // 清除 Timer0 的模式位
    TMOD |= 0x01;             // 设置 Timer0 为模式1（16位定时器）
    // 11.0592MHz, 1T 模式下，1ms 重装值：65536 - 11059 = 54477 => 0xD4CD
    TL0 = 0xCD;               //设置定时初值
    TH0 = 0xD4;               //设置定时初值
    TF0 = 0;                  //清除TF0标志
    TR0 = 1;                  //定时器0开始计时

    ET0 = 1;                  // 允许定时器0中断
    EA  = 1;                  // 允许总中断
}


// Timer2 中断向量号要按 STC8G 手册来，这里先假定为 12
void Timer0_ISR(void) interrupt 1
{
    // 重装定时器初值（模式1需要手动重装）
    TL0 = 0xCD;
    TH0 = 0xD4;
    
    // UART_SendByte(0xAA); // 每 1ms 输出一个字节

    Modbus_OnTimer1ms();

    // DS18B20 计时器
    if (ds18b20_state == DS18B20_CONVERTING && ds18b20_timer > 0) {
        ds18b20_timer--;
    }
    
    // 水位测量计时器
    if (cap_measure_timer > 0) {
        cap_measure_timer--;
    }
}

void main(void)
{
    // 初始化EEPROM配置（读取从机地址和波特率）
    EEPROM_InitConfig();
    // 使用保存的波特率初始化串口
    Uart1_Init(g_baudrate_index);  
    Modbus_Init();
    cap_init();  // 初始化电容测量模块
    DS18B20_ConfigPort(); // 配置DS18B20端口
    Timer0_Init_1ms();
    // 生产模式：不发送启动调试行

    while (1)
    {
        // DS18B20 温度测量状态机（非阻塞）
        switch (ds18b20_state) {
            case DS18B20_IDLE:
                // 启动温度转换
                if (Ds18b20ConvertTemp() == 0) {
                    // 启动成功
                    ds18b20_timer = 800;  // 等待750ms转换时间，留50ms余量
                    ds18b20_state = DS18B20_CONVERTING;
                }
                // 如果初始化失败，保持IDLE状态，下次循环再试
                break;
                
            case DS18B20_CONVERTING:
                // 等待转换完成
                if (ds18b20_timer == 0) {
                    ds18b20_state = DS18B20_READY;
                }
                break;
                
            case DS18B20_READY:
                // 读取温度值
                g_temperature = Ds18b20ReadTemp();
                // 检查是否读取失败
                if (g_temperature != 0xFFFF) {
                    // DS18B20 返回值：16位有符号补码格式，单位0.0625°C
                    // 例如：+25.0625°C = 0x0191, -10.125°C = 0xFF5E
                    // 转换为0.1°C单位的有符号数
                    int16_t temp_raw = (int16_t)g_temperature;  // 作为有符号数处理
                    
                    // 转换公式：temp_0_1C = (temp_raw * 10) / 16
                    // 由于temp_raw已经是有符号数，负数会自动正确处理
                    HoldingReg[1] = (temp_raw * 10) / 16;  // 转换为0.1°C单位
                    // 正温度：+25.0625°C = 0x0191 = 401 -> (401*10)/16 = 250 (25.0°C) ✓
                    // 负温度：-10.125°C = 0xFF5E = -162 -> (-162*10)/16 = -101 (-10.1°C) ✓
                }
                ds18b20_state = DS18B20_IDLE;   // 准备下次测量
                break;
        }
        
        // 电容传感器测量水位 - 使用定时器减少测量频率，避免阻塞
        // HoldingReg[0] 存储水位百分比 (0-100)
        if (cap_measure_timer == 0)
        {
            cap_measure_timer = CAP_MEASURE_INTERVAL_MS;  // 重新装载定时器
            uint16_t cap_time = cap_measure_avg(5);  // 测量5次取平均（增加采样次数）
            // HoldingReg[2] = cap_time;  // 存储原始电容时间值（调试用）
            if (cap_time != 0xFFFF) {
                uint8_t level = water_level_get_percent(cap_time);
                
                // 滑动平均滤波
                water_level_filter[water_level_filter_index] = level;
                water_level_filter_index = (water_level_filter_index + 1) % WATER_LEVEL_FILTER_SIZE;
                
                // 计算平均值
                uint32_t sum = 0;
                for (uint8_t i = 0; i < WATER_LEVEL_FILTER_SIZE; i++) {
                    sum += water_level_filter[i];
                }
                HoldingReg[0] = sum / WATER_LEVEL_FILTER_SIZE;
            }
        }
        
        Modbus_Task();         // 非阻塞解析 + 回应


        // 生产模式：去掉周期性调试输出，保持实时处理
        // 其他你的业务逻辑...
    }
}

// 串口中断
void UART_ISR(void) __interrupt 4
{
    if (RI)
    {
        uint8_t b = SBUF;
        RI = 0;
        Modbus_OnByteReceived(b);   // 不做解析，只丢进“DMA 缓冲”

        // ISR 不回显（临时调试回显已移除）
    }
}