#include "dht11.h"
#include "delay.h"
#include "uart_debug.h"
#include <stdio.h>

/* ---- 硬件宏 ------------------------------------------------ */
#define DHT11_PORT      GPIOC
#define DHT11_PIN       GPIO_PIN_1
#define DHT11_PORT_BASE (GPIOC_BASE)

/* 直接寄存器操作, 避免 HAL 开销 (单总线时序 ~1us 级) */
#define DHT11_DQ_H()    (GPIOC->BSRR = GPIO_PIN_1)       /* PC1 = 1 */
#define DHT11_DQ_L()    (GPIOC->BRR  = GPIO_PIN_1)       /* PC1 = 0 */
#define DHT11_DQ_R()    ((GPIOC->IDR  & GPIO_PIN_1) != 0) /* 读 PC1 */

/* ---- 内部 DWT (Data Watchpoint) 延时 (72MHz → 1tick≈14ns) -- */
static uint32_t dwt_enabled = 0;

static void dwt_enable(void)
{
    if (!dwt_enabled)
    {
        CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
        DWT->CYCCNT = 0;
        DWT->CTRL   |= DWT_CTRL_CYCCNTENA_Msk;
        dwt_enabled = 1;
    }
}

/* 微秒级硬件延时 (72MHz = 72 cycles/us) */
static inline void dwt_delay_us(uint32_t us)
{
    uint32_t start = DWT->CYCCNT;
    uint32_t ticks = us * (SystemCoreClock / 1000000);
    while ((DWT->CYCCNT - start) < ticks);
}

/* ---- 引脚模式切换 ------------------------------------------ */
static void dht11_pin_out(void)
{
    GPIO_InitTypeDef gpio = {0};
    gpio.Mode  = GPIO_MODE_OUTPUT_PP;
    gpio.Speed = GPIO_SPEED_FREQ_HIGH;
    gpio.Pin   = DHT11_PIN;
    HAL_GPIO_Init(DHT11_PORT, &gpio);
}

static void dht11_pin_in(void)
{
    GPIO_InitTypeDef gpio = {0};
    gpio.Mode = GPIO_MODE_INPUT;
    gpio.Pull = GPIO_PULLUP;
    gpio.Pin  = DHT11_PIN;
    HAL_GPIO_Init(DHT11_PORT, &gpio);
}

/* ---- 初始化 ------------------------------------------------ */
void dht11_init(void)
{
    __HAL_RCC_GPIOC_CLK_ENABLE();

    /* 上电后 DHT11 需 1s 稳定 */
    dht11_pin_out();
    DHT11_DQ_H();          /* 空闲高电平 */
    dwt_enable();

    printf("[DHT11] PC1 (H9_1) initialized, waiting 1s for sensor ready...\r\n");
    delay_ms(1000);
}

/* ---- 读一次温湿度 (核心) ------------------------------------- */
int dht11_read(dht11_data_t *out)
{
    uint8_t buf[5] = {0};
    uint8_t i, j;
    uint32_t timeout;
    uint32_t primask;

    if (!out) return DHT11_ERR_NORSP;

    /* === STEP 1: 主机发起始信号 === */
    primask = __get_PRIMASK();
    __disable_irq();               /* 关全局中断, 确保时序 */

    dht11_pin_out();
    DHT11_DQ_L();                  /* 拉低 > 18ms */
    dwt_delay_us(20000);           /* 20ms */

    DHT11_DQ_H();                  /* 拉高 20~40us */
    dwt_delay_us(30);

    /* === STEP 2: 切换输入, 等待 DHT11 响应 === */
    dht11_pin_in();

    /* DHT11 应先拉低 ~80us, 再拉高 ~80us */
    timeout = 1000;
    while (DHT11_DQ_R() && --timeout);   /* 等它拉低 */
    if (timeout == 0) { __set_PRIMASK(primask); return DHT11_ERR_NORSP; }

    timeout = 1000;
    while (!DHT11_DQ_R() && --timeout);  /* 等它拉高 */
    if (timeout == 0) { __set_PRIMASK(primask); return DHT11_ERR_NORSP; }

    timeout = 1000;
    while (DHT11_DQ_R() && --timeout);   /* 等它再次拉低 (准备发数据) */
    if (timeout == 0) { __set_PRIMASK(primask); return DHT11_ERR_NORSP; }

    /* === STEP 3: 读 40 位数据 === */
    for (i = 0; i < 5; i++)
    {
        for (j = 0; j < 8; j++)
        {
            /* 等待低电平结束 (每 bit 以 50us 低电平开始) */
            timeout = 1000;
            while (!DHT11_DQ_R() && --timeout);
            if (timeout == 0) { __set_PRIMASK(primask); return DHT11_ERR_TIMEO; }

            /* 延时 40us 后采样：26~28us 为 0, 70us 为 1 */
            dwt_delay_us(40);

            buf[i] <<= 1;
            if (DHT11_DQ_R())
                buf[i] |= 0x01;

            /* 等待高电平结束 (进入下一位的低电平) */
            timeout = 1000;
            while (DHT11_DQ_R() && --timeout);
            if (timeout == 0) { __set_PRIMASK(primask); return DHT11_ERR_TIMEO; }
        }
    }

    __set_PRIMASK(primask);          /* 恢复中断 */

    /* === STEP 4: 校验 === */
    if (buf[4] != (uint8_t)(buf[0] + buf[1] + buf[2] + buf[3]))
        return DHT11_ERR_CHECK;

    out->humi     = buf[0];
    out->humi_dec = buf[1];
    out->temp     = buf[2];
    out->temp_dec = buf[3];

    return DHT11_OK;
}

/* ---- 测试: 连续读 5 次 -------------------------------------- */
void dht11_test(void)
{
    int i;

    printf("\r\n========== DHT11 Temp & Humi Test ==========\r\n");

    for (i = 1; i <= 5; i++)
    {
        dht11_data_t data;
        int ret = dht11_read(&data);

        if (ret == DHT11_OK)
        {
            printf("  [%d/5] Temp: %d.%d C  |  Humi: %d.%d %%\r\n",
                   i, data.temp, data.temp_dec,
                   data.humi, data.humi_dec);
        }
        else if (ret == DHT11_ERR_CHECK)
        {
            printf("  [%d/5] Checksum error!\r\n", i);
        }
        else
        {
            printf("  [%d/5] No response / timeout (%d)\r\n", i, ret);
        }

        delay_ms(1500);   /* DHT11 采样周期 ≥ 1s, 取 1.5s */
    }
    printf("==============================================\r\n");
}
