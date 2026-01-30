#ifndef __CAP_H__
#define __CAP_H__

#include <stdint.h>

void cap_init(void);  // 初始化电容测量模块
uint16_t cap_measure(void); // 测量一次电容充电时间，单位：us
uint8_t water_level_get_percent(uint16_t t);    // 根据时间获取水位百分比
uint16_t cap_measure_avg(uint8_t n);    // 测量 n 次并取平均

#endif
