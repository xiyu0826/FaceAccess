#ifndef __ADC_TEST_H__
#define __ADC_TEST_H__

#include "stm32f1xx_hal.h"

extern ADC_HandleTypeDef hadc1;    /* 供外部传感器驱动共用 */

/*
 * ADC 多通道测试驱动
 *
 * 板载ADC排针:
 *   ADC1 → H9_8  (MQ-135 空气质量传感器)
 *   ADC2 → H8_6  (光敏电阻模块)
 *   ADC3 → H9_5  (备用)
 *
 * 使用 ADC1 的多个通道进行扫描测试,
 * 通过实测确定每个排针对应的具体ADC通道。
 *
 * STM32F103RCT6 ADC1 可用通道:
 *   IN0=PA0, IN1=PA1(KEY0), IN2=PA2(UART2_TX),
 *   IN3=PA3(UART2_RX), IN4=PA4, IN5=PA5, IN6=PA6,
 *   IN7=PA7(KEY1), IN8=PB0, IN9=PB1,
 *   IN10=PC0, IN11=PC1, IN12=PC2, IN13=PC3
 *
 * 已知占用: PA1,PA2,PA3,PA7 → 排除 ADC 测试
 * 待测候选: PA0,PA4,PA5,PA6, PB0,PB1, PC0,PC1,PC2,PC3
 */

/* ADC 测试通道列表 (排除已占用的引脚: PA1,PA2,PA3,PA7, PB1(舵机PWM)) */
#define ADC_TEST_CHANNELS  9

typedef struct {
    char    *name;        /* GPIO 名称, 如 "PA0" */
    GPIO_TypeDef *port;
    uint16_t pin;
    uint32_t channel;     /* ADC_CHANNEL_x */
} adc_test_pin_t;

void adc_test_init(void);
void adc_scan_all(void);              /* 扫描所有候选通道, 打印原始值+电压 */
void adc_scan_channel(uint32_t ch);   /* 单通道连续采样, 按任意键退出 */

/*
 * 单独读取某个通道 (软件触发, 阻塞)
 * 返回: 12-bit ADC 原始值 (0~4095)
 */
uint16_t adc_read_channel(uint32_t ch);

#endif
