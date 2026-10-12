#ifndef __BT_TEST_H__
#define __BT_TEST_H__

#include "stm32f1xx_hal.h"

/*
 * HC-05 蓝牙模块测试
 *
 * 接口:   UART2  PA2=TX  PA3=RX
 * AT模式: 按住模块上黑色按钮, 38400 baud
 * 透传:   松开按钮, 9600 baud, PIN=1234
 */

void bt_test_init(void);
void bt_test_at(void);
void bt_bridge(void);      /* 透传桥: PC串口 <-> 蓝牙, 按 'q' 退出 */

#endif
