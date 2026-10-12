#ifndef __SERVO_H__
#define __SERVO_H__

#include "stm32f1xx_hal.h"

/*
 * SG90 舵机驱动
 * 控制信号：50Hz PWM，脉宽 0.5ms=0° / 1.5ms=90° / 2.5ms=180°
 *
 * 引脚 PB1 (TIM3_CH4)，对应板子上的 SG3 排针（SG3 网络 = PB1）。
 */

#define SERVO_GPIO_PORT      GPIOB
#define SERVO_GPIO_PIN       GPIO_PIN_1
#define SERVO_TIM            TIM3
#define SERVO_CHANNEL        TIM_CHANNEL_4

#define SERVO_ANGLE_CLOSED   0      // 上锁角度
#define SERVO_ANGLE_OPEN     90     // 开锁角度

void servo_init(void);
void servo_set_angle(int angle);   // 设置角度 0~180
void servo_open(void);             // 开锁 (90°)
void servo_close(void);            // 上锁 (0°)
uint32_t servo_get_pulse(void);    // 读取当前 CCR 脉宽值（调试用）

#endif
