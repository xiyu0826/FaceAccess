#ifndef  __LED_H__
#define  __LED_H__
#include "stm32f1xx_hal.h"

void led_init(void);
void led_ctrl(int led_num, int led_state);
void led_all_on(void);     // 4 颗 LED 全部点亮（低电平）
void led_all_off(void);    // 4 颗 LED 全部熄灭（高电平）
void led_toggle(int led_num);      // 翻转单颗 LED
void led_all_toggle(void);         // 翻转全部 LED

enum LED_NUM{LED0, LED1, LED2, LED3};
enum LED_STATE{LED_ON, LED_OFF};

/*
 * LED 引脚定义（根据 SCH_Schematic1_2026-07-17.pdf 原理图）
 *   LED0 -> PA11 (LED6, R8=1k)
 *   LED1 -> PA12 (LED7, R9=1k)
 *   LED2 -> PA13 (LED8, R10=1k)  [默认 SWDIO]
 *   LED3 -> PA14 (LED9, R11=1k)  [默认 SWCLK]
 * 电路为共阳极接法：阳极接 VCC-MCU，阴极接 MCU 引脚
 * 因此低电平点亮，高电平熄灭
 * 注意：PA13/PA14 默认为 SWD 调试口，led_init() 中会关闭 SWD
 */

#define LED0_GPIO_PORT		GPIOA
#define LED1_GPIO_PORT		GPIOA
#define LED2_GPIO_PORT		GPIOA
#define LED3_GPIO_PORT		GPIOA

#define LED0_GPIO_PIN		GPIO_PIN_11
#define LED1_GPIO_PIN		GPIO_PIN_12
#define LED2_GPIO_PIN		GPIO_PIN_13
#define LED3_GPIO_PIN		GPIO_PIN_14

#endif
