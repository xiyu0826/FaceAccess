#ifndef __UART_SCAN_H__
#define __UART_SCAN_H__

#include "stm32f1xx_hal.h"

/*
 * 全UART扫描器 — 确定 WiFi 和蓝牙模块到底接在哪个 UART
 *
 * 依次测试 UART1/2/3, 各波特率 9600~115200
 * 每个组合发 AT 指令, 打印响应
 */

void uart_scan_all(void);

#endif
