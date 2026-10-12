#ifndef __BEEP_H__
#define __BEEP_H__

#include "stm32f1xx_hal.h"

/*
 * 蜂鸣器引脚映射（根据 SCH_Schematic1_2026-07-17.pdf + Gerber 网表）
 *   BEEP -> PA8 (U5 pin 36)
 *   经 NPN 三极管 S8050 (Q1) 驱动，R17=1kΩ 基极限流，H10 2P 跳线串联
 *
 * 蜂鸣器类型：无源蜂鸣器 (BUZZER1)，谐振频率 3kHz
 *   - 需要方波驱动，单纯直流电平不响（只有"咔"一声）
 *   - beep_burst() 用软件延时产生 ~3kHz 方波
 *   - beep_on/off/toggle 仍保留，但对无源蜂鸣器效果有限
 */

#define BEEP_GPIO_PORT    GPIOA
#define BEEP_GPIO_PIN     GPIO_PIN_8

/* 初始化 GPIO */
void beep_init(void);

/* 无源蜂鸣器专用：产生 3kHz 方波持续指定毫秒 */
void beep_burst(uint32_t ms);

/* 以下函数对无源蜂鸣器效果有限，仅作兼容保留 */
void beep_on(void);
void beep_off(void);
void beep_toggle(void);
int  beep_state(void);

#endif
