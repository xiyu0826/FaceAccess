/**
  ******************************************************************************
  * @file    stm32f1xx_it.c
  * @brief   Interrupt Service Routines.
  ******************************************************************************
  */

#include "stm32f1xx_hal.h"
#include "stm32f1xx_it.h"
#include "FreeRTOS.h"
#include "task.h"
#include "wifi_test.h"
#include "door.h"

extern void xPortSysTickHandler(void);   /* FreeRTOS 端口 SysTick 处理 (port.c) */

/******************************************************************************/
/*            Cortex-M3 Processor Exceptions Handlers                         */
/******************************************************************************/

void NMI_Handler(void) { }

void HardFault_Handler(void)
{
    while (1) { }
}

void MemManage_Handler(void)
{
    while (1) { }
}

void BusFault_Handler(void)
{
    while (1) { }
}

void UsageFault_Handler(void)
{
    while (1) { }
}

/*
 * SVC_Handler 和 PendSV_Handler 由 FreeRTOS 的 port.c 提供
 * (FreeRTOSConfig.h 中已用 #define 映射), 此处不再重复定义。
 */

void DebugMon_Handler(void) { }

/*
 * SysTick: 同时服务 HAL (1ms 时基) 和 FreeRTOS (任务调度)
 */
void SysTick_Handler(void)
{
    HAL_IncTick();
    if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED)
    {
        xPortSysTickHandler();
    }
}

/******************************************************************************/
/*                 STM32F1xx Peripherals Interrupt Handlers                   */
/*  后续按需添加: EXTI / UART / DMA / TIM 等中断处理                           */
/******************************************************************************/

/* PIR 传感器中断 (PB12 → EXTI line 12 → EXTI15_10) */
void EXTI15_10_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_12);
}

/* ESP8266 WiFi 串口中断 (USART3) */
void USART3_IRQHandler(void)
{
    wifi_uart_irq_handler();
}

/* 门禁串口中断 (USART1, CH340) */
void USART1_IRQHandler(void)
{
    door_uart_irq_handler();
}
