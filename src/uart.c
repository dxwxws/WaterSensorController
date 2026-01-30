#include "uart.h"
#include "modbus.h"

//--------------------- 串口初始化 ---------------------
// 支持多种波特率配置
// baudrate_index: 0=2400, 1=4800, 2=9600, 3=19200, 4=38400, 5=57600, 6=115200, 7=1200

void Uart1_Init(uint8_t baudrate_index)
{
	uint16_t brt_value;
	
	// 根据波特率索引计算定时器初值
	switch(baudrate_index)
	{
		case 0: // 2400bps
			brt_value = 65536 - FOSC / 2400 / 4;
			break;
		case 1: // 4800bps
			brt_value = 65536 - FOSC / 4800 / 4;
			break;
		case 2: // 9600bps
			brt_value = 65536 - FOSC / 9600 / 4;
			break;
		case 3: // 19200bps
			brt_value = 65536 - FOSC / 19200 / 4;
			break;
		case 4: // 38400bps
			brt_value = 65536 - FOSC / 38400 / 4;
			break;
		case 5: // 57600bps
			brt_value = 65536 - FOSC / 57600 / 4;
			break;
		case 6: // 115200bps
			brt_value = 65536 - FOSC / 115200 / 4;
			break;
		case 7: // 1200bps
			brt_value = 65536 - FOSC / 1200 / 4;
			break;
		default: // 默认115200bps
			brt_value = 65536 - FOSC / 115200 / 4;
			break;
	}
	
	SCON = 0x50;		//8位数据,可变波特率
	AUXR |= 0x40;		//定时器时钟1T模式
	AUXR &= 0xFE;		//串口1选择定时器1为波特率发生器
	TMOD &= 0x0F;		//设置定时器模式
	TL1 = (uint8_t)brt_value;			//设置定时初始值
	TH1 = brt_value >> 8;		//设置定时初始值
    ET1 = 0;			//禁止定时器中断
    TR1 = 1;			//定时器1开始计时

    // 清除可能的串口标志，启用串口中断（接收中断）。
    TI = 0;
    RI = 0;
    ES = 1;             // 允许串口中断
}


void UART_SendByte(uint8_t dat)
{
    uint8_t oldES = ES;
    ES = 0; // 临时禁止串口中断，避免发送时被接收中断干扰
    SBUF = dat;
    while (!TI);
    TI = 0;
    ES = oldES;
}

void UART_SendBytes(uint8_t *buf, uint8_t len)
{
    uint8_t oldES = ES;
    ES = 0; // 在整个多字节发送过程中禁用串口中断，防止中断处理修改缓冲或产生竞态
    for (uint8_t i = 0; i < len; i++)
    {
        SBUF = buf[i];
        while (!TI);
        TI = 0;
    }
    ES = oldES;
}

