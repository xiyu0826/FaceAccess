#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

#include <stdint.h>
extern uint32_t SystemCoreClock;

/* 基础配置 */
#define configUSE_PREEMPTION            1
#define configUSE_IDLE_HOOK             0
#define configUSE_TICK_HOOK             0
#define configCPU_CLOCK_HZ              ( SystemCoreClock )
#define configTICK_RATE_HZ              ( ( TickType_t ) 1000 )
#define configMAX_PRIORITIES            ( 7 )
#define configMINIMAL_STACK_SIZE        ( ( uint16_t ) 128 )
#define configTOTAL_HEAP_SIZE           ( ( size_t ) ( 16 * 1024 ) )   /* 8K->16K，容纳 6 个任务(door_task) */
#define configMAX_TASK_NAME_LEN         ( 16 )
#define configUSE_TRACE_FACILITY        1
#define configUSE_16_BIT_TICKS          0
#define configIDLE_SHOULD_YIELD         1
#define configUSE_MUTEXES               1
#define configQUEUE_REGISTRY_SIZE       8
#define configCHECK_FOR_STACK_OVERFLOW  0
#define configUSE_RECURSIVE_MUTEXES     1
#define configUSE_MALLOC_FAILED_HOOK    0
#define configUSE_APPLICATION_TASK_TAG  0
#define configUSE_COUNTING_SEMAPHORES   1
#define configGENERATE_RUN_TIME_STATS   0

/* 协程 */
#define configUSE_CO_ROUTINES           0
#define configMAX_CO_ROUTINE_PRIORITIES ( 2 )

/* 软件定时器 */
#define configUSE_TIMERS                0
#define configTIMER_TASK_PRIORITY       ( 2 )
#define configTIMER_QUEUE_LENGTH        10
#define configTIMER_TASK_STACK_DEPTH    ( configMINIMAL_STACK_SIZE * 2 )

/* API 函数 */
#define INCLUDE_vTaskPrioritySet        1
#define INCLUDE_uxTaskPriorityGet       1
#define INCLUDE_vTaskDelete             1
#define INCLUDE_vTaskCleanUpResources   0
#define INCLUDE_vTaskSuspend            1
#define INCLUDE_vTaskDelayUntil         1
#define INCLUDE_vTaskDelay              1
#define INCLUDE_xQueueGetMutexHolder    1
#define INCLUDE_xTaskGetSchedulerState  1

/* Cortex-M3 中断优先级 */
#ifdef __NVIC_PRIO_BITS
    #define configPRIO_BITS             __NVIC_PRIO_BITS
#else
    #define configPRIO_BITS             4
#endif

#define configLIBRARY_LOWEST_INTERRUPT_PRIORITY         0xf
#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY    5

#define configKERNEL_INTERRUPT_PRIORITY         ( configLIBRARY_LOWEST_INTERRUPT_PRIORITY << (8 - configPRIO_BITS) )
#define configMAX_SYSCALL_INTERRUPT_PRIORITY    ( configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY << (8 - configPRIO_BITS) )

#define configASSERT( x ) if( ( x ) == 0 ) { taskDISABLE_INTERRUPTS(); for( ;; ); }

/* 映射 FreeRTOS 端口中断处理到 CMSIS 标准名 */
#define vPortSVCHandler    SVC_Handler
#define xPortPendSVHandler PendSV_Handler

/* 注意: 不要定义 xPortSysTickHandler, 由用户自己的 SysTick_Handler 里同时调用
   HAL_IncTick() 和 xPortSysTickHandler() */

#endif /* FREERTOS_CONFIG_H */
