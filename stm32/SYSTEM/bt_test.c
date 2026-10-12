/**
  ******************************************************************************
  * @file           : bt_test.c
  * @brief          : HC-05 Bluetooth Module Test
  *
  * HC-05 工作模式:
  *   AT 模式:  按住模块上黑色按钮, 38400 baud, 可收发 AT 指令
  *   透传模式: 松开按钮, 9600 baud, 手机配对后透明传输
  *
  * 配对: PIN = 1234, 连上后模块状态灯长亮
  ******************************************************************************
  */

#include "bt_test.h"
#include "uart_debug.h"
#include <stdio.h>
#include <string.h>

static UART_HandleTypeDef huart_bt;

void bt_test_init(void)
{
    GPIO_InitTypeDef gpio = {0};

    __HAL_RCC_USART2_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();

    /* UART2: PA2=TX, PA3=RX */
    gpio.Mode  = GPIO_MODE_AF_PP;
    gpio.Speed = GPIO_SPEED_FREQ_HIGH;
    gpio.Pin   = GPIO_PIN_2;
    HAL_GPIO_Init(GPIOA, &gpio);
    gpio.Mode  = GPIO_MODE_INPUT;
    gpio.Pull  = GPIO_PULLUP;
    gpio.Pin   = GPIO_PIN_3;
    HAL_GPIO_Init(GPIOA, &gpio);

    huart_bt.Instance          = USART2;
    huart_bt.Init.BaudRate     = 9600;
    huart_bt.Init.WordLength   = UART_WORDLENGTH_8B;
    huart_bt.Init.StopBits     = UART_STOPBITS_1;
    huart_bt.Init.Parity       = UART_PARITY_NONE;
    huart_bt.Init.Mode         = UART_MODE_TX_RX;
    huart_bt.Init.HwFlowCtl    = UART_HWCONTROL_NONE;
    huart_bt.Init.OverSampling = UART_OVERSAMPLING_16;
    HAL_UART_Init(&huart_bt);
}

/*
 * HC-05 AT 指令测试
 * 操作: 按住蓝牙模块上黑色按钮不放, 然后按数字键 '0'
 */
void bt_test_at(void)
{
    uint8_t rx[128];
    int i;

    /* 切到 AT 模式波特率 38400 */
    huart_bt.Init.BaudRate = 38400;
    HAL_UART_Init(&huart_bt);

    printf("\r\n");
    printf("============================================\r\n");
    printf("  HC-05 AT Command Test\r\n");
    printf("  UART2 PA2/PA3 @ 38400 (AT mode)\r\n");
    printf("============================================\r\n");
    printf("  PRESS and HOLD the black button\r\n");
    printf("  on the BT module NOW!\r\n");
    printf("============================================\r\n\r\n");

    /* 测试 AT */
    printf("[1] AT:\r\n");
    HAL_UART_Transmit(&huart_bt, (uint8_t *)"AT\r\n", 4, 500);
    HAL_Delay(500);
    memset(rx, 0, sizeof(rx));
    HAL_UART_Receive(&huart_bt, rx, 127, 500);
    printf("<<<");
    for (i = 0; rx[i] && i < 100; i++)
    {
        if (rx[i] >= 0x20 && rx[i] <= 0x7E) printf("%c", rx[i]);
        else if (rx[i] == '\r' || rx[i] == '\n') printf(".");
        else printf("[%02X]", rx[i]);
    }
    printf(">>>\r\n");

    /* AT+VERSION */
    printf("[2] AT+VERSION:\r\n");
    HAL_UART_Transmit(&huart_bt, (uint8_t *)"AT+VERSION?\r\n", 13, 500);
    HAL_Delay(500);
    memset(rx, 0, sizeof(rx));
    HAL_UART_Receive(&huart_bt, rx, 127, 500);
    printf("<<<");
    for (i = 0; rx[i] && i < 100; i++)
    {
        if (rx[i] >= 0x20 && rx[i] <= 0x7E) printf("%c", rx[i]);
        else if (rx[i] == '\r' || rx[i] == '\n') printf(".");
        else printf("[%02X]", rx[i]);
    }
    printf(">>>\r\n");

    /* AT+NAME */
    printf("[3] AT+NAME:\r\n");
    HAL_UART_Transmit(&huart_bt, (uint8_t *)"AT+NAME?\r\n", 10, 500);
    HAL_Delay(500);
    memset(rx, 0, sizeof(rx));
    HAL_UART_Receive(&huart_bt, rx, 127, 500);
    printf("<<<");
    for (i = 0; rx[i] && i < 100; i++)
    {
        if (rx[i] >= 0x20 && rx[i] <= 0x7E) printf("%c", rx[i]);
        else if (rx[i] == '\r' || rx[i] == '\n') printf(".");
        else printf("[%02X]", rx[i]);
    }
    printf(">>>\r\n");

    /* 切回 9600 */
    huart_bt.Init.BaudRate = 9600;
    HAL_UART_Init(&huart_bt);

    printf("\r\n============================================\r\n");
    printf("  You can release the button now.\r\n");
    printf("============================================\r\n");
}

/*
 * 透传桥接: PC串口 <-> UART2/HC-05 @ 9600
 * 松开按钮, 手机配对后即可双向通信
 * PIN: 1234
 */
void bt_bridge(void)
{
    uint8_t ch;

    /* 恢复透传模式: 9600 */
    huart_bt.Init.BaudRate = 9600;
    HAL_UART_Init(&huart_bt);

    printf("\r\n");
    printf("============================================\r\n");
    printf("  HC-05 Bridge Mode @ 9600\r\n");
    printf("  1. Open phone Bluetooth & scan\r\n");
    printf("  2. Pair with HC-05 (PIN: 1234)\r\n");
    printf("  3. Open BT Serial APP, connect\r\n");
    printf("  4. Type here <-> phone, both ways\r\n");
    printf("  Press 'q' to exit\r\n");
    printf("============================================\r\n\r\n");

    while (1)
    {
        /* BT -> PC */
        if (HAL_UART_Receive(&huart_bt, &ch, 1, 2) == HAL_OK)
            HAL_UART_Transmit(&huart1, &ch, 1, 100);

        /* PC -> BT */
        if (HAL_UART_Receive(&huart1, &ch, 1, 2) == HAL_OK)
        {
            if (ch == 'q' || ch == 'Q') break;
            HAL_UART_Transmit(&huart_bt, &ch, 1, 100);
        }
    }

    printf("\r\nBridge exited.\r\n");
}
