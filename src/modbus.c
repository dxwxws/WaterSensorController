#include "modbus.h"
#include "crc16.h"
#include "uart.h"
#include "eeprom.h"
#include "cap.h"
#include "reg52.h"

#define TRUE  1
#define FALSE 0
// #define MODBUS_DEBUG 0

// 环形缓冲区（串口“DMA 接收缓存”）
__xdata uint8_t modbus_rx_buf[MODBUS_RX_BUF_SIZE];
volatile uint8_t modbus_rx_write = 0;   // 写指针（中断中更新）
volatile uint8_t modbus_rx_read  = 0;   // 读指针（主循环中使用）

// 当前帧缓冲
__xdata uint8_t modbus_frame_buf[MODBUS_FRAME_MAX];
uint8_t modbus_frame_len = 0;

// RTU 帧间隔检测
volatile uint8_t rtu_silence_cnt = 0;   // 静默 ms 计数
volatile bit     rtu_receiving   = 0;   // 当前是否处于接收过程
volatile bit     frame_ready     = 0;   // 有完整帧待处理

// 寄存器表
__xdata uint16_t HoldingReg[HOLDING_REG_SIZE];   // 0:水位  1:温度 等

// 调试计数器
volatile uint16_t modbus_rx_count = 0;
volatile uint16_t msTicks = 0;

//--------------------- 内部工具函数 ---------------------
static uint8_t Modbus_RxAvailable(void)
{
    return (modbus_rx_write != modbus_rx_read);
}

static uint8_t Modbus_RxGetByte(uint8_t *byte)
{
    if (modbus_rx_write == modbus_rx_read)
        return FALSE;

    *byte = modbus_rx_buf[modbus_rx_read++];
    if (modbus_rx_read >= MODBUS_RX_BUF_SIZE)
        modbus_rx_read = 0;

    return TRUE;
}

//--------------------- 初始化 ---------------------
void Modbus_Init(void)
{
    modbus_rx_write = 0;
    modbus_rx_read  = 0;
    modbus_frame_len = 0;
    rtu_silence_cnt  = 0;
    rtu_receiving    = 0;
    frame_ready      = 0;

    // 加载 EEPROM 中的参数
    HoldingReg[HR_T_AIR] = g_t_air;
    HoldingReg[HR_T_FULL] = g_t_full;
    HoldingReg[2] = 0;  // 初始化电磁阀控制寄存器为0（关闭）
    SV = 0; // 初始关闭电磁阀
}

//--------------------- 串口中断钩子：收到 1 字节 ---------------------
void Modbus_OnByteReceived(uint8_t byte)
{
    // UART_SendByte(byte); // ← 调试：确认接收到数据

    uint8_t next = modbus_rx_write + 1;
    if (next >= MODBUS_RX_BUF_SIZE)
        next = 0;

    // 简单防止溢出：丢弃最老数据或直接丢弃本字节，这里选择丢弃本字节
    if (next != modbus_rx_read)
    {
        modbus_rx_buf[modbus_rx_write] = byte;
        modbus_rx_write = next;
    }

    // 收到字节，说明当前处于接收状态
    rtu_receiving   = 1;
    rtu_silence_cnt = 0;   // 清零静默计时

    // 调试：统计收到的字节数（在 ISR 中递增简短变量）
    modbus_rx_count++;
}

//--------------------- 1ms 定时器钩子：用于帧间隔检测 ---------------------
void Modbus_OnTimer1ms(void)
{
    // ms 计数（调试用）
    if (msTicks < 0xFFFF) msTicks++;

    if (rtu_receiving)
    {
        if (rtu_silence_cnt < 255)
            rtu_silence_cnt++;

        if (rtu_silence_cnt >= RTU_FRAME_GAP_MS)
        {
            rtu_receiving   = 0;
            rtu_silence_cnt = 0;

            // ★★★ 在帧结束时锁定帧长度 ★★★
            modbus_frame_len = (modbus_rx_write >= modbus_rx_read)
                             ? (modbus_rx_write - modbus_rx_read)
                             : (MODBUS_RX_BUF_SIZE - modbus_rx_read + modbus_rx_write);

            frame_ready     = 1;   // 通知主循环：有完整帧
            // 不再在中断/定时器中打印调试字节（会干扰 Modbus 实时发送）
        }
    }
}

//--------------------- 主循环中调用：处理 Modbus 帧 ---------------------

void Modbus_Task(void)
{
    // 仅在收到“完整帧”标记时一次性从环形缓冲搬运整帧，避免重复/竞态读取
    if (!frame_ready)
        return;

    frame_ready = 0;    
    
    // 防止异常长度导致越界
    if (modbus_frame_len == 0 || modbus_frame_len > MODBUS_FRAME_MAX)
    {
        modbus_frame_len = 0;
        modbus_rx_read = modbus_rx_write; // 同步清空环形缓冲
        return;
    }

    // 一次性搬运整帧到 modbus_frame_buf
    for (uint8_t i = 0; i < modbus_frame_len; i++)
    {
        if (!Modbus_RxGetByte(&modbus_frame_buf[i]))
        {
            // 读取失败：可能被中断再次修改指针，丢弃本帧
            modbus_frame_len = 0;
            modbus_rx_read = modbus_rx_write;
            return;
        }
    }
    // 此时 modbus_frame_buf[] 已经是完整的一帧

    // 生产模式：不打印原始帧，直接处理

    if (modbus_frame_len < 8)
    {
        modbus_frame_len = 0;
        return; // 最小 Modbus 帧长度为 8
    }

    // 校验 CRC16
{
    uint16_t crcCalc = Modbus_CRC16(modbus_frame_buf, modbus_frame_len - 2);
    uint16_t crcRecv = modbus_frame_buf[modbus_frame_len - 2]
                     | (modbus_frame_buf[modbus_frame_len - 1] << 8);

    if (crcCalc != crcRecv)
    {
    // #if MODBUS_DEBUG
    //     UART_SendBytes((uint8_t*)"ERR:CRC\r\n", 9);
    // #endif
        modbus_frame_len = 0;
        return; // CRC 错误
    }
}

    // 地址判断
    if (modbus_frame_buf[0] != g_slave_id)
    {
    // #if MODBUS_DEBUG
    //     UART_SendBytes((uint8_t*)"IGN:ADDR\r\n", 10);
    // #endif
        modbus_frame_len = 0;
        return; // 不是发给我的
    }

    {
        uint8_t func = modbus_frame_buf[1];

        switch (func)
        {
            case 0x03:
                Modbus_Read_Holding_Register(modbus_frame_buf, modbus_frame_len);
                break;

            case 0x06:
                Modbus_Write_Single_Register(modbus_frame_buf, modbus_frame_len);
                break;

            default:
            // #if MODBUS_DEBUG
            //     UART_SendBytes((uint8_t*)"ERR:FUNC\r\n", 10);
            // #endif
                Modbus_Send_Error(func, 0x01); // 不支持的功能码
                break;
        }
    }

    // 当前帧处理完
    modbus_frame_len = 0;
    modbus_rx_read = modbus_rx_write;  // 清空环形缓冲区

}

uint8_t Modbus_HasFrameReady(void)
{
    return frame_ready ? 1 : 0;
}

//--------------------- 读保持寄存器 0x03 ---------------------
void Modbus_Read_Holding_Register(uint8_t *frame, uint8_t len)
{
    // 生产模式：不回显，直接构造应答

    if (len < 8)
    {
    // #if MODBUS_DEBUG
    //     UART_SendBytes((uint8_t*)"ERR:PARAM\r\n", 11);
    // #endif
        Modbus_Send_Error(0x03, 0x02);
        return;
    }

    uint16_t addr = (frame[2] << 8) | frame[3];
    uint16_t num  = (frame[4] << 8) | frame[5];

    // 检查地址范围：支持0-9和0x07D0-0x07D1
    if (num == 0 || num > 64)
    {
    // #if MODBUS_DEBUG
    //     UART_SendBytes((uint8_t*)"ERR:PARAM\r\n", 11);
    // #endif
        Modbus_Send_Error(0x03, 0x02);
        return;
    }

    // 发送缓冲放在栈上也可以，但为稳妥可放 __xdata 全局
    __xdata uint8_t sendBuf[MODBUS_FRAME_MAX];
    uint8_t idx = 0;

    sendBuf[idx++] = g_slave_id;
    sendBuf[idx++] = 0x03;
    sendBuf[idx++] = num * 2;

    for (uint16_t i = 0; i < num; i++)
    {
        uint16_t current_addr = addr + i;
        uint16_t val;
        
        // 根据地址读取不同的值
        if (current_addr < 10)
        {
            // 普通寄存器 - 直接从HoldingReg读取
            // 寄存器0（水位）和寄存器1（温度）由main.c主循环定期更新
            val = HoldingReg[current_addr];
        }
       
        else if (current_addr == 0x07D0)
        {
            // 读取从机地址
            val = g_slave_id;
        }
        else if (current_addr == 0x07D1)
        {
            // 读取波特率索引
            val = g_baudrate_index;
        }
        else
        {
            // 非法地址
        // #if MODBUS_DEBUG
        //     UART_SendBytes((uint8_t*)"ERR:ADDR\r\n", 10);
        // #endif
            Modbus_Send_Error(0x03, 0x02);
            return;
        }
        
        sendBuf[idx++] = val >> 8;
        sendBuf[idx++] = val & 0xFF;
    }

    {
        uint16_t crc = Modbus_CRC16(sendBuf, idx);
        sendBuf[idx++] = crc & 0xFF;
        sendBuf[idx++] = crc >> 8;
    }

    for (uint8_t i = 0; i < idx; i++)
        UART_SendByte(sendBuf[i]);
}

//--------------------- 异常响应 ---------------------
void Modbus_Send_Error(uint8_t func, uint8_t err)
{
    uint8_t sendBuf[5];
    uint8_t idx = 0;

    sendBuf[idx++] = g_slave_id;
    sendBuf[idx++] = func | 0x80;
    sendBuf[idx++] = err;

    {
        uint16_t crc = Modbus_CRC16(sendBuf, idx);
        sendBuf[idx++] = crc & 0xFF;
        sendBuf[idx++] = crc >> 8;
    }

    UART_SendBytes(sendBuf, idx);
}

void HoldingReg_WriteCallback(uint16_t addr, uint16_t value)
{
    HoldingReg[addr] = value;

    if (addr == HR_T_AIR)
    {
        g_cfg.t_air = value;
        g_t_air = value;
        EEPROM_SaveAll();
    }
    else if (addr == HR_T_FULL)
    {
        g_cfg.t_full = value;
        g_t_full = value;
        EEPROM_SaveAll();
    }
}


//--------------------- 写单个寄存器 0x06 ---------------------
void Modbus_Write_Single_Register(uint8_t *frame, uint8_t len)
{
    if (len < 8)
    {
    // #if MODBUS_DEBUG
    //     UART_SendBytes((uint8_t*)"ERR:PARAM\r\n", 11);
    // #endif
        Modbus_Send_Error(0x06, 0x02);
        return;
    }

    uint16_t addr = (frame[2] << 8) | frame[3];
    uint16_t value = (frame[4] << 8) | frame[5];

    if (addr == REG_VALVE_CTRL)
    {
        // 写入寄存器（不需要回调）
        HoldingReg[addr] = value;
        // 电磁阀控制寄存器写入
        if (value == 0)
        {
            SV = 0; // 关闭电磁阀
        }
        else if (value == 1)
        {
            SV = 1; // 打开电磁阀
        }
        else
        {
            Modbus_Send_Error(0x06, 0x03); // 非法数据值
            return;
        }
        // 直接回显请求帧表示成功
        for (uint8_t i = 0; i < len; i++)
            UART_SendByte(frame[i]);
    }

    else if (addr < 10)
    {
        HoldingReg_WriteCallback(addr, value);   // ★★★ 调用回调 ★★★

        for (uint8_t i = 0; i < len; i++)
            UART_SendByte(frame[i]);
    }


    // 判断是否为配置寄存器
   else if (addr == 0x07D0)
    {
        // 修改从机地址
        if (value > 0 && value <= 247)
        {
            EEPROM_SaveSlaveID((uint8_t)value);
            // 直接回显请求帧表示成功
            for (uint8_t i = 0; i < len; i++)
                UART_SendByte(frame[i]);
        }
        else
        {
            Modbus_Send_Error(0x06, 0x03); // 非法数据值
        }
    }
    else if (addr == 0x07D1)
    {
        // 修改波特率
        if (value <= 7)
        {
            EEPROM_SaveBaudrate((uint8_t)value);
            // 直接回显请求帧表示成功
            for (uint8_t i = 0; i < len; i++)
                UART_SendByte(frame[i]);
            
            // 延时确保数据发送完成
            {
                uint16_t delay = 0;
                for(delay = 0; delay < 30000; delay++);
            }
            // 软件复位
            IAP_CONTR |= 0x60;
        }
        else
        {
            Modbus_Send_Error(0x06, 0x03); // 非法数据值
        }
    }
    else if (addr == CMD_CAL_AIR)
    {
        if (value == 1)
        {
            uint16_t raw = cap_measure_avg(20);  // 测 20 次取平均
            g_cfg.t_air = raw;
            EEPROM_SaveAll();
            HoldingReg[HR_T_AIR] = 0; // 自动清零
            // 直接回显请求帧表示成功
            for (uint8_t i = 0; i < len; i++)
                UART_SendByte(frame[i]);
        }
        else
        {
            Modbus_Send_Error(0x06, 0x03);
        }
    }

    else if (addr == CMD_CAL_FULL)
    {
        if (value == 1)
        {
            uint16_t raw = cap_measure_avg(20);
            g_cfg.t_full = raw;
            EEPROM_SaveAll();
            HoldingReg[HR_T_FULL] = 0;
            // 直接回显请求帧表示成功
            for (uint8_t i = 0; i < len; i++)
                UART_SendByte(frame[i]);
        }
        else
        {
            Modbus_Send_Error(0x06, 0x03);
        }
    }

    else
    {
        Modbus_Send_Error(0x06, 0x02); // 非法地址
    }
}

