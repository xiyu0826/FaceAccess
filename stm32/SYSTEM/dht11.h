#ifndef __DHT11_H__
#define __DHT11_H__

#include "stm32f1xx_hal.h"

/*
 * DHT11 温湿度传感器单总线驱动
 *
 * 硬件:  PC1 = DHT11 DATA (排针 H9_1, 三针模块: VCC=3.3V, GND, DATA=PC1)
 * 注意:  PA0 未引出到排针，改用 H9_1 (PC1)
 * 协议:  单总线 (One-Wire), 主机发起 → 从机响应 → 40bit 数据
 * 数据格式: 湿度整数(8) + 湿度小数(8) + 温度整数(8) + 温度小数(8) + 校验(8)
 * 注意:  读取期间需关中断, 确保时序精度
 */

/* ---- 返回值 ------------------------------------------------ */
#define DHT11_OK        0     /* 读取成功, 校验通过 */
#define DHT11_ERR_CHECK 1     /* 校验和错误 */
#define DHT11_ERR_NORSP 2     /* 传感器无响应 */
#define DHT11_ERR_TIMEO 3     /* 读取超时 */

/* ---- 数据结构 ---------------------------------------------- */
typedef struct {
    uint8_t  humi;            /* 湿度 整数部分 (%) */
    uint8_t  humi_dec;        /* 湿度 小数部分 (通常为0) */
    uint8_t  temp;            /* 温度 整数部分 (°C) */
    uint8_t  temp_dec;        /* 温度 小数部分 (通常为0) */
} dht11_data_t;

/* ---- API -------------------------------------------------- */
void dht11_init(void);                              /* 初始化 PA0 */
int  dht11_read(dht11_data_t *out);                 /* 读一次温湿度, 返回错误码 */
void dht11_test(void);                              /* 连续打印 5 次温湿度 */

#endif
