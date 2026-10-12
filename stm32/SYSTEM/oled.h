#ifndef __OLED_H__
#define __OLED_H__

#include "stm32f1xx_hal.h"

/*
 * SSD1306 OLED 128x64 I2C 驱动
 *
 * 接口:    I2C1  PB6=SCL(H6_4)  PB7=SDA(H7_3)
 * 地址:    0x3C (7-bit)
 * 分辨率:  128 x 64
 */

void oled_init(void);                          /* 初始化 OLED */
void oled_clear(void);                         /* 清屏 */
void oled_show(void);                          /* 刷新显存到屏幕 */
void oled_set_pos(uint8_t x, uint8_t y);       /* 设置光标位置 */

/* 显示内容 */
void oled_show_string(uint8_t x, uint8_t y, const char *str);  /* 6x8 英文字符串 */
void oled_show_chinese(uint8_t x, uint8_t y, uint8_t idx);     /* 16x16 中文字 (需字库) */

/* 中文字库索引 (与 chinese_font.h 对应) */
enum {
    CN_WEN     = 0,   /* 温 */
    CN_SHI     = 1,   /* 湿 */
    CN_DU      = 2,   /* 度 */
    CN_KONG    = 3,   /* 空 */
    CN_QI      = 4,   /* 气 */
    CN_ZHI     = 5,   /* 质 */
    CN_BAO     = 6,   /* 报 */
    CN_JING    = 7,   /* 警 */
    CN_ZHENG   = 8,   /* 正 */
    CN_CHANG   = 9,   /* 常 */
    CN_ZHIHUI  = 10,  /* 智 */
    CN_NENG    = 11,  /* 能 */
    CN_JIA     = 12,  /* 家 */
    CN_JU      = 13,  /* 居 */
    CN_GUANG   = 14,  /* 光 */
    CN_ZHAO    = 15,  /* 照 */
    CN_REN     = 16,  /* 人 */
    CN_TI      = 17,  /* 体 */
    CN_GAO     = 18,  /* 高 */
    CN_DI      = 19,  /* 低 */
    CN_JIAN    = 20,  /* 检 */
    CN_CE      = 21,  /* 测 */
    CN_HUAN    = 22,  /* 环 */
    CN_JING2   = 23,  /* 境 */
    CN_SHIJIAN = 24,  /* 时 */
    CN_JIAN2   = 25,  /* 间 */
    CN_COUNT   = 26
};
void oled_show_num(uint8_t x, uint8_t y, int num, uint8_t len);/* 显示数字 */
void oled_draw_point(uint8_t x, uint8_t y, uint8_t mode);      /* 画点 */
uint8_t oled_get_pixel(uint8_t x, uint8_t y);                  /* 读点 (调试用) */

/* 测试: 全屏填充 → 显示 Hello World */
void oled_test_demo(void);

#endif
