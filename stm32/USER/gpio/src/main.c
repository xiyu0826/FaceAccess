#include "stm32f1xx_hal.h"
#include "led.h"
#include "delay.h"

/**
  * @brief  System Clock Configuration
  *         System Clock source            = PLL (HSE)
  *         SYSCLK(Hz)                     = 72000000
  *         HCLK(Hz)                       = 72000000
  *         AHB Prescaler                  = 1
  *         APB1 Prescaler                 = 2
  *         APB2 Prescaler                 = 1
  *         PLLMUL                         = 9
  *         Flash Latency(WS)              = 2
  * @param  None
  * @retval None
  */
static void SystemClock_Config(void)
{
  RCC_OscInitTypeDef oscinitstruct = {0};
  RCC_ClkInitTypeDef clkinitstruct = {0};

  /* 1. 配置外部晶振 + PLL */
  oscinitstruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  oscinitstruct.HSEState       = RCC_HSE_ON;
  oscinitstruct.HSIState       = RCC_HSI_ON;
  oscinitstruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  oscinitstruct.PLL.PLLState   = RCC_PLL_ON;
  oscinitstruct.PLL.PLLSource  = RCC_PLLSOURCE_HSE;
  oscinitstruct.PLL.PLLMUL     = RCC_PLL_MUL9;   /* 8MHz x 9 = 72MHz */

  if (HAL_RCC_OscConfig(&oscinitstruct) != HAL_OK)
  {
    while (1);
  }

  /* 2. 选择 PLL 作为系统时钟，配置总线分频 */
  clkinitstruct.ClockType = RCC_CLOCKTYPE_SYSCLK |
                            RCC_CLOCKTYPE_HCLK   |
                            RCC_CLOCKTYPE_PCLK1  |
                            RCC_CLOCKTYPE_PCLK2;

  clkinitstruct.SYSCLKSource   = RCC_SYSCLKSOURCE_PLLCLK;
  clkinitstruct.AHBCLKDivider  = RCC_SYSCLK_DIV1;   /* HCLK  = 72 MHz */
  clkinitstruct.APB1CLKDivider = RCC_HCLK_DIV2;     /* PCLK1 = 36 MHz */
  clkinitstruct.APB2CLKDivider = RCC_HCLK_DIV1;     /* PCLK2 = 72 MHz */

  if (HAL_RCC_ClockConfig(&clkinitstruct, FLASH_LATENCY_2) != HAL_OK)
  {
    while (1);
  }
}

int main(void)
{
	//    1、初始化
	HAL_Init();                                      //    HAL库正常运行初始化
	SystemClock_Config();                            //    系统时钟配置 72MHz

	led_init();                                      //    LED GPIO 初始化（含 SWD 关闭）
	led_all_on();                                    //    4 颗 LED 全部常亮

	//    2、常亮主循环（灯已点亮，空转保持）
	while (1)
	{
	}
}
