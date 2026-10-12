#ifndef __DS3231_H__
#define __DS3231_H__

#include "stm32f1xx_hal.h"

/*
 * DS3231 高精度 RTC 驱动 (I2C)
 *
 * 硬件:  I2C1 总线, 与 OLED 共用 (PB6=SCL, PB7=SDA)
 * 地址:  0x68 (7-bit)
 * 精度:  ±2ppm, 年误差 < 1分钟
 *
 * 寄存器:
 *   0x00  秒 (BCD, bit7=OSF)
 *   0x01  分 (BCD)
 *   0x02  时 (BCD, bit6=12/24h)
 *   0x03  星期 (1~7)
 *   0x04  日 (BCD)
 *   0x05  月 (BCD, bit7=世纪)
 *   0x06  年 (BCD, 00~99)
 *   0x11  温度 MSB (8-bit signed, 1°C 分辨率)
 */

/* ---- 数据结构 ---------------------------------------------- */
typedef struct {
    uint8_t sec;          /* 0~59 */
    uint8_t min;          /* 0~59 */
    uint8_t hour;         /* 0~23 */
    uint8_t day;          /* 1~31 */
    uint8_t date;         /* 1~31 (同 day) */
    uint8_t month;        /* 1~12 */
    uint8_t year;         /* 0~99 (20xx) */
    uint8_t week;         /* 1~7 (星期一=1) */
} ds3231_time_t;

/* ---- API -------------------------------------------------- */
void    ds3231_init(void);                              /* 探测 I2C 0x68 */
int     ds3231_detected(void);                          /* 1=在线, 0=不存在 */
void    ds3231_get_time(ds3231_time_t *t);              /* 读取当前时间 */
void    ds3231_set_time(ds3231_time_t *t);              /* 写入时间 */
int8_t  ds3231_get_temp(void);                          /* 读取芯片温度 (°C) */
void    ds3231_print_time(void);                        /* 一次性打印当前时间 */
void    ds3231_set_prompt(void);                         /* 串口交互式设置时间 */

#endif
