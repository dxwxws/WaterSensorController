#ifndef __MODBUS_H__
#define __MODBUS_H__

#include <stdint.h>

// RTU 帧相关定义
#define MODBUS_RX_BUF_SIZE   128   // 环形缓冲区大小
#define MODBUS_FRAME_MAX     64    // 单帧最大长度
#define RTU_FRAME_GAP_MS     5     // 静默时间阈值（ms）

void Modbus_Init(void);

// 串口收到 1 字节时调用（类似 DMA 接收）
void Modbus_OnByteReceived(uint8_t byte);

// 定时器每 1ms 调用一次，用于 RTU 帧间隔检测
void Modbus_OnTimer1ms(void);

// 主循环中调用，用于处理完整 Modbus 帧（非阻塞）
void Modbus_Task(void);

void Modbus_Read_Holding_Register(uint8_t *frame, uint8_t len);
void Modbus_Write_Single_Register(uint8_t *frame, uint8_t len);
void Modbus_Send_Error(uint8_t func, uint8_t err);
// void UART_SendHex(uint8_t byte);

extern __xdata uint16_t HoldingReg[];
extern volatile uint16_t modbus_rx_count;
extern volatile uint16_t msTicks;
extern volatile uint8_t need_reset;

#endif
