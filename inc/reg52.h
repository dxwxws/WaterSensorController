/*--------------------------------------------------------------------------
REG52.H

Header file for generic 80C52 and 80C32 microcontroller.
Copyright (c) 1988-2002 Keil Elektronik GmbH and Keil Software, Inc.
All rights reserved.
这个头文件是经过修改以支持SDCC编译器的reg52.h版本，主要修改包括：
1. 添加了对SDCC编译器的支持
2. 修改了一些数据类型的定义
3. 添加了一些特定于SDCC的关键字
4. 添加了一些寄存器的定义
--------------------------------------------------------------------------*/

#ifndef __REG52_H__
#define __REG52_H__

#ifndef __SDCC
#error "This header file must be used for SDCC compiler !"
#endif

/* redefine some keywords */

// primitive type
typedef __sfr sfr;
typedef __sfr16 sfr16;
typedef __sfr32 sfr32;
typedef __sbit sbit;
typedef __bit bit;

// keywords
#define interrupt __interrupt
#define using __using
#define _at_ __at
#define _priority_
#define _task_

// storage type
#define reentrant __reentrant
#define compact
#define small __near
#define large __far
#define data __data
#define bdata
#define idata __idata
#define pdata __pdata
#define xdata __xdata
#define code __code

/*  BYTE Registers  */
__sfr __at(0x80) P0;
__sbit __at(0x80+0) P00;
__sbit __at(0x80+1) P01;
__sbit __at(0x80+2) P02;
__sbit __at(0x80+3) P03;
__sbit __at(0x80+4) P04;
__sbit __at(0x80+5) P05;
__sbit __at(0x80+6) P06;
__sbit __at(0x80+7) P07;

__sfr __at(0x90) P1;
__sbit __at(0x90+0) P10;
__sbit __at(0x90+1) P11;
__sbit __at(0x90+2) P12;
__sbit __at(0x90+3) P13;
__sbit __at(0x90+4) P14;
__sbit __at(0x90+5) P15;
__sbit __at(0x90+6) P16;
__sbit __at(0x90+7) P17;
__sfr __at(0x91) P1M1;
__sfr __at(0x92) P1M0;
__sfr __at(0x93) P0M1;
__sfr __at(0x94) P0M0;
__sfr __at(0x95) P2M1;
__sfr __at(0x96) P2M0;
__sfr __at(0x97) CLK_DIV;

__sfr __at(0xA0) P2;
__sbit __at(0xA0+0) P20;
__sbit __at(0xA0+1) P21;
__sbit __at(0xA0+2) P22;
__sbit __at(0xA0+3) P23;
__sbit __at(0xA0+4) P24;
__sbit __at(0xA0+5) P25;
__sbit __at(0xA0+6) P26;
__sbit __at(0xA0+7) P27;

__sfr __at(0xB0) P3;
__sbit __at(0xB0+0) P30;
__sbit __at(0xB0+1) P31;
__sbit __at(0xB0+2) P32;
__sbit __at(0xB0+3) P33;
__sbit __at(0xB0+4) P34;
__sbit __at(0xB0+5) P35;
__sbit __at(0xB0+6) P36;
__sbit __at(0xB0+7) P37;

/*  P5  */
__sfr __at(0xC8) P5;
__sbit __at(0xC8+0) P50;
__sbit __at(0xC8+1) P51;
__sbit __at(0xC8+2) P52;
__sbit __at(0xC8+3) P53;
__sbit __at(0xC8+4) P54;
__sbit __at(0xC8+5) P55;
__sbit __at(0xC8+6) P56;
__sbit __at(0xC8+7) P57;
__sfr  __at(0xC9)  P5M1;
__sfr  __at(0xCA)  P5M0;
/****************************/
/*****添加的寄存器定义******/
__sfr  __at(0xBA)  P_SW2;
__sfr  __at(0x8E)  AUXR;
__sfr  __at(0xB1)  P3M1;
__sfr  __at(0xB2)  P3M0;

/*****EEPROM********/
__sfr __at(0xC2) IAP_DATA;
__sfr __at(0xC3) IAP_ADDRH;
__sfr __at(0xC4) IAP_ADDRL;
__sfr __at(0xC5) IAP_CMD;
__sfr __at(0xC6) IAP_TRIG;
__sfr __at(0xC7) IAP_CONTR;
__sfr __at(0xF5) IAP_TPS;
/*************************/
__sfr __at(0xD0) PSW;
__sfr __at(0xE0) ACC;
__sfr __at(0xF0) B;
__sfr __at(0x81) SP;
__sfr __at(0x82) DPL;
__sfr __at(0x83) DPH;
__sfr __at(0x87) PCON;
__sfr __at(0x88) TCON;
__sfr __at(0x89) TMOD;
__sfr __at(0x8A) TL0;
__sfr __at(0x8B) TL1;
__sfr __at(0x8C) TH0;
__sfr __at(0x8D) TH1;
__sfr __at(0xA8) IE;
__sfr __at(0xB8) IP;
__sfr __at(0x98) SCON;
__sfr __at(0x99) SBUF;

/*  8052 Extensions  */
__sfr __at(0xC8) T2CON;
__sfr __at(0xCA) RCAP2L;
__sfr __at(0xCB) RCAP2H;
__sfr __at(0xCC) TL2;
__sfr __at(0xCD) TH2;


/*  BIT Registers  */
/*  PSW  */
__sbit __at(0xd7) CY;
__sbit __at(0xd6) AC;
__sbit __at(0xd5) F0;
__sbit __at(0xd4) RS1;
__sbit __at(0xd3) RS0;
__sbit __at(0xd2) OV;
__sbit __at(0xd0) P; //8052 only

/*  TCON  */
__sbit __at(0x8f) TF1;
__sbit __at(0x8e) TR1;
__sbit __at(0x8d) TF0;
__sbit __at(0x8c) TR0;
__sbit __at(0x8b) IE1;
__sbit __at(0x8a) IT1;
__sbit __at(0x89) IE0;
__sbit __at(0x88) IT0;

/*  IE  */
__sbit __at(0xaf) EA;
__sbit __at(0xad) ET2; //8052 only
__sbit __at(0xac) ES;
__sbit __at(0xab) ET1;
__sbit __at(0xaa) EX1;
__sbit __at(0xa9) ET0;
__sbit __at(0xa8) EX0;

/*  IP  */
__sbit __at(0xbd) PT2;
__sbit __at(0xbc) PS;
__sbit __at(0xbb) PT1;
__sbit __at(0xba) PX1;
__sbit __at(0xb9) PT0;
__sbit __at(0xb8) PX0;

/*  P3  */
__sbit __at(0xb7) RD;
__sbit __at(0xb6) WR;
__sbit __at(0xb5) T1;
__sbit __at(0xb4) T0;
__sbit __at(0xb3) INT1;
__sbit __at(0xb2) INT0;
__sbit __at(0xb1) TXD;
__sbit __at(0xb0) RXD;

/*  SCON  */
__sbit __at(0x9f) SM0;
__sbit __at(0x9e) SM1;
__sbit __at(0x9d) SM2;
__sbit __at(0x9c) REN;
__sbit __at(0x9b) TB8;
__sbit __at(0x9a) RB8;
__sbit __at(0x99) TI;
__sbit __at(0x98) RI;

/*  P1  */
__sbit __at(0x91) T2EX; // 8052 only
__sbit __at(0x90) T2; // 8052 only
             
/*  T2CON  */
__sbit __at(0xcf) TF2;
__sbit __at(0xce) EXF2;
__sbit __at(0xcd) RCLK;
__sbit __at(0xcc) TCLK;
__sbit __at(0xcb) EXEN2;
__sbit __at(0xca) TR2;
__sbit __at(0xc9) C_T2;
__sbit __at(0xc8) CP_RL2;

#endif
