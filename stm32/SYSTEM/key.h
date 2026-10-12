#ifndef __KEY_H__
#define __KEY_H__

#include "stm32f1xx_hal.h"

/*
 * 独立按键引脚映射（根据 Gerber 飞针网表 + 原理图坐标核对）
 *
 * Gerber 网表 (FlyingProbeTesting.json):
 *   U6_4 -> KEY0 -> U5_22  (MCU pin 22)
 *   U7_4 -> KEY1 -> U5_16  (MCU pin 16)
 *   U8_4 -> KEY2 -> U5_15  (MCU pin 15)
 *   U9_4 -> KEY3 -> U5_14  (MCU pin 14)
 *
 * 原理图 MCU 符号 (SCH_Schematic1_2026-07-17.pdf) 引脚-GPIO 对应：
 *   pin 14 -> PC5   (底边，+2.9pt X偏移验证)
 *   pin 15 -> PC4   (左边，+2.9pt Y偏移验证)
 *   pin 16 -> PA7   (底边)
 *   pin 22 -> PA1   (底边)
 *
 * 最终映射：
 *   KEY0 -> PA1  (U6，板上丝印 U6)
 *   KEY1 -> PA7  (U7)
 *   KEY2 -> PC4  (U8)
 *   KEY3 -> PC5  (U9)
 *
 * 按键外部接 10k 上拉电阻到 VCC，按下后引脚接地
 * 因此：高电平 = 未按下，低电平 = 按下
 */

#define KEY0_GPIO_PORT    GPIOA
#define KEY1_GPIO_PORT    GPIOA
#define KEY2_GPIO_PORT    GPIOC
#define KEY3_GPIO_PORT    GPIOC

#define KEY0_GPIO_PIN     GPIO_PIN_1
#define KEY1_GPIO_PIN     GPIO_PIN_7
#define KEY2_GPIO_PIN     GPIO_PIN_4
#define KEY3_GPIO_PIN     GPIO_PIN_5

enum KEY_NUM { KEY0, KEY1, KEY2, KEY3 };
enum KEY_STATE { KEY_RELEASED = 1, KEY_PRESSED = 0 };

void key_init(void);
int  key_read(int key_num);            // 读取当前电平：1=释放，0=按下
int  key_pressed(int key_num);         // 带消抖的按下检测（按下瞬间返回 1）
int  key_released(int key_num);        // 带消抖的释放检测（释放瞬间返回 1）

#endif
