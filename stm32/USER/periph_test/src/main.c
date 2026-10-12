#include "stm32f1xx_hal.h"
#include "led.h"
#include "key.h"
#include "beep.h"
#include "delay.h"

/**
  * @brief  System Clock Configuration
  *         System Clock source            = PLL (HSE)
  *         SYSCLK(Hz)                     = 72000000
  *         HCLK(Hz)                       = 72000000
  *         AHB Prescaler                  = 1
  *         APB1 Prescaler                 = 2
  *         APB2 Prescaler                 = 1
  *         PLLMUL                         = 9
  *         Flash Latency(WS)              = 2
  */
static void SystemClock_Config(void)
{
    RCC_OscInitTypeDef oscinitstruct = {0};
    RCC_ClkInitTypeDef clkinitstruct = {0};

    oscinitstruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    oscinitstruct.HSEState       = RCC_HSE_ON;
    oscinitstruct.HSIState       = RCC_HSI_ON;
    oscinitstruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
    oscinitstruct.PLL.PLLState   = RCC_PLL_ON;
    oscinitstruct.PLL.PLLSource  = RCC_PLLSOURCE_HSE;
    oscinitstruct.PLL.PLLMUL     = RCC_PLL_MUL9;   /* 8MHz x 9 = 72MHz */

    if (HAL_RCC_OscConfig(&oscinitstruct) != HAL_OK)
    {
        while (1);
    }

    clkinitstruct.ClockType = RCC_CLOCKTYPE_SYSCLK |
                              RCC_CLOCKTYPE_HCLK   |
                              RCC_CLOCKTYPE_PCLK1  |
                              RCC_CLOCKTYPE_PCLK2;

    clkinitstruct.SYSCLKSource   = RCC_SYSCLKSOURCE_PLLCLK;
    clkinitstruct.AHBCLKDivider  = RCC_SYSCLK_DIV1;
    clkinitstruct.APB1CLKDivider = RCC_HCLK_DIV2;
    clkinitstruct.APB2CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&clkinitstruct, FLASH_LATENCY_2) != HAL_OK)
    {
        while (1);
    }
}

/*
 * 开机自检：LED 闪烁 3 次 + 蜂鸣器响 3 次
 */
static void power_on_self_test(void)
{
    int i;

    /* 1. LED 闪烁 3 次 */
    for (i = 0; i < 3; i++)
    {
        led_all_on();
        delay_ms(200);
        led_all_off();
        delay_ms(200);
    }

    /* 2. 蜂鸣器一直 on（方便万用表测电压，排查线路问题） */
    beep_on();
}

int main(void)
{
    /* 1. 初始化 */
    HAL_Init();
    SystemClock_Config();   /* 72MHz */

    led_init();
    key_init();
    beep_init();

    /* 2. 开机自检 */
    power_on_self_test();

    /* 3. 按键交互测试主循环
     *    每个按键控制一个 LED，方便逐一验证
     *    KEY0 (U6) -> 翻转 LED0 (PA11 / LED6)
     *    KEY1 (U7) -> 翻转 LED1 (PA12 / LED7)
     *    KEY2 (U8) -> 翻转 LED2 (PA13 / LED8)
     *    KEY3 (U9) -> 翻转 LED3 (PA14 / LED9)
     */
    while (1)
    {
        if (key_pressed(KEY0))
        {
            led_toggle(LED0);
        }
        else if (key_pressed(KEY1))
        {
            led_toggle(LED1);
        }
        else if (key_pressed(KEY2))
        {
            led_toggle(LED2);
        }
        else if (key_pressed(KEY3))
        {
            led_toggle(LED3);
        }

        delay_ms(50);   /* 轮询间隔，降低 CPU 占用 */
    }
}
