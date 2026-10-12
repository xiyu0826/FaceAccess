#include "delay.h"

/*
 * 延时 (适配 FreeRTOS)
 *
 * delay_ms(): 使用 HAL_Delay (基于 SysTick, 与 FreeRTOS 共用 1ms 时基)
 * delay_us(): 使用 DWT 硬件计数器 (不占用/不重配 SysTick)
 *
 * 注意: SysTick_Handler 已移至 stm32f1xx_it.c,
 *       其中同时调用 HAL_IncTick() 和 xPortSysTickHandler()
 */

/* 毫秒延时 (FreeRTOS 下用 HAL_Delay, 保持 1ms 时基) */
void delay_ms(uint16_t ms)
{
    HAL_Delay(ms);
}

/* 微秒延时 (DWT, 不碰 SysTick) */
void delay_us(uint16_t us)
{
    uint32_t start, ticks;

    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0;
    DWT->CTRL   |= DWT_CTRL_CYCCNTENA_Msk;

    start = DWT->CYCCNT;
    ticks = us * (SystemCoreClock / 1000000);
    while ((DWT->CYCCNT - start) < ticks);
}
