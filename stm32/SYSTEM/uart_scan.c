#include "uart_scan.h"
#include "uart_debug.h"
#include <stdio.h>
#include <string.h>

static UART_HandleTypeDef huart;

static void uart2_setup(void)
{
    GPIO_InitTypeDef g = {0};
    __HAL_RCC_USART2_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();

    /* UART2: PA2=TX, PA3=RX */
    g.Mode = GPIO_MODE_AF_PP; g.Speed = GPIO_SPEED_FREQ_HIGH;
    g.Pin = GPIO_PIN_2;  HAL_GPIO_Init(GPIOA, &g);
    g.Mode = GPIO_MODE_INPUT; g.Pull = GPIO_PULLUP;
    g.Pin = GPIO_PIN_3;  HAL_GPIO_Init(GPIOA, &g);

    /* 初始化可能用作蓝牙使能的引脚: PA4, PA5, PA6, PC13 */
    g.Mode  = GPIO_MODE_OUTPUT_PP;
    g.Speed = GPIO_SPEED_FREQ_LOW;
    g.Pull  = GPIO_NOPULL;
    g.Pin   = GPIO_PIN_4;  HAL_GPIO_Init(GPIOA, &g);
    g.Pin   = GPIO_PIN_5;  HAL_GPIO_Init(GPIOA, &g);
    g.Pin   = GPIO_PIN_6;  HAL_GPIO_Init(GPIOA, &g);
    g.Pin   = GPIO_PIN_13; HAL_GPIO_Init(GPIOC, &g);

    /* 全部拉高 */
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
}

void uart_scan_all(void)
{
    uint32_t bauds[] = {1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200, 230400};
    uint8_t rx[256];
    int b;

    printf("\r\n");
    printf("============================================\r\n");
    printf("  UART2 Deep Scan - Find Bluetooth\r\n");
    printf("============================================\r\n\r\n");

    uart2_setup();

    printf("Trying with EN pins HIGH (PA4/PA5/PA6/PC13=1):\r\n\r\n");

    for (b = 0; b < 9; b++)
    {
        huart.Instance          = USART2;
        huart.Init.BaudRate     = bauds[b];
        huart.Init.WordLength   = UART_WORDLENGTH_8B;
        huart.Init.StopBits     = UART_STOPBITS_1;
        huart.Init.Parity       = UART_PARITY_NONE;
        huart.Init.Mode         = UART_MODE_TX_RX;
        huart.Init.HwFlowCtl    = UART_HWCONTROL_NONE;
        huart.Init.OverSampling = UART_OVERSAMPLING_16;
        HAL_UART_Init(&huart);

        printf("  %6lu baud: ", bauds[b]);

        /* 发几次 AT */
        HAL_UART_Transmit(&huart, (uint8_t *)"AT\r\n", 4, 500);
        HAL_Delay(100);
        memset(rx, 0, sizeof(rx));
        HAL_UART_Receive(&huart, rx, 128, 500);

        if (rx[0] != 0)
        {
            int i;
            for (i = 0; rx[i] && i < 60; i++)
            {
                if (rx[i] >= 0x20 && rx[i] <= 0x7E)
                    printf("%c", rx[i]);
                else if (rx[i] == '\r' || rx[i] == '\n')
                    printf(".");
                else
                    printf("[%02X]", rx[i]);
            }
            if (strstr((char *)rx, "OK"))
                printf("  <-- OK!!!");
        }
        else
        {
            printf("silent");
        }
        printf("\r\n");
    }

    /* 再试一次, 拉低 EN */
    printf("\r\nTrying with EN pins LOW:\r\n\r\n");
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);

    for (b = 0; b < 9; b++)
    {
        huart.Init.BaudRate = bauds[b];
        HAL_UART_Init(&huart);

        printf("  %6lu baud: ", bauds[b]);

        HAL_UART_Transmit(&huart, (uint8_t *)"AT\r\n", 4, 500);
        HAL_Delay(100);
        memset(rx, 0, sizeof(rx));
        HAL_UART_Receive(&huart, rx, 128, 500);

        if (rx[0] != 0)
        {
            int i;
            for (i = 0; rx[i] && i < 60; i++)
            {
                if (rx[i] >= 0x20 && rx[i] <= 0x7E)
                    printf("%c", rx[i]);
                else if (rx[i] == '\r' || rx[i] == '\n')
                    printf(".");
                else
                    printf("[%02X]", rx[i]);
            }
            if (strstr((char *)rx, "OK"))
                printf("  <-- OK!!!");
        }
        else
        {
            printf("silent");
        }
        printf("\r\n");
    }

    printf("\r\n============================================\r\n");
    printf("If still silent: check H15 jumpers.\r\n");
    printf("Remove H15 jumpers to disconnect CH340.\r\n");
    printf("============================================\r\n");
}
