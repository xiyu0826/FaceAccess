#include "uart_debug.h"

UART_HandleTypeDef huart1;

void uart_debug_init(void)
{
    GPIO_InitTypeDef gpio = {0};

    /* 1. 使能时钟 */
    __HAL_RCC_USART1_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();

    /* 2. PA9(TX) 推挽复用输出 */
    gpio.Mode      = GPIO_MODE_AF_PP;
    gpio.Speed     = GPIO_SPEED_FREQ_HIGH;
    gpio.Pin       = GPIO_PIN_9;
    HAL_GPIO_Init(GPIOA, &gpio);

    /* 3. PA10(RX) 浮空输入 */
    gpio.Mode      = GPIO_MODE_INPUT;
    gpio.Pull      = GPIO_PULLUP;
    gpio.Pin       = GPIO_PIN_10;
    HAL_GPIO_Init(GPIOA, &gpio);

    /* 4. UART1 初始化: 115200-8-N-1 */
    huart1.Instance          = USART1;
    huart1.Init.BaudRate     = 115200;
    huart1.Init.WordLength   = UART_WORDLENGTH_8B;
    huart1.Init.StopBits     = UART_STOPBITS_1;
    huart1.Init.Parity       = UART_PARITY_NONE;
    huart1.Init.Mode         = UART_MODE_TX_RX;
    huart1.Init.HwFlowCtl    = UART_HWCONTROL_NONE;
    huart1.Init.OverSampling = UART_OVERSAMPLING_16;

    HAL_UART_Init(&huart1);
}

/*
 * 重定向 printf → UART1
 * 方式1: Keil 勾选 MicroLIB + 实现 fputc()
 * 方式2: 实现 __io_putchar() + _write()
 */

/* MicroLIB 方式: 重写 fputc */
int fputc(int ch, FILE *f)
{
    (void)f;
    HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, 0xFFFF);
    return ch;
}

/* 标准库方式: 重写 __io_putchar (AC5) 或 _write (AC6) */
int __io_putchar(int ch)
{
    HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, 0xFFFF);
    return ch;
}

/*
 * MicroLIB stdin 重定向: fgetc() → UART1 RX
 * 使 getchar() / scanf() 可以从串口读取
 */
int fgetc(FILE *f)
{
    (void)f;
    uint8_t ch;
    /* 阻塞等待一个字符 */
    while (HAL_UART_Receive(&huart1, &ch, 1, 0xFFFF) != HAL_OK);
    return (int)ch;
}

/* 非阻塞接收: -1 = 无数据 */
int uart_getchar(void)
{
    uint8_t ch;
    if (HAL_UART_Receive(&huart1, &ch, 1, 10) == HAL_OK)
        return ch;
    return -1;
}

/* 阻塞接收: 一直等到有数据 */
int uart_getchar_blocking(void)
{
    uint8_t ch;
    while (HAL_UART_Receive(&huart1, &ch, 1, 0xFFFF) != HAL_OK);
    return (int)ch;
}

/* 发送原始数据 */
void uart_send(const uint8_t *data, uint16_t len)
{
    HAL_UART_Transmit(&huart1, (uint8_t *)data, len, 0xFFFF);
}
