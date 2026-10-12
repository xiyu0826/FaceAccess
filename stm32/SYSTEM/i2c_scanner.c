#include "i2c_scanner.h"
#include <stdio.h>

I2C_HandleTypeDef hi2c1;

void i2c_scanner_init(void)
{
    GPIO_InitTypeDef gpio = {0};

    /* 1. 使能时钟 */
    __HAL_RCC_I2C1_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    /* 2. PB6(SCL) + PB7(SDA): 开漏复用输出, 外部需 4.7k 上拉 */
    gpio.Mode  = GPIO_MODE_AF_OD;
    gpio.Speed = GPIO_SPEED_FREQ_HIGH;
    gpio.Pull  = GPIO_NOPULL;   /* 依赖外部上拉电阻 */

    gpio.Pin = GPIO_PIN_6;
    HAL_GPIO_Init(GPIOB, &gpio);
    gpio.Pin = GPIO_PIN_7;
    HAL_GPIO_Init(GPIOB, &gpio);

    /* 3. I2C1 标准模式 100kHz */
    hi2c1.Instance             = I2C1;
    hi2c1.Init.ClockSpeed      = 100000;
    hi2c1.Init.DutyCycle       = I2C_DUTYCYCLE_2;
    hi2c1.Init.OwnAddress1     = 0x00;
    hi2c1.Init.AddressingMode  = I2C_ADDRESSINGMODE_7BIT;
    hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c1.Init.NoStretchMode   = I2C_NOSTRETCH_DISABLE;

    HAL_I2C_Init(&hi2c1);
}

int i2c_probe(uint8_t addr)
{
    /*
     * HAL_I2C_IsDeviceReady 发送 START + 地址 + STOP,
     * 收到 ACK 返回 HAL_OK → 设备存在
     */
    if (HAL_I2C_IsDeviceReady(&hi2c1, (uint16_t)(addr << 1), 2, 50) == HAL_OK)
        return 1;
    return 0;
}

void i2c_scan(void)
{
    uint8_t addr;
    int found = 0;

    printf("\r\n========== I2C Bus Scan (0x02 ~ 0x7E) ==========\r\n");
    printf("     00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F\r\n");

    for (addr = 0x00; addr < 0x80; addr += 16)
    {
        printf("0x%02X ", addr & 0xF0);
        int i;
        for (i = 0; i < 16; i++)
        {
            uint8_t a = addr + i;
            if (a < 0x02 || a > 0x7E)
            {
                printf(" -- ");
            }
            else if (i2c_probe(a))
            {
                printf(" %02X ", a);
                found++;
            }
            else
            {
                printf(" .. ");
            }
        }
        printf("\r\n");
    }

    printf("\r\nFound %d device(s)\r\n", found);

    /* 提示已知设备 */
    if (i2c_probe(0x3C))
        printf("  [0x3C] OLED SSD1306 detected!\r\n");
    if (i2c_probe(0x68))
        printf("  [0x68] DS3231 RTC detected!\r\n");

    printf("==================================================\r\n");
}

void i2c_set_speed(uint32_t speed_hz)
{
    hi2c1.Init.ClockSpeed = speed_hz;

    /* 速率 >100k 时需切换占空比模式 */
    if (speed_hz > 100000)
        hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_16_9;
    else
        hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;

    HAL_I2C_Init(&hi2c1);   /* 重新初始化生效 */
    printf("I2C speed set to %lu Hz\r\n", speed_hz);
}
