#include "mq135.h"
#include "adc_test.h"
#include "uart_debug.h"
#include "delay.h"
#include <stdio.h>

/* ---- 静态配置 ---------------------------------------------- */
static uint32_t mq135_channel = ADC_CHANNEL_8;   /* PB0 → H9_8 */
static float    mq135_vref   = 3.3f;

/* ---- 初始化 ------------------------------------------------ */
void mq135_init(uint32_t adc_channel)
{
    mq135_channel = adc_channel;

    printf("[MQ-135] PB0 (H9_8) ADC IN%lu, VREF=3.3V\r\n",
           (unsigned long)(adc_channel & 0x1F));
    printf("[MQ-135] Warm-up: 2~3 minutes for stable readings.\r\n");
}

/* ---- 读 ADC 原始值 ----------------------------------------- */
uint16_t mq135_read_raw(void)
{
    return adc_read_channel(mq135_channel);
}

/* ---- 读电压 ------------------------------------------------ */
float mq135_read_voltage(void)
{
    uint16_t raw = adc_read_channel(mq135_channel);
    if (raw == 0xFFFF) return -1.0f;
    return (float)raw * mq135_vref / 4095.0f;
}

/* ---- 读空气质量等级 ---------------------------------------- */
aqi_level_t mq135_get_aqi(void)
{
    float voltage = mq135_read_voltage();

    /*
     * 阈值判定 (需根据实际环境标定, 以下是经验值)
     * 洁净空气中 MQ-135 输出约 0.5V
     * 污染越重电压越高
     */
    if (voltage < 0)       return AQI_GOOD;       /* 读取出错 */
    if (voltage < 0.8f)    return AQI_GOOD;
    if (voltage < 1.5f)    return AQI_MODERATE;
    if (voltage < 2.5f)    return AQI_UNHEALTHY;
    return AQI_HAZARDOUS;
}

/* ---- 等级 → 字符串 ----------------------------------------- */
const char* mq135_aqi_str(aqi_level_t level)
{
    switch (level)
    {
    case AQI_GOOD:      return "GOOD";
    case AQI_MODERATE:  return "MODERATE";
    case AQI_UNHEALTHY: return "UNHEALTHY";
    case AQI_HAZARDOUS: return "HAZARDOUS!";
    default:            return "UNKNOWN";
    }
}

/* ---- 测试: 连续读取 ---------------------------------------- */
void mq135_test(void)
{
    int i;

    printf("\r\n========== MQ-135 Air Quality Test ==========\r\n");
    printf("Warm-up... please wait.\r\n");

    for (i = 1; i <= 10; i++)
    {
        uint16_t raw = mq135_read_raw();

        if (raw == 0xFFFF)
        {
            printf("  [%2d/10] ADC read error!\r\n", i);
        }
        else
        {
            float v = (float)raw * mq135_vref / 4095.0f;
            aqi_level_t aqi = mq135_get_aqi();

            printf("  [%2d/10] Raw:%4d  %.3fV  AQI:%s\r\n",
                   i, raw, v, mq135_aqi_str(aqi));
        }

        delay_ms(1000);
    }
    printf("==============================================\r\n");
}
