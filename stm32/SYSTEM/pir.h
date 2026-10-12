#ifndef __PIR_H__
#define __PIR_H__

#include "stm32f1xx_hal.h"

/*
 * HC-SR501 人体红外传感器驱动 (GPIO + EXTI 中断)
 *
 * 硬件:  H8_11 = PB12 (SPI Flash CS, 本项目不用SPI Flash, 可复用作 GPIO)
 *         PB12 为 FT 5V 耐受脚, HC-SR501 5V输出安全
 * 模块:  三针 (VCC=5V, GND, OUT→H8_11)
 * 调节:  模块上有两个电位器:
 *          - 灵敏度 (距离 3~7m)
 *          - 延时 (输出高电平持续时间 5~300s)
 * 跳线:  触发模式选择:
 *          - L 位置: 不可重复触发 (单次)
 *          - H 位置: 可重复触发 (推荐, 人一直在就持续输出)
 *
 * 注意:  PB3 是 JTDO 复用脚, 必须先在 AFIO 中关闭 JTAG
 *        所有用到 PB3/PB4/PA15 的代码都需要先调用 pir_disable_jtag()
 */

/* ---- 回调类型 ---------------------------------------------- */
typedef void (*pir_callback_t)(void);   /* 人体检测回调 */

/* ---- API -------------------------------------------------- */
void pir_disable_jtag(void);                      /* 关 JTAG, 保留 SWD (PA13/PA14) */
void pir_init(void);                              /* 初始化 PB3 + EXTI 中断 */
void pir_set_callback(pir_callback_t cb);         /* 注册回调 (检测到人时调用) */
void pir_handle_interrupt(void);                  /* 中断处理入口: 在 HAL_GPIO_EXTI_Callback 中调用 */
int  pir_is_present(void);                        /* 当前是否有人 (1=有人) */
void pir_test(void);                              /* 测试: 持续监测 30s */

#endif
