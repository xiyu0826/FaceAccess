#include "beep.h"
#include "delay.h"

static int beep_cur_state = 0;

void beep_init(void)
{
    GPIO_InitTypeDef GPIO_Init = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();

    GPIO_Init.Mode  = GPIO_MODE_OUTPUT_PP;
    GPIO_Init.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_Init.Pull  = GPIO_NOPULL;
    GPIO_Init.Pin   = BEEP_GPIO_PIN;

    HAL_GPIO_Init(BEEP_GPIO_PORT, &GPIO_Init);

    HAL_GPIO_WritePin(BEEP_GPIO_PORT, BEEP_GPIO_PIN, GPIO_PIN_RESET);
    beep_cur_state = 0;
}

/*
 * 无源蜂鸣器驱动：软件产生 ~3kHz 方波
 * 3kHz 周期 = 333us，半周期 = 167us
 */
void beep_burst(uint32_t ms)
{
    uint32_t cycles = ms * 3;   /* 3kHz -> 每 ms 3 个周期 */
    uint32_t i;

    for (i = 0; i < cycles; i++)
    {
        HAL_GPIO_WritePin(BEEP_GPIO_PORT, BEEP_GPIO_PIN, GPIO_PIN_SET);
        delay_us(167);
        HAL_GPIO_WritePin(BEEP_GPIO_PORT, BEEP_GPIO_PIN, GPIO_PIN_RESET);
        delay_us(167);
    }
    beep_cur_state = 0;
}

void beep_on(void)
{
    HAL_GPIO_WritePin(BEEP_GPIO_PORT, BEEP_GPIO_PIN, GPIO_PIN_SET);
    beep_cur_state = 1;
}

void beep_off(void)
{
    HAL_GPIO_WritePin(BEEP_GPIO_PORT, BEEP_GPIO_PIN, GPIO_PIN_RESET);
    beep_cur_state = 0;
}

void beep_toggle(void)
{
    HAL_GPIO_TogglePin(BEEP_GPIO_PORT, BEEP_GPIO_PIN);
    beep_cur_state = !beep_cur_state;
}

int beep_state(void)
{
    return beep_cur_state;
}
