#include "ds3231.h"
#include "i2c_scanner.h"
#include "uart_debug.h"
#include <stdio.h>

#define DS3231_ADDR  0x68    /* 7-bit I2C 地址 */

/* ---- BCD ↔ 二进制转换 -------------------------------------- */
static inline uint8_t bcd2bin(uint8_t bcd)
{
    return (bcd >> 4) * 10 + (bcd & 0x0F);
}

static inline uint8_t bin2bcd(uint8_t bin)
{
    return ((bin / 10) << 4) | (bin % 10);
}

/* ---- 初始化 + 探测 ----------------------------------------- */
void ds3231_init(void)
{
    if (i2c_probe(DS3231_ADDR))
        printf("[DS3231] Detected at 0x68!\r\n");
    else
        printf("[DS3231] NOT found at 0x68! Check wiring.\r\n");
}

int ds3231_detected(void)
{
    return i2c_probe(DS3231_ADDR);
}

/* ---- 读取当前时间 ------------------------------------------ */
void ds3231_get_time(ds3231_time_t *t)
{
    uint8_t buf[7];

    /* 从寄存器 0x00 开始连续读 7 字节 */
    HAL_I2C_Mem_Read(&hi2c1, (uint16_t)(DS3231_ADDR << 1),
                     0x00, I2C_MEMADD_SIZE_8BIT,
                     buf, 7, 100);

    t->sec   = bcd2bin(buf[0] & 0x7F);   /* 去掉 OSF 位 */
    t->min   = bcd2bin(buf[1]);
    t->hour  = bcd2bin(buf[2] & 0x3F);   /* 24h 模式 */
    t->week  = buf[3];
    t->date  = bcd2bin(buf[4]);
    t->day   = t->date;
    t->month = bcd2bin(buf[5] & 0x1F);   /* 去掉世纪位 */
    t->year  = bcd2bin(buf[6]);
}

/* ---- 写入时间 ---------------------------------------------- */
void ds3231_set_time(ds3231_time_t *t)
{
    uint8_t buf[7];

    buf[0] = bin2bcd(t->sec & 0x7F);
    buf[1] = bin2bcd(t->min);
    buf[2] = bin2bcd(t->hour & 0x3F);     /* 24h 模式 */
    buf[3] = t->week;
    buf[4] = bin2bcd(t->date);
    buf[5] = bin2bcd(t->month & 0x1F);
    buf[6] = bin2bcd(t->year);

    HAL_I2C_Mem_Write(&hi2c1, (uint16_t)(DS3231_ADDR << 1),
                      0x00, I2C_MEMADD_SIZE_8BIT,
                      buf, 7, 100);
}

/* ---- 读取芯片温度 (内部温度传感器, 精度 ±3°C) --------------- */
int8_t ds3231_get_temp(void)
{
    uint8_t raw;

    HAL_I2C_Mem_Read(&hi2c1, (uint16_t)(DS3231_ADDR << 1),
                     0x11, I2C_MEMADD_SIZE_8BIT,
                     &raw, 1, 100);

    return (int8_t)raw;   /* 8-bit signed, 1°C/LSB */
}

/* ---- 打印当前时间 ------------------------------------------ */
void ds3231_print_time(void)
{
    ds3231_time_t t;

    if (!ds3231_detected())
    {
        printf("[DS3231] Device not found!\r\n");
        return;
    }

    ds3231_get_time(&t);

    printf("Time: 20%02d-%02d-%02d  %02d:%02d:%02d  Weekday=%d\r\n",
           t.year, t.month, t.date,
           t.hour, t.min, t.sec,
           t.week);
}

/* ---- 辅助: 读2位数字 --------------------------------------- */
static int read2digits(void)
{
    int c1, c2;

    /* 跳过非数字 */
    do { c1 = getchar(); } while (c1 != '\r' && c1 != '\n' && !(c1 >= '0' && c1 <= '9'));
    if (c1 == '\r' || c1 == '\n') return -1;

    c2 = getchar();
    if (c2 >= '0' && c2 <= '9')
    {
        printf("%c%c", c1, c2);
        return (c1 - '0') * 10 + (c2 - '0');
    }
    return -1;
}

/* ---- 串口交互式设置时间 ------------------------------------ */
void ds3231_set_prompt(void)
{
    ds3231_time_t t;
    int y, mo, d, h, mi, s;

    if (!ds3231_detected())
    {
        printf("[DS3231] Device not found!\r\n");
        return;
    }

    printf("\r\n--- Set DS3231 Time ---\r\n");

    printf("Year   (00-99) > "); y = read2digits();
    if (y < 0) goto fail;
    printf("\r\n");

    printf("Month  (01-12) > "); mo = read2digits();
    if (mo < 1 || mo > 12) goto fail;
    printf("\r\n");

    printf("Day    (01-31) > "); d = read2digits();
    if (d < 1 || d > 31) goto fail;
    printf("\r\n");

    printf("Hour   (00-23) > "); h = read2digits();
    if (h < 0 || h > 23) goto fail;
    printf("\r\n");

    printf("Minute (00-59) > "); mi = read2digits();
    if (mi < 0 || mi > 59) goto fail;
    printf("\r\n");

    printf("Second (00-59) > "); s = read2digits();
    if (s < 0 || s > 59) goto fail;
    printf("\r\n");

    printf("Weekday (1=Mon 2=Tue 3=Wed 4=Thu 5=Fri 6=Sat 7=Sun) > ");
    {
        int w = getchar();
        printf("%c\r\n", w);
        t.week = (w >= '1' && w <= '7') ? (uint8_t)(w - '0') : 1;
    }

    t.year  = (uint8_t)y;
    t.month = (uint8_t)mo;
    t.date  = (uint8_t)d;
    t.day   = (uint8_t)d;
    t.hour  = (uint8_t)h;
    t.min   = (uint8_t)mi;
    t.sec   = (uint8_t)s;

    ds3231_set_time(&t);
    printf("\r\nOK! Time set to 20%02d-%02d-%02d %02d:%02d:%02d  Weekday=%d\r\n",
           y, mo, d, h, mi, s, t.week);
    return;

fail:
    printf("\r\nInvalid input. Canceled.\r\n");
}
