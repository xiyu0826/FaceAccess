#ifndef __DOOR_H__
#define __DOOR_H__

#include "stm32f1xx_hal.h"

/*
 * 开锁命令模块
 * UART1(CH340) 中断接收 + 环形缓冲，收到 "OPEN" 命令就开锁
 * 命令格式：OPEN 或 OPEN:名字
 */
void door_task(void* arg);

/* ---- UART1 中断接收相关（供中断/回调调用） ---- */
void door_uart_init(void);                         // 初始化中断接收
void door_uart_irq_handler(void);                  // USART1_IRQHandler 调用
void door_uart_rx_cplt(UART_HandleTypeDef* huart); // HAL_UART_RxCpltCallback 调用
int  door_rx_getchar(void);                        // 从环形缓冲读字节(-1=空)
const char* door_get_last_name(void);              // 最近开锁人名（供 MQTT 上报）
void door_uart_error_cplt(UART_HandleTypeDef* huart);  // 溢出(ORE)后重启接收

#endif
