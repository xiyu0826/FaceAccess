#include "led.h"

void led_init(void)
{
	GPIO_InitTypeDef GPIO_Init = {0};

	// 1. 使能 GPIOA 和 AFIO 时钟
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_AFIO_CLK_ENABLE();

	// 2. 关闭 SWD（JTAG-DP + SW-DP），释放 PA13/PA14 为普通 GPIO
	__HAL_AFIO_REMAP_SWJ_DISABLE();

	// 3. 初始化 PA11/PA12/PA13/PA14 为推挽输出，50MHz
	GPIO_Init.Mode  = GPIO_MODE_OUTPUT_PP;
	GPIO_Init.Speed = GPIO_SPEED_FREQ_HIGH;
	GPIO_Init.Pull  = GPIO_NOPULL;

	GPIO_Init.Pin = LED0_GPIO_PIN;
	HAL_GPIO_Init(LED0_GPIO_PORT, &GPIO_Init);
	GPIO_Init.Pin = LED1_GPIO_PIN;
	HAL_GPIO_Init(LED1_GPIO_PORT, &GPIO_Init);
	GPIO_Init.Pin = LED2_GPIO_PIN;
	HAL_GPIO_Init(LED2_GPIO_PORT, &GPIO_Init);
	GPIO_Init.Pin = LED3_GPIO_PIN;
	HAL_GPIO_Init(LED3_GPIO_PORT, &GPIO_Init);

	// 4. 默认全部熄灭（输出高电平）
	HAL_GPIO_WritePin(LED0_GPIO_PORT, LED0_GPIO_PIN, GPIO_PIN_SET);
	HAL_GPIO_WritePin(LED1_GPIO_PORT, LED1_GPIO_PIN, GPIO_PIN_SET);
	HAL_GPIO_WritePin(LED2_GPIO_PORT, LED2_GPIO_PIN, GPIO_PIN_SET);
	HAL_GPIO_WritePin(LED3_GPIO_PORT, LED3_GPIO_PIN, GPIO_PIN_SET);
}

void led_ctrl(int led_num, int led_state)
{
	// 低电平点亮（LED_ON=0 对应 GPIO_PIN_RESET），高电平熄灭（LED_OFF=1 对应 GPIO_PIN_SET）
	GPIO_PinState pin_state = (led_state == LED_ON) ? GPIO_PIN_RESET : GPIO_PIN_SET;

	switch(led_num)
	{
		case LED0:
			HAL_GPIO_WritePin(LED0_GPIO_PORT, LED0_GPIO_PIN, pin_state);
			break;
		case LED1:
			HAL_GPIO_WritePin(LED1_GPIO_PORT, LED1_GPIO_PIN, pin_state);
			break;
		case LED2:
			HAL_GPIO_WritePin(LED2_GPIO_PORT, LED2_GPIO_PIN, pin_state);
			break;
		case LED3:
			HAL_GPIO_WritePin(LED3_GPIO_PORT, LED3_GPIO_PIN, pin_state);
			break;
	}
}

void led_all_on(void)
{
	// 共阳极电路，低电平点亮：全部拉低
	HAL_GPIO_WritePin(LED0_GPIO_PORT, LED0_GPIO_PIN, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LED1_GPIO_PORT, LED1_GPIO_PIN, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LED2_GPIO_PORT, LED2_GPIO_PIN, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LED3_GPIO_PORT, LED3_GPIO_PIN, GPIO_PIN_RESET);
}

void led_all_off(void)
{
	// 共阳极电路，高电平熄灭：全部拉高
	HAL_GPIO_WritePin(LED0_GPIO_PORT, LED0_GPIO_PIN, GPIO_PIN_SET);
	HAL_GPIO_WritePin(LED1_GPIO_PORT, LED1_GPIO_PIN, GPIO_PIN_SET);
	HAL_GPIO_WritePin(LED2_GPIO_PORT, LED2_GPIO_PIN, GPIO_PIN_SET);
	HAL_GPIO_WritePin(LED3_GPIO_PORT, LED3_GPIO_PIN, GPIO_PIN_SET);
}

void led_toggle(int led_num)
{
	switch(led_num)
	{
		case LED0:
			HAL_GPIO_TogglePin(LED0_GPIO_PORT, LED0_GPIO_PIN);
			break;
		case LED1:
			HAL_GPIO_TogglePin(LED1_GPIO_PORT, LED1_GPIO_PIN);
			break;
		case LED2:
			HAL_GPIO_TogglePin(LED2_GPIO_PORT, LED2_GPIO_PIN);
			break;
		case LED3:
			HAL_GPIO_TogglePin(LED3_GPIO_PORT, LED3_GPIO_PIN);
			break;
	}
}

void led_all_toggle(void)
{
	HAL_GPIO_TogglePin(LED0_GPIO_PORT, LED0_GPIO_PIN);
	HAL_GPIO_TogglePin(LED1_GPIO_PORT, LED1_GPIO_PIN);
	HAL_GPIO_TogglePin(LED2_GPIO_PORT, LED2_GPIO_PIN);
	HAL_GPIO_TogglePin(LED3_GPIO_PORT, LED3_GPIO_PIN);
}
