#include "runled.h"
#include "led.h"

/*
 * 板子上 4 颗 LED，共阳极接法，低电平点亮
 *   LED0 -> PA11
 *   LED1 -> PA12
 *   LED2 -> PA13
 *   LED3 -> PA14
 * 跑马灯模式：来回流水
 *   0 -> 1 -> 2 -> 3 -> 2 -> 1 -> 0 -> 1 -> 2 -> 3 -> ...
 */

static int led_run_index = 0;     // 当前点亮的灯编号：0/1/2/3
static int led_run_dir   = 1;     // 方向：1=正向(0->3)，-1=反向(3->0)

void led_run_init(void)
{
	led_init();    // 复用 led.h 中的初始化（含 SWD 关闭）
}

void led_run_step(void)
{
	int i;

	// 1. 先把 4 颗灯全部熄灭
	for(i = 0; i < 4; i++)
		led_ctrl(i, LED_OFF);

	// 2. 点亮当前这一盏
	led_ctrl(led_run_index, LED_ON);

	// 3. 移动到下一盏，到达端点后反向
	led_run_index += led_run_dir;
	if(led_run_index >= 3)
	{
		led_run_index = 3;
		led_run_dir = -1;       // 到达右端，反向
	}
	else if(led_run_index <= 0)
	{
		led_run_index = 0;
		led_run_dir = 1;        // 到达左端，正向
	}
}
