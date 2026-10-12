#ifndef __LIGHT_H__
#define __LIGHT_H__

#include "stm32f1xx_hal.h"

/*
 * 光敏电阻传感器驱动 (DO→ADC)
 *
 * 硬件:  排针 H8_6 = PA6 (ADC_CHANNEL_6)
 * 模块:  三线制 (VCC=5V, GND, DO→H8_6)
 *        只有 DO 数字输出, 走 ADC 读电压判断暗/亮
 *        DO=0.14V(亮) / DO=3.3V(暗)
 */

/* ---- 光线等级 ---------------------------------------------- */
typedef enum {
    LIGHT_DARK   = 0,    /* 暗 */
    LIGHT_DIM    = 1,    /* 较暗 */
    LIGHT_NORMAL = 2,    /* 正常室内光 */
    LIGHT_BRIGHT = 3,    /* 明亮 */
} light_level_t;

/* ---- API -------------------------------------------------- */
void         light_init(uint32_t adc_channel);
uint16_t     light_read_raw(void);
float        light_read_voltage(void);
light_level_t light_get_level(void);
const char*  light_level_str(light_level_t lv);
void         light_test(void);

void         light_auto_led(int enable);
void         light_auto_led_poll(void);

#endif
