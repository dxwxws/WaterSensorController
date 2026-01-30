#ifndef __UART_H__
#define __UART_H__

#include "reg52.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define FOSC 11059200UL 

void Uart1_Init(uint8_t baudrate_index);
void UART_SendByte(uint8_t dat);
void UART_SendBytes(uint8_t *buf, uint8_t len);

#endif
