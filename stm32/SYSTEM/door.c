#include "door.h"
#include "servo.h"
#include "beep.h"
#include "led.h"
#include "uart_debug.h"
#include "FreeRTOS.h"
#include "task.h"
#include <string.h>

/* ---- UART1 接收环形缓冲区 (中断累积, 可靠不丢数据) ---- */
#define DOOR_RX_BUF_SIZE 128
static uint8_t  door_rx_buf[DOOR_RX_BUF_SIZE];
static volatile uint16_t door_rx_head = 0;
static volatile uint16_t door_rx_tail = 0;
static uint8_t  door_rx_byte;

/* 初始化 UART1 接收中断 (UART1 本身已在 uart_debug_init 配好 115200-8-N-1) */
void door_uart_init(void)
{
    HAL_NVIC_SetPriority(USART1_IRQn, 6, 0);
    HAL_NVIC_EnableIRQ(USART1_IRQn);
    HAL_UART_Receive_IT(&huart1, &door_rx_byte, 1);
}

/* USART1 中断入口 (stm32f1xx_it.c 的 USART1_IRQHandler 调用) */
void door_uart_irq_handler(void)
{
    HAL_UART_IRQHandler(&huart1);
}

/* 每收到 1 字节由 HAL_UART_RxCpltCallback 调用：入队 + 继续接收 */
void door_uart_rx_cplt(UART_HandleTypeDef* huart)
{
    (void)huart;
    uint16_t next = (uint16_t)((door_rx_head + 1) % DOOR_RX_BUF_SIZE);
    if (next != door_rx_tail)                 /* 未满则入队，满了丢弃（防溢出） */
    {
        door_rx_buf[door_rx_head] = door_rx_byte;
        door_rx_head = next;
    }
    HAL_UART_Receive_IT(&huart1, &door_rx_byte, 1);   /* 继续接收下一字节 */
}

/* 从环形缓冲读一个字节，-1 = 空 */
int door_rx_getchar(void)
{
    if (door_rx_head == door_rx_tail)
        return -1;
    uint8_t c = door_rx_buf[door_rx_tail];
    door_rx_tail = (uint16_t)((door_rx_tail + 1) % DOOR_RX_BUF_SIZE);
    return c;
}

/* UART1 出错（如溢出 ORE）时清标志并重启接收，否则接收会永久死掉 */
void door_uart_error_cplt(UART_HandleTypeDef* huart)
{
    (void)huart;
    __HAL_UART_CLEAR_OREFLAG(&huart1);
    HAL_UART_Receive_IT(&huart1, &door_rx_byte, 1);
}

/* ---- 命令解析 ---- */
static void trim_line(char* line)
{
    int len = strlen(line);
    while (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r'))
        line[--len] = '\0';
}

/* 最近开门的人名（供 MQTT 上报到看板） */
static char door_last_name[32] = "";

const char* door_get_last_name(void)
{
    return door_last_name;
}

static void door_handle_command(char* line)
{
    trim_line(line);

    /* 识别 "OPEN" 或 "OPEN:名字" */
    if (strncmp(line, "OPEN", 4) == 0)
    {
        // 提取名字（"OPEN:名字" 冒号后的部分），用于 MQTT 上报
        const char* name = line + 4;
        while (*name == ':') name++;
        strncpy(door_last_name, name, sizeof(door_last_name) - 1);
        door_last_name[sizeof(door_last_name) - 1] = '\0';

        servo_open();                      // 舵机转 90° 开门
        led_ctrl(LED3, LED_ON);            // 门禁用 LED3，避开报警任务用的 LED0-2
        beep_burst(100);                   // 蜂鸣一声

        vTaskDelay(pdMS_TO_TICKS(2000));   // 门开 2 秒

        servo_close();                     // 舵机回 0° 关门
        led_ctrl(LED3, LED_OFF);           // 熄灭 LED3
    }
}

/* 门禁任务：轮询环形缓冲，累积字符直到换行，再解析命令 */
void door_task(void* arg)
{
    (void)arg;
    char line[64];
    int idx = 0;

    for (;;)
    {
        int c = door_rx_getchar();
        if (c >= 0)
        {
            if (c == '\n' || c == '\r')
            {
                if (idx > 0)
                {
                    line[idx] = '\0';
                    door_handle_command(line);
                    idx = 0;
                }
            }
            else if (idx < 63)
            {
                line[idx++] = (char)c;
            }
        }

        vTaskDelay(pdMS_TO_TICKS(20));
    }
}
