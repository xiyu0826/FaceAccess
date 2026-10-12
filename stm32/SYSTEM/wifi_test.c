#include "wifi_test.h"
#include "uart_debug.h"
#include "door.h"
#include "mqtt.h"
#include "delay.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static UART_HandleTypeDef huart_wifi;

/* ---- UART 接收环形缓冲区 (中断累积, 可靠不丢数据) ------------ */
#define WIFI_RX_BUF_SIZE 1024
static uint8_t  wifi_rx_buf[WIFI_RX_BUF_SIZE];
static volatile uint16_t wifi_rx_head = 0;
static volatile uint16_t wifi_rx_tail = 0;
static uint8_t  wifi_rx_byte;

/* 缓冲区剩余可读字节数 */
static uint16_t wifi_rx_available(void)
{
    return (uint16_t)((wifi_rx_head - wifi_rx_tail) % WIFI_RX_BUF_SIZE);
}

/* UART 接收完成回调 (中断中, 每收 1 字节调用一次) */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART3)
    {
        uint16_t next = (uint16_t)((wifi_rx_head + 1) % WIFI_RX_BUF_SIZE);
        if (next != wifi_rx_tail)   /* 未满则入队 */
        {
            wifi_rx_buf[wifi_rx_head] = wifi_rx_byte;
            wifi_rx_head = next;
        }
        HAL_UART_Receive_IT(&huart_wifi, &wifi_rx_byte, 1);  /* 继续接收 */
    }
    else if (huart->Instance == USART1)
    {
        door_uart_rx_cplt(huart);   /* 门禁命令：入队 + 继续接收 */
    }
}

/* UART 错误回调：溢出(ORE)后清标志并重启接收，否则接收会永久死掉 */
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1)
    {
        door_uart_error_cplt(huart);
    }
    else if (huart->Instance == USART3)
    {
        __HAL_UART_CLEAR_OREFLAG(huart);
        HAL_UART_Receive_IT(huart, &wifi_rx_byte, 1);
    }
}

void wifi_test_init(void)
{
    GPIO_InitTypeDef g = {0};
    __HAL_RCC_USART3_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    g.Mode  = GPIO_MODE_AF_PP;  g.Speed = GPIO_SPEED_FREQ_HIGH;
    g.Pin   = GPIO_PIN_10;      HAL_GPIO_Init(GPIOB, &g);
    g.Mode  = GPIO_MODE_INPUT;  g.Pull = GPIO_PULLUP;
    g.Pin   = GPIO_PIN_11;      HAL_GPIO_Init(GPIOB, &g);

    huart_wifi.Instance          = USART3;
    huart_wifi.Init.BaudRate     = 115200;
    huart_wifi.Init.WordLength   = UART_WORDLENGTH_8B;
    huart_wifi.Init.StopBits     = UART_STOPBITS_1;
    huart_wifi.Init.Parity       = UART_PARITY_NONE;
    huart_wifi.Init.Mode         = UART_MODE_TX_RX;
    huart_wifi.Init.HwFlowCtl    = UART_HWCONTROL_NONE;
    huart_wifi.Init.OverSampling = UART_OVERSAMPLING_16;
    HAL_UART_Init(&huart_wifi);

    /* 开启 UART3 接收中断 (数据累积到环形缓冲区) */
    HAL_NVIC_SetPriority(USART3_IRQn, 6, 0);
    HAL_NVIC_EnableIRQ(USART3_IRQn);
    HAL_UART_Receive_IT(&huart_wifi, &wifi_rx_byte, 1);
}

static int wifi_recv(uint8_t *buf, uint16_t maxlen, uint32_t timeout_ms);

static void send_at(const char *cmd, int timeout_ms)
{
    uint8_t buf[512];
    int i;

    printf("\r\n>>> %s", cmd);
    HAL_UART_Transmit(&huart_wifi, (uint8_t *)cmd, strlen(cmd), 2000);

    /* 从环形缓冲区读响应 */
    memset(buf, 0, sizeof(buf));
    wifi_recv(buf, sizeof(buf)-1, timeout_ms);

    printf("\r\n");
    for (i = 0; i < 500 && buf[i]; i++)
    {
        if (buf[i] >= 0x20 && buf[i] <= 0x7E) printf("%c", buf[i]);
        else if (buf[i]=='\r') ;
        else if (buf[i]=='\n') printf("\r\n");
        else printf("[%02X]", buf[i]);
    }
}

void wifi_test_at(void)
{
    printf("\r\n========== ESP8266 WiFi Test ==========");

    send_at("AT\r\n", 300);
    send_at("AT+GMR\r\n", 500);
    send_at("AT+CWLAP\r\n", 5000);

    printf("\r\n================================\r\n");
}

void wifi_connect(void)
{
    printf("\r\n========== ESP8266 AP Mode ==========\r\n");

    /* 开 AP 热点: 名字 STM32_SmartHome, 密码 12345678 */
    send_at("AT+CWMODE=2\r\n", 500);
    send_at("AT+CWSAP=\"STM32_Home\",\"12345678\",1,4\r\n", 1000);

    printf("\r\nNow check your phone WiFi list!\r\n");
    printf("Look for:  STM32_Home\r\n");
    printf("Password:  12345678\r\n");
    printf("\r\n================================\r\n");
}

/* UART3 中断处理 (在 USART3_IRQHandler 中调用) */
void wifi_uart_irq_handler(void)
{
    HAL_UART_IRQHandler(&huart_wifi);
}

void wifi_join(const char *ssid, const char *pwd)
{
    char cmd[128];

    printf("\r\n========== ESP8266 Join WiFi ==========\r\n");

    /* 0. 复位模块, 清除残留的连接状态 */
    send_at("AT+RST\r\n", 2000);
    delay_ms(2000);   /* 等复位完成 */

    /* 1. Station 模式 */
    send_at("AT+CWMODE=1\r\n", 500);

    /* 2. 连接 WiFi (手机热点) */
    snprintf(cmd, sizeof(cmd), "AT+CWJAP=\"%s\",\"%s\"\r\n", ssid, pwd);
    send_at(cmd, 8000);   /* 连接需要几秒, 超时给足 */

    /* 3. 查询 IP, 确认连接成功 */
    send_at("AT+CIFSR\r\n", 1000);

    printf("\r\n================================\r\n");
}

/* ---- 接收响应到 buf, 返回收到的字节数 ------------------------- */
static int wifi_recv(uint8_t *buf, uint16_t maxlen, uint32_t timeout_ms)
{
    uint32_t start = HAL_GetTick();
    uint16_t n = 0;
    memset(buf, 0, maxlen);

    /* 从环形缓冲区读, 直到缓冲区满或总超时 */
    while (n < maxlen - 1 && (HAL_GetTick() - start) < timeout_ms)
    {
        if (wifi_rx_available() > 0)
        {
            buf[n++] = wifi_rx_buf[wifi_rx_tail];
            wifi_rx_tail = (uint16_t)((wifi_rx_tail + 1) % WIFI_RX_BUF_SIZE);
        }
    }
    return (int)n;
}

void wifi_tcp_connect(const char *ip, uint16_t port)
{
    char cmd[64];

    /* 先关闭可能存在的旧连接 */
    send_at("AT+CIPCLOSE\r\n", 500);

    /* 单连接模式 */
    send_at("AT+CIPMUX=0\r\n", 500);

    snprintf(cmd, sizeof(cmd), "AT+CIPSTART=\"TCP\",\"%s\",%d\r\n", ip, port);
    send_at(cmd, 5000);
}

void wifi_tcp_send(const uint8_t *data, uint16_t len)
{
    char cmd[32];
    uint8_t rx[128];
    int nr;

    /* 1. 发 CIPSEND 命令, 等待 ">" 提示 */
    snprintf(cmd, sizeof(cmd), "AT+CIPSEND=%d\r\n", len);
    HAL_UART_Transmit(&huart_wifi, (uint8_t *)cmd, strlen(cmd), 2000);
    nr = wifi_recv(rx, sizeof(rx), 300);    /* 等 ">" (响应很快) */
    if (nr > 0) mqtt_rx_accumulate(rx, nr);   /* 累积可能混入的 +IPD */

    /* 2. 发原始数据 */
    HAL_UART_Transmit(&huart_wifi, (uint8_t *)data, len, 3000);

    /* 3. 读 SEND OK (可能混有 +IPD 下发的数据) */
    nr = wifi_recv(rx, sizeof(rx), 500);
    if (nr > 0) mqtt_rx_accumulate(rx, nr);   /* 累积下发的指令 */
    printf("<< ");
    {
        int i;
        for (i = 0; i < 128 && rx[i]; i++)
        {
            if (rx[i] >= 0x20 && rx[i] <= 0x7E) printf("%c", rx[i]);
            else if (rx[i] == '\r' || rx[i] == '\n') printf(" ");
            else printf("[%02X]", rx[i]);
        }
    }
    printf("\r\n");
}

void wifi_tcp_close(void)
{
    send_at("AT+CIPCLOSE\r\n", 500);
}

/* ---- 非阻塞读取收到的数据 (从环形缓冲区) ------------------ */
uint16_t wifi_poll(uint8_t *buf, uint16_t maxlen)
{
    uint16_t n = 0;

    while (wifi_rx_available() > 0 && n < maxlen - 1)
    {
        buf[n++] = wifi_rx_buf[wifi_rx_tail];
        wifi_rx_tail = (uint16_t)((wifi_rx_tail + 1) % WIFI_RX_BUF_SIZE);
    }
    buf[n] = '\0';
    return n;
}

/* ---- 缓存收到的 +IPD 数据 --------------------------------- */
static uint8_t wifi_ipd_buf[128];
static uint16_t wifi_ipd_len = 0;

void wifi_extract_ipd(const uint8_t *rx)
{
    char *p = strstr((char *)rx, "+IPD,");
    int len;
    if (!p) return;
    p += 5;
    len = atoi(p);
    p = strchr(p, ':');
    if (!p) return;
    p++;
    if (len <= 0 || len >= (int)sizeof(wifi_ipd_buf)) return;
    memcpy(wifi_ipd_buf, p, len);
    wifi_ipd_len = (uint16_t)len;
    printf("[IPD] %d bytes: ", len);
    {
        int k;
        for (k = 0; k < len && k < 32; k++)
            printf("%02X ", wifi_ipd_buf[k]);
    }
    printf("\r\n");
}

uint16_t wifi_get_ipd(uint8_t *buf, uint16_t maxlen)
{
    uint16_t n;
    if (wifi_ipd_len == 0) return 0;
    n = wifi_ipd_len < maxlen ? wifi_ipd_len : maxlen;
    memcpy(buf, wifi_ipd_buf, n);
    wifi_ipd_len = 0;
    return n;
}
