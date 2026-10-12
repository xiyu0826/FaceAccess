#ifndef  __RUNLED_H__
#define  __RUNLED_H__

#include "stm32f1xx_hal.h"

void led_run_init(void);    // 跑马灯初始化（时钟 + GPIO + 默认熄灭）
void led_run_step(void);    // 前进到下一盏灯（来回流水，每次只亮一盏）

#endif
