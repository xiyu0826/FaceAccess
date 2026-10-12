#include "light.h"
#include "adc_test.h"
#include "led_all.h"
#include "uart_debug.h"
#include "delay.h"
#include <stdio.h>

/*
 * 光敏模块 (3脚 DO), 接 H8_6 = PA6
 * DO 数字输出, 经 ADC 读取:
 *   ~0.14V → BRIGHT
 *   ~3.3V → DARK
 */

static uint32_t light_channel = ADC_CHANNEL_6;
static float    light_vref    = 3.3f;
static int      light_auto    = 0;
static uint32_t light_last_ms = 0;

#define LIGHT_THRESH_DARK    2.5f
#define LIGHT_THRESH_DIM     2.0f
#define LIGHT_THRESH_NORMAL  1.0f

void light_init(uint32_t adc_channel)
{
    light_channel = adc_channel;
    printf("[LIGHT] PA6 (H8_6), ADC IN%lu, VREF=3.3V\r\n",
           (unsigned long)(adc_channel & 0x1F));
}

uint16_t light_read_raw(void)
{
    return adc_read_channel(light_channel);
}

float light_read_voltage(void)
{
    uint16_t raw = adc_read_channel(light_channel);
    if (raw == 0xFFFF) return -1.0f;
    return (float)raw * light_vref / 4095.0f;
}

light_level_t light_get_level(void)
{
    float v = light_read_voltage();
    if (v < 0) return LIGHT_NORMAL;
    if (v > LIGHT_THRESH_DARK)   return LIGHT_DARK;
    if (v > LIGHT_THRESH_DIM)    return LIGHT_DIM;
    if (v > LIGHT_THRESH_NORMAL) return LIGHT_NORMAL;
    return LIGHT_BRIGHT;
}

const char* light_level_str(light_level_t lv)
{
    switch (lv)
    {
    case LIGHT_DARK:   return "DARK";
    case LIGHT_DIM:    return "DIM";
    case LIGHT_NORMAL: return "NORMAL";
    case LIGHT_BRIGHT: return "BRIGHT";
    default:           return "UNKNOWN";
    }
}

void light_auto_led(int enable)
{
    light_auto = enable;
    printf("[LIGHT] Auto LED mode %s\r\n", enable ? "ON" : "OFF");
}

void light_auto_led_poll(void)
{
    if (!light_auto) return;
    if (HAL_GetTick() - light_last_ms < 500) return;
    light_last_ms = HAL_GetTick();

    if (light_get_level() == LIGHT_DARK)
        led_all_on();
    else
        led_all_off();
}

void light_test(void)
{
    int i;
    printf("\r\n========== Light Sensor Test ==========\r\n");
    for (i = 1; i <= 10; i++)
    {
        uint16_t raw = light_read_raw();
        if (raw == 0xFFFF)
            printf("  [%2d/10] ADC read error!\r\n", i);
        else
        {
            float v = (float)raw * light_vref / 4095.0f;
            printf("  [%2d/10] Raw:%4d  %.3fV  Level:%s\r\n",
                   i, raw, v, light_level_str(light_get_level()));
        }
        delay_ms(500);
    }
    printf("========================================\r\n");
}
