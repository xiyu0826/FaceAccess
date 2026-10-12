#include "pir.h"
#include "uart_debug.h"
#include <stdio.h>

/*
 * HC-SR501 接 H8_11 = PB12
 *
 * PB12 = SPI2_NSS (SPI Flash CS), 本项目不用 SPI Flash
 * PB12 为 FT 5V 耐受脚, PIR 模块 5V 输出安全
 * 中断: EXTI line 12 → EXTI15_10_IRQHandler
 */

/* ---- 静态变量 ---------------------------------------------- */
static pir_callback_t pir_cb = NULL;    /* 用户回调 */

/* ---- 关闭 JTAG (无操作, PA4 不需要) ------------------------- */
void pir_disable_jtag(void)
{
    /* PA4 是普通 GPIO, 不需要 JTAG 重映射。保留此函数兼容旧接口。 */
}

/* ---- EXTI 中断初始化 --------------------------------------- */
void pir_init(void)
{
    GPIO_InitTypeDef gpio = {0};

    /* 1. 使能时钟 */
    __HAL_RCC_GPIOB_CLK_ENABLE();

    /* 2. PB12: 下拉输入, 上升沿中断 (FT 5V耐受) */
    gpio.Mode = GPIO_MODE_IT_RISING;
    gpio.Pull = GPIO_PULLDOWN;
    gpio.Pin  = GPIO_PIN_12;
    HAL_GPIO_Init(GPIOB, &gpio);

    /* 3. PB12 → EXTI line 12 → EXTI15_10_IRQn */
    HAL_NVIC_SetPriority(EXTI15_10_IRQn, 2, 0);
    HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

    printf("[PIR] HC-SR501 on PB12 (H8_11), EXTI12 rising edge ready.\r\n");
}

/* ---- 注册回调 ---------------------------------------------- */
void pir_set_callback(pir_callback_t cb)
{
    pir_cb = cb;
}

/* ---- 中断处理入口 (在 HAL_GPIO_EXTI_Callback 中调用) --------- */
void pir_handle_interrupt(void)
{
    if (pir_cb)
        pir_cb();
}

/* ---- 当前是否有人 ------------------------------------------ */
int pir_is_present(void)
{
    return (GPIOB->IDR & GPIO_PIN_12) != 0;
}

/* ---- 测试: 持续监测 30 秒 ----------------------------------- */
void pir_test(void)
{
    uint32_t start = HAL_GetTick();

    printf("\r\n========== PIR Human Detection Test (30s) ==========\r\n");
    printf("  Move in front of the sensor...\r\n");
    printf("  Current state: %s\r\n", pir_is_present() ? "DETECTED!" : "no one");
    printf("  Press 'q' to quit early.\r\n");

    while (1)
    {
        int c = uart_getchar();
        if (c == 'q' || c == 'Q') break;

        if (HAL_GetTick() - start > 30000)
        {
            printf("\r\n30 seconds elapsed. Test done.\r\n");
            break;
        }

        if (pir_is_present())
        {
            printf("  [%5lu ms] Human detected!\r\n", HAL_GetTick());
            HAL_Delay(2000);
        }

        HAL_Delay(100);
    }
    printf("====================================================\r\n");
}

/*
 * 在 main.c 或 stm32f1xx_it.c 中覆盖 HAL_GPIO_EXTI_Callback:
 *
 *   void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
 *   {
 *       if (GPIO_Pin == GPIO_PIN_12)
 *           pir_handle_interrupt();
 *   }
 */
