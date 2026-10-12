#include "adc_test.h"
#include "uart_debug.h"
#include <stdio.h>

ADC_HandleTypeDef hadc1;    /* extern in .h → 传感器驱动共用 */

/* 候选测试通道表 (排除已占用的 PA1,PA2,PA3,PA7) */
static const adc_test_pin_t adc_pins[ADC_TEST_CHANNELS] = {
    {"PA0",  GPIOA, GPIO_PIN_0,  ADC_CHANNEL_0},
    {"PA4",  GPIOA, GPIO_PIN_4,  ADC_CHANNEL_4},
    {"PA5",  GPIOA, GPIO_PIN_5,  ADC_CHANNEL_5},
    {"PA6",  GPIOA, GPIO_PIN_6,  ADC_CHANNEL_6},
    {"PB0",  GPIOB, GPIO_PIN_0,  ADC_CHANNEL_8},
    {"PC0",  GPIOC, GPIO_PIN_0,  ADC_CHANNEL_10},
    {"PC1",  GPIOC, GPIO_PIN_1,  ADC_CHANNEL_11},
    {"PC2",  GPIOC, GPIO_PIN_2,  ADC_CHANNEL_12},
    {"PC3",  GPIOC, GPIO_PIN_3,  ADC_CHANNEL_13},
};

void adc_test_init(void)
{
    int i;

    /* 1. 使能 GPIO + ADC1 时钟 */
    __HAL_RCC_ADC1_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();

    /* 2. 将所有候选通道配置为模拟输入 */
    for (i = 0; i < ADC_TEST_CHANNELS; i++)
    {
        GPIO_InitTypeDef gpio = {0};
        gpio.Mode = GPIO_MODE_ANALOG;
        gpio.Pin  = adc_pins[i].pin;
        HAL_GPIO_Init(adc_pins[i].port, &gpio);
    }

    /* 3. ADC1 基础初始化 (仅校准, 具体通道在读取时切换) */
    hadc1.Instance                   = ADC1;
    hadc1.Init.ScanConvMode          = ADC_SCAN_DISABLE;     /* 单通道 */
    hadc1.Init.ContinuousConvMode    = DISABLE;               /* 软件触发 */
    hadc1.Init.DiscontinuousConvMode = DISABLE;
    hadc1.Init.ExternalTrigConv      = ADC_SOFTWARE_START;
    hadc1.Init.DataAlign             = ADC_DATAALIGN_RIGHT;
    /* STM32F1 HAL ADC_InitTypeDef has no NbrOfChannel field */
    HAL_ADC_Init(&hadc1);

    /* 4. ADC 校准 */
    HAL_ADCEx_Calibration_Start(&hadc1);

    printf("ADC1 initialized & calibrated.\r\n");
    printf("VREF+ = 3.3V, VREF- = GND\r\n");
}

uint16_t adc_read_channel(uint32_t ch)
{
    ADC_ChannelConfTypeDef sConfig = {0};

    sConfig.Channel = ch;
    sConfig.Rank    = ADC_REGULAR_RANK_1;
    sConfig.SamplingTime = ADC_SAMPLETIME_55CYCLES_5;  /* 最长采样时间, 高阻抗信号兼容 */
    HAL_ADC_ConfigChannel(&hadc1, &sConfig);

    HAL_ADC_Start(&hadc1);
    if (HAL_ADC_PollForConversion(&hadc1, 10) == HAL_OK)
    {
        uint16_t val = HAL_ADC_GetValue(&hadc1);
        HAL_ADC_Stop(&hadc1);
        return val;
    }
    HAL_ADC_Stop(&hadc1);
    return 0xFFFF;  /* 超时/错误 */
}

void adc_scan_all(void)
{
    int i;

    printf("\r\n========== ADC Channel Scan ==========\r\n");
    printf(" GPIO  | Channel | Raw  | Voltage(V)\r\n");
    printf("-------+---------+------+------------\r\n");

    for (i = 0; i < ADC_TEST_CHANNELS; i++)
    {
        uint16_t raw = adc_read_channel(adc_pins[i].channel);
        float voltage;

        if (raw == 0xFFFF)
        {
            printf(" %-4s  |  IN%-2d   | ERR  | timeout\r\n",
                   adc_pins[i].name, (int)(adc_pins[i].channel & 0x1F));
        }
        else
        {
            voltage = (float)raw * 3.3f / 4095.0f;
            printf(" %-4s  |  IN%-2d   | %4d | %d.%03dV\r\n",
                   adc_pins[i].name,
                   (int)(adc_pins[i].channel & 0x1F),
                   raw,
                   (int)voltage,
                   (int)(voltage * 1000) % 1000);
        }
    }
    printf("=====================================\r\n");
    printf("Tip: 将待测模块AO输出接到H8_6/H9_5/H9_8,\r\n");
    printf("     看哪个通道电压跟随变化即对应排针。\r\n");
}

void adc_scan_channel(uint32_t ch)
{
    printf("\r\nContinuous read ADC IN%d (press 'q' to quit)...\r\n",
           (int)(ch & 0x1F));
    printf("Press Enter to start, 'q' to exit\r\n");

    while (1)
    {
        /* 非阻塞检查串口输入 */
        int c = uart_getchar();
        if (c == 'q' || c == 'Q') break;

        uint16_t raw = adc_read_channel(ch);
        if (raw != 0xFFFF)
        {
            float v = (float)raw * 3.3f / 4095.0f;
            printf("  ADC_IN%d: %4d raw, %d.%03dV\r\n",
                   (int)(ch & 0x1F), raw, (int)v, (int)(v * 1000) % 1000);
        }
        else
        {
            printf("  ADC_IN%d: read error\r\n", (int)(ch & 0x1F));
        }

        HAL_Delay(500);
    }
}
