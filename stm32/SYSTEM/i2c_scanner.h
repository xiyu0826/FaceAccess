#ifndef __I2C_SCANNER_H__
#define __I2C_SCANNER_H__

#include "stm32f1xx_hal.h"

extern I2C_HandleTypeDef hi2c1;     /* 供 OLED 等 I2C 设备共用 */

void i2c_scanner_init(void);
void i2c_scan(void);                       /* 扫描 0x02~0x7E, 打印结果 */
int  i2c_probe(uint8_t addr);              /* 探测单个地址: 1=存在, 0=不存在 */
void i2c_set_speed(uint32_t speed_hz);     /* 切换速率 100000 或 400000 */

#endif
