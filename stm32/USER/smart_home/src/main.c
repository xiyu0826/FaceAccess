/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Smart Home v1.0 - STM32F103RCT6 + FreeRTOS
  *                   环境监测与远程控制系统
  ******************************************************************************
  */

#include "stm32f1xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"
#include "uart_debug.h"
#include "delay.h"
#include "i2c_scanner.h"
#include "adc_test.h"
#include "oled.h"
#include "led_all.h"
#include "beep.h"
#include "servo.h"
#include "door.h"
#include "key.h"
#include "ds3231.h"
#include "dht11.h"
#include "mq135.h"
#include "light.h"
#include "pir.h"
#include "wifi_test.h"
#include "mqtt.h"
#include <stdio.h>
#include <string.h>

/* ---- WiFi 配置 ------------------------------------------------ */
/* 改成自己的 2.4G 热点名称与密码（ESP8266 不支持 5G 频段） */
#define WIFI_SSID   "your_wifi_ssid"
#define WIFI_PWD    "your_wifi_password"

/* ---- 菜单页定义 ---------------------------------------------- */
#define PAGE_MAIN   0
#define PAGE_ENV    1
#define PAGE_SYS    2
#define PAGE_COUNT  3

/* ---- 报警阈值 ------------------------------------------------ */
#define TEMP_HIGH      35
#define TEMP_LOW       10
#define HUMI_HIGH      80
#define HUMI_LOW       20
#define AQI_ALARM_LV   AQI_UNHEALTHY

/* ================================================================
 * 全局共享数据 (各任务间共享)
 * ================================================================ */
static dht11_data_t  g_dht;
static int           g_dht_valid = 0;
static ds3231_time_t g_time;
static int           g_time_valid = 0;
static aqi_level_t   g_aqi = AQI_GOOD;
static light_level_t g_light = LIGHT_NORMAL;
static uint8_t       g_alarm_flags = 0;
static uint8_t       g_alarm_enabled = 1;   /* 演示前默认撤防，避免 MQ135 飘高误报警盖过开锁提示音 */
static uint8_t       g_alarm_muted = 0;
static int           g_display_dirty = 1;
static int           g_current_page = PAGE_MAIN;

/* ---- System Clock 72MHz -------------------------------------- */
static void SystemClock_Config(void)
{
    RCC_OscInitTypeDef osc = {0};
    RCC_ClkInitTypeDef clk = {0};

    osc.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    osc.HSEState       = RCC_HSE_ON;
    osc.HSIState       = RCC_HSI_ON;
    osc.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
    osc.PLL.PLLState   = RCC_PLL_ON;
    osc.PLL.PLLSource  = RCC_PLLSOURCE_HSE;
    osc.PLL.PLLMUL     = RCC_PLL_MUL9;

    if (HAL_RCC_OscConfig(&osc) != HAL_OK) while (1);

    clk.ClockType      = RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK |
                         RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    clk.SYSCLKSource   = RCC_SYSCLKSOURCE_PLLCLK;
    clk.AHBCLKDivider  = RCC_SYSCLK_DIV1;
    clk.APB1CLKDivider = RCC_HCLK_DIV2;
    clk.APB2CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&clk, FLASH_LATENCY_2) != HAL_OK) while (1);
}

/* ---- 报警检测 ------------------------------------------------ */
static uint8_t check_alarm(const dht11_data_t *dht, int dht_valid, aqi_level_t aqi)
{
    uint8_t flags = 0;
    if (dht_valid)
    {
        if (dht->temp > TEMP_HIGH)  flags |= (1 << 0);
        if (dht->temp < TEMP_LOW)   flags |= (1 << 1);
        if (dht->humi > HUMI_HIGH)  flags |= (1 << 2);
        if (dht->humi < HUMI_LOW)   flags |= (1 << 3);
    }
    if (aqi >= AQI_ALARM_LV)        flags |= (1 << 4);
    return flags;
}

static const char* alarm_desc(uint8_t flags)
{
    if (flags & (1 << 0)) return "temp too high";
    if (flags & (1 << 1)) return "temp too low";
    if (flags & (1 << 2)) return "humi too high";
    if (flags & (1 << 3)) return "humi too low";
    if (flags & (1 << 4)) return "air pollution";
    return "";
}

static uint8_t alarm_severity(uint8_t flags)
{
    uint8_t sev = 0;
    if (flags & (1 << 4)) sev += 3;
    if (flags & (1 << 0)) sev += 1;
    if (flags & (1 << 1)) sev += 1;
    if (flags & (1 << 2)) sev += 1;
    if (flags & (1 << 3)) sev += 1;
    if (sev > 3) sev = 3;
    return sev;
}

/* ---- OLED 显示 ----------------------------------------------- */
static void show_page_footer(int page)
{
    char buf[16];
    snprintf(buf, sizeof(buf), "%d/%d", page + 1, PAGE_COUNT);
    oled_show_string(110, 7, buf);
}

static void page_main(void)
{
    char buf[32];
    oled_show_string(0, 0, "Smart Home v1.0");

    if (g_time_valid)
    {
        snprintf(buf, sizeof(buf), "20%02d-%02d-%02d %02d:%02d:%02d",
                 g_time.year, g_time.month, g_time.date,
                 g_time.hour, g_time.min, g_time.sec);
        oled_show_string(0, 1, buf);
    }
    else
        oled_show_string(0, 1, "DS3231: NOT FOUND");

    if (g_alarm_flags)
        oled_show_string(0, 2, alarm_desc(g_alarm_flags));

    if (g_dht_valid)
    {
        snprintf(buf, sizeof(buf), "Temp: %d C", g_dht.temp);
        oled_show_string(0, 4, buf);
        snprintf(buf, sizeof(buf), "Humi: %d %%", g_dht.humi);
        oled_show_string(0, 6, buf);
    }
    else
    {
        oled_show_string(0, 4, "Temp: --");
        oled_show_string(0, 6, "Humi: --");
    }
    show_page_footer(PAGE_MAIN);
}

static void page_env(void)
{
    char buf[32];
    snprintf(buf, sizeof(buf), "Air : %s", mq135_aqi_str(g_aqi));
    oled_show_string(0, 0, buf);

    snprintf(buf, sizeof(buf), "MQ-135: %.2fV", mq135_read_voltage());
    oled_show_string(0, 2, buf);

    snprintf(buf, sizeof(buf), "Light: %s", light_level_str(g_light));
    oled_show_string(0, 4, buf);

    snprintf(buf, sizeof(buf), "PIR  : %s", pir_is_present() ? "DETECTED" : "none");
    oled_show_string(0, 5, buf);

    show_page_footer(PAGE_ENV);
}

static void page_sys(void)
{
    oled_show_string(0, 0, "System Info");
    oled_show_string(0, 1, "MCU:STM32F103RC");
    oled_show_string(0, 2, "RTOS:FreeRTOS");
    oled_show_string(0, 3, "OLED:128x64 I2C");
    oled_show_string(0, 4, "Sensors: DHT11");
    oled_show_string(0, 5, "DS3231 MQ135");
    oled_show_string(0, 6, "Light PIR");
    show_page_footer(PAGE_SYS);
}

static void display_update(void)
{
    oled_clear();
    switch (g_current_page)
    {
    case PAGE_MAIN: page_main(); break;
    case PAGE_ENV:  page_env();  break;
    case PAGE_SYS:  page_sys();  break;
    default:        page_main(); break;
    }
    oled_show();
}

/* ================================================================
 * 任务函数
 * ================================================================ */

/* 传感器采集任务: 每 1s 读常规传感器, 每 2s 读 DHT11 */
static void sensor_task(void *arg)
{
    (void)arg;
    uint32_t last_dht = 0;

    for (;;)
    {
        if (ds3231_detected())
        {
            ds3231_get_time(&g_time);
            g_time_valid = 1;
        }
        else
            g_time_valid = 0;

        g_aqi   = mq135_get_aqi();
        g_light = light_get_level();

        if (HAL_GetTick() - last_dht >= 2000)
        {
            last_dht = HAL_GetTick();
            g_dht_valid = (dht11_read(&g_dht) == DHT11_OK);
        }

        g_alarm_flags = g_alarm_enabled ? check_alarm(&g_dht, g_dht_valid, g_aqi) : 0;
        g_display_dirty = 1;

        vTaskDelay(1000);
    }
}

/* 显示任务: 数据变化时刷新 OLED */
static void display_task(void *arg)
{
    (void)arg;
    for (;;)
    {
        if (g_display_dirty)
        {
            g_display_dirty = 0;
            display_update();
        }
        vTaskDelay(100);
    }
}

/* 报警指示任务: 每 200ms 处理 LED 闪灯 + 蜂鸣 */
static void alarm_task(void *arg)
{
    (void)arg;
    uint32_t cnt = 0;

    for (;;)
    {
        if (g_alarm_flags)
        {
            uint8_t sev = alarm_severity(g_alarm_flags);
            uint8_t beep_period = (sev >= 3) ? 1 : (sev == 2 ? 2 : 5);
            uint8_t led_period  = (sev >= 3) ? 2 : (sev == 2 ? 4 : 5);
            uint8_t led_count   = (sev >= 3) ? 3 : (sev == 2 ? 2 : 1);
            uint8_t half = led_period / 2;
            uint8_t on = ((cnt % led_period) < half);

            led_ctrl(LED0, on ? LED_ON : LED_OFF);
            led_ctrl(LED1, (led_count >= 2) ? (on ? LED_ON : LED_OFF) : LED_OFF);
            led_ctrl(LED2, (led_count >= 3) ? (on ? LED_ON : LED_OFF) : LED_OFF);

            if ((cnt % beep_period) == 0 && !g_alarm_muted)
            {
                beep_burst(200);
                printf("[ALARM L%d] %s!\r\n", sev, alarm_desc(g_alarm_flags));
            }
            cnt++;
        }
        else
        {
            led_ctrl(LED0, LED_OFF);
            led_ctrl(LED1, LED_OFF);
            led_ctrl(LED2, LED_OFF);
            cnt = 0;
        }
        vTaskDelay(200);
    }
}

/* MQTT 任务: 检查命令 + 每 2s 上报 (命令处理后立即上报) */
static uint8_t g_publish_now = 0;   /* 命令处理后立即上报标志 */

static void mqtt_task(void *arg)
{
    (void)arg;
    uint32_t last_pub = 0;

    for (;;)
    {
        /* 检查下发的命令 */
        char cmd[32];
        if (mqtt_check_command(cmd, sizeof(cmd)))
        {
            printf("[CMD] %s\r\n", cmd);
            if (strcmp(cmd, "ARM") == 0)          { g_alarm_enabled = 1; g_publish_now = 1; }
            else if (strcmp(cmd, "DISARM") == 0)  { g_alarm_enabled = 0; g_alarm_flags = 0; led_ctrl(LED0, LED_OFF); g_publish_now = 1; }
            else if (strcmp(cmd, "MUTE") == 0)    { g_alarm_muted = 1; }
            else if (strcmp(cmd, "UNMUTE") == 0)  { g_alarm_muted = 0; }
        }

        /* 每 2s 上报, 或命令处理后立即上报 */
        if (HAL_GetTick() - last_pub >= 2000 || g_publish_now)
        {
            last_pub = HAL_GetTick();
            g_publish_now = 0;
            char json[160];
            snprintf(json, sizeof(json),
                     "{\"temp\":%d,\"humi\":%d,\"air\":%d,\"light\":%d,\"pir\":%d,\"alarm\":%d,\"armed\":%d,\"door\":\"%s\"}",
                     g_dht_valid ? g_dht.temp : -1,
                     g_dht_valid ? g_dht.humi : -1,
                     (int)g_aqi, (int)g_light, pir_is_present(),
                     alarm_severity(g_alarm_flags), g_alarm_enabled,
                     door_get_last_name());
            mqtt_publish_msg("smart_home/test", json);
        }

        vTaskDelay(50);
    }
}

/* 按键任务: 每 50ms 轮询, 切换页面 */
static void key_task(void *arg)
{
    (void)arg;
    for (;;)
    {
        if (key_pressed(KEY0))
        {
            g_current_page = (g_current_page + 1) % PAGE_COUNT;
            g_display_dirty = 1;
        }
        if (key_pressed(KEY1))
        {
            g_current_page = (g_current_page - 1 + PAGE_COUNT) % PAGE_COUNT;
            g_display_dirty = 1;
        }
        vTaskDelay(50);
    }
}

/* PIR 中断回调 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == GPIO_PIN_12)
        pir_handle_interrupt();
}

/* ---- main ---------------------------------------------------- */
int main(void)
{
    HAL_Init();
    SystemClock_Config();

    /* 舵机自检：开机扫一次验证接线（T2.2 验证通过后可删） */
    servo_init();
    servo_open();
    delay_ms(1000);
    servo_close();

    /* 基础外设 */
    uart_debug_init();
    delay_ms(100);
    door_uart_init();   /* 开启 UART1 中断接收（开锁命令） */
    led_all_init();
    beep_init();
    i2c_scanner_init();
    oled_init();
    ds3231_init();
    key_init();

    /* ADC + 传感器 */
    adc_test_init();
    mq135_init(ADC_CHANNEL_8);
    light_init(ADC_CHANNEL_6);
    dht11_init();
    pir_init();

    /* WiFi + MQTT */
    wifi_test_init();
    wifi_join(WIFI_SSID, WIFI_PWD);
    mqtt_connect_broker("broker.emqx.io", 1883, "stm32_smarthome_01", NULL, NULL);
    mqtt_subscribe("smart_home/cmd");

    printf("\r\n============================================\r\n");
    printf("  Smart Home Environment Monitor v1.0\r\n");
    printf("  FreeRTOS 多任务版\r\n");
    printf("============================================\r\n\r\n");

    /* 创建任务 */
    xTaskCreate(sensor_task,  "sensor",  256, NULL, 2, NULL);
    xTaskCreate(display_task, "display", 256, NULL, 2, NULL);
    xTaskCreate(alarm_task,   "alarm",   128, NULL, 3, NULL);
    xTaskCreate(mqtt_task,    "mqtt",    512, NULL, 1, NULL);
    xTaskCreate(key_task,     "key",     128, NULL, 2, NULL);
    xTaskCreate(door_task,    "door",    256, NULL, 2, NULL);

    /* 启动调度器 (永不返回) */
    vTaskStartScheduler();

    while (1) { }
}
