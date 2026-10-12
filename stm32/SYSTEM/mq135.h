#ifndef __MQ135_H__
#define __MQ135_H__

#include "stm32f1xx_hal.h"

/*
 * MQ-135 空气质量传感器驱动 (ADC)
 *
 * 硬件:  排针 H9_8 → PB0 (ADC_CHANNEL_8)
 *        直接连接, 无需分压电路 (老师确认 AO 输出在安全范围)
 * 供电:  5V (务必用板载 5V)
 * 预热:  上电后需预热 2~3 分钟, 读数才稳定
 *
 * 注意:  MQ-135 是半导体气体传感器, 对 NH3/NOx/酒精/苯/烟雾/CO2 等
 *        多种气体敏感。输出的电压值与空气中还原性气体浓度负相关。
 *        洁浄空气中 ~0.5V, 污染越高电压越高。
 */

/* ---- 空气质量等级 ------------------------------------------ */
typedef enum {
    AQI_GOOD      = 0,   /* 优   (< 500ppm 等效) */
    AQI_MODERATE  = 1,   /* 良   (500~1000) */
    AQI_UNHEALTHY = 2,   /* 差   (1000~2000) */
    AQI_HAZARDOUS = 3,   /* 危险 (> 2000) */
} aqi_level_t;

/* ---- API -------------------------------------------------- */
void         mq135_init(uint32_t adc_channel);   /* 指定 ADC 通道, e.g. ADC_CHANNEL_10 */
uint16_t     mq135_read_raw(void);               /* 读 ADC 原始值 0~4095 */
float        mq135_read_voltage(void);           /* 读电压 0~3.3V */
aqi_level_t  mq135_get_aqi(void);                /* 读空气质量等级 */
const char*  mq135_aqi_str(aqi_level_t level);   /* 等级→中文名称 */
void         mq135_test(void);                   /* 连续打印 10 次读数 */

#endif
