#ifndef __WIFI_TEST_H__
#define __WIFI_TEST_H__

#include "stm32f1xx_hal.h"

/*
 * ESP8266 WiFi 模块 AT 指令测试
 *
 * 接口:   UART3  PB10=TX  PB11=RX
 *         H16 跳线: MCU-UART3 ↔ ESP8266
 * 波特率: 115200
 */

void wifi_test_init(void);
void wifi_test_at(void);          /* AT 基础测试 */
void wifi_connect(void);          /* 开 AP 热点 */
void wifi_join(const char *ssid, const char *pwd);  /* 连接指定 WiFi (Station模式) */
void wifi_uart_irq_handler(void); /* UART3 中断处理 (在 USART3_IRQHandler 中调用) */

/* TCP 通信 (供 MQTT 使用) */
void wifi_tcp_connect(const char *ip, uint16_t port);  /* 建立 TCP 连接 */
void wifi_tcp_send(const uint8_t *data, uint16_t len); /* 发送原始数据 */
void wifi_tcp_close(void);                             /* 关闭连接 */

/* 非阻塞读取收到的数据, 返回字节数 (0=无数据) */
uint16_t wifi_poll(uint8_t *buf, uint16_t maxlen);

/* 从响应中提取 +IPD 数据并缓存 */
void wifi_extract_ipd(const uint8_t *rx);

/* 读取缓存的 +IPD 数据 (读取后清空), 返回字节数 */
uint16_t wifi_get_ipd(uint8_t *buf, uint16_t maxlen);

#endif
