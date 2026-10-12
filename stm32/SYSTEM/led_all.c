#include "led_all.h"
#include "delay.h"
#include <stdio.h>

/*
 * 全板10颗LED 测试流水灯
 *
 * 注意: 板子LED6-9为共阳极 (低电平点亮)
 *       LED1 直接跨5V电源, 通电常亮, 不可控
 *       LED2-5 和 LED10 需要确认电路后才知道控制方式
 */

void led_all_init(void)
{
    GPIO_InitTypeDef gpio = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_AFIO_CLK_ENABLE();

    /* 关闭SWD, 释放 PA13/PA14 为普通 GPIO */
    __HAL_AFIO_REMAP_SWJ_DISABLE();

    /* 初始化 PA11/PA12/PA13/PA14 (LED6-9, 共阳, 低电平点亮) */
    gpio.Mode  = GPIO_MODE_OUTPUT_PP;
    gpio.Speed = GPIO_SPEED_FREQ_HIGH;
    gpio.Pull  = GPIO_NOPULL;

    gpio.Pin = GPIO_PIN_11;  HAL_GPIO_Init(GPIOA, &gpio);
    gpio.Pin = GPIO_PIN_12;  HAL_GPIO_Init(GPIOA, &gpio);
    gpio.Pin = GPIO_PIN_13;  HAL_GPIO_Init(GPIOA, &gpio);
    gpio.Pin = GPIO_PIN_14;  HAL_GPIO_Init(GPIOA, &gpio);

    /* 默认全灭 (输出高电平) */
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_13, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_14, GPIO_PIN_SET);

    printf("LEDs init OK (PA11-PA14, active-low).\r\n");
}

/*
 * 开机自检流水灯: LED0→LED1→LED2→LED3 依次闪烁3轮
 */
void led_all_self_test(void)
{
    int round, i;
    uint16_t pins[] = {GPIO_PIN_11, GPIO_PIN_12, GPIO_PIN_13, GPIO_PIN_14};
    char *names[]   = {"LED6(PA11)", "LED7(PA12)", "LED8(PA13)", "LED9(PA14)"};

    printf("\r\n--- LED Self-Test (3 rounds) ---\r\n");

    for (round = 0; round < 3; round++)
    {
        for (i = 0; i < 4; i++)
        {
            printf("  Round %d: %s ON\r\n", round + 1, names[i]);
            HAL_GPIO_WritePin(GPIOA, pins[i], GPIO_PIN_RESET);  /* 低电平点亮 */
            delay_ms(300);
            HAL_GPIO_WritePin(GPIOA, pins[i], GPIO_PIN_SET);    /* 高电平熄灭 */
        }
    }

    printf("LED self-test done.\r\n");
}

/*
 * led_all_on() / led_all_off() 已在 SYSTEM/led.c 中定义,
 * 此处不再重复定义, 直接复用 led.c 的实现.
 */
