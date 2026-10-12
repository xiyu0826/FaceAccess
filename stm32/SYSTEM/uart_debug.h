#ifndef __UART_DEBUG_H__
#define __UART_DEBUG_H__

#include "stm32f1xx_hal.h"
#include <stdio.h>

extern UART_HandleTypeDef huart1;  /* 供 bridge 等共用 */

void uart_debug_init(void);

/*
 * 重定向 printf/getchar 到底层 UART
 * 在 Keil 中需勾选 "Use MicroLIB"
 */
int fputc(int ch, FILE *f);
int fgetc(FILE *f);
int __io_putchar(int ch);

/* 非阻塞接收: -1 = 无数据, ≥0 = 收到的字符 */
int uart_getchar(void);

/* 阻塞接收: 一直等到有数据 */
int uart_getchar_blocking(void);

/* 发送原始数据 */
void uart_send(const uint8_t *data, uint16_t len);

#endif
