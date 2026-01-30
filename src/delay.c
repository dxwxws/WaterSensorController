#include "delay.h"

// 简单空转延时（SDCC 下足够用于测试）
void delay_us(uint16_t us)
{
    while(us--)
    {
        __asm
            nop
            nop
            nop
        __endasm;
    }
}

void delay_ms(uint16_t ms)
{
    while(ms--)
        delay_us(1000);
}
