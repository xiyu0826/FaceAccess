#include "key.h"
#include "delay.h"

static int key_state[4] = {1, 1, 1, 1};   // 历史状态，1=释放，0=按下

void key_init(void)
{
    GPIO_InitTypeDef GPIO_Init = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();

    GPIO_Init.Mode  = GPIO_MODE_INPUT;
    GPIO_Init.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_Init.Pull  = GPIO_PULLUP;          // 外部已有 10k 上拉，内部也配置上拉

    GPIO_Init.Pin = KEY0_GPIO_PIN;
    HAL_GPIO_Init(KEY0_GPIO_PORT, &GPIO_Init);
    GPIO_Init.Pin = KEY1_GPIO_PIN;
    HAL_GPIO_Init(KEY1_GPIO_PORT, &GPIO_Init);
    GPIO_Init.Pin = KEY2_GPIO_PIN;
    HAL_GPIO_Init(KEY2_GPIO_PORT, &GPIO_Init);
    GPIO_Init.Pin = KEY3_GPIO_PIN;
    HAL_GPIO_Init(KEY3_GPIO_PORT, &GPIO_Init);
}

int key_read(int key_num)
{
    GPIO_PinState st;
    switch(key_num)
    {
        case KEY0: st = HAL_GPIO_ReadPin(KEY0_GPIO_PORT, KEY0_GPIO_PIN); break;
        case KEY1: st = HAL_GPIO_ReadPin(KEY1_GPIO_PORT, KEY1_GPIO_PIN); break;
        case KEY2: st = HAL_GPIO_ReadPin(KEY2_GPIO_PORT, KEY2_GPIO_PIN); break;
        case KEY3: st = HAL_GPIO_ReadPin(KEY3_GPIO_PORT, KEY3_GPIO_PIN); break;
        default:   return KEY_RELEASED;
    }
    return (st == GPIO_PIN_RESET) ? KEY_PRESSED : KEY_RELEASED;
}

int key_pressed(int key_num)
{
    int cur = key_read(key_num);

    if(cur == KEY_PRESSED && key_state[key_num] == KEY_RELEASED)
    {
        delay_ms(15);                        // 消抖
        if(key_read(key_num) == KEY_PRESSED)
        {
            key_state[key_num] = KEY_PRESSED;
            return 1;                        // 按下沿有效
        }
    }
    else if(cur == KEY_RELEASED)
    {
        key_state[key_num] = KEY_RELEASED;
    }
    return 0;
}

int key_released(int key_num)
{
    int cur = key_read(key_num);

    if(cur == KEY_RELEASED && key_state[key_num] == KEY_PRESSED)
    {
        delay_ms(15);                        // 消抖
        if(key_read(key_num) == KEY_RELEASED)
        {
            key_state[key_num] = KEY_RELEASED;
            return 1;                        // 释放沿有效
        }
    }
    else if(cur == KEY_PRESSED)
    {
        key_state[key_num] = KEY_PRESSED;
    }
    return 0;
}
