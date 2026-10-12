#ifndef __LED_ALL_H__
#define __LED_ALL_H__

#include "stm32f1xx_hal.h"
#include "led.h"                /* led_all_on/off 等基础函数在 led.h */

/*
 * LED 自检扩展驱动
 * 基础函数 (led_init, led_all_on, led_all_off, led_toggle 等) 在 led.c/led.h
 * 本文件只提供自检流水灯功能
 */

void led_all_init(void);         /* 初始化 PA11-PA14, 关闭SWD */
void led_all_self_test(void);    /* 流水灯自检3轮 */

#endif
