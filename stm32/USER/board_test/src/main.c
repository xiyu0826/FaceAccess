/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : STM32F103RCT6 Board Self-Test v1.0
  ******************************************************************************
  */

#include "stm32f1xx_hal.h"
#include "uart_debug.h"
#include "i2c_scanner.h"
#include "adc_test.h"
#include "led_all.h"
#include "led.h"
#include "key.h"
#include "beep.h"
#include "delay.h"
#include "oled.h"
#include "wifi_test.h"
#include "bt_test.h"
#include "uart_scan.h"
#include "dht11.h"
#include "ds3231.h"
#include "mq135.h"
#include "light.h"
#include "pir.h"
#include <stdio.h>

/* ---- System Clock 72MHz -------------------------------------- */
static void SystemClock_Config(void)
{
    RCC_OscInitTypeDef osc = {0};
    RCC_ClkInitTypeDef clk = {0};

    osc.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    osc.HSEState       = RCC_HSE_ON;
    osc.HSIState       = RCC_HSI_ON;
    osc.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
    osc.PLL.PLLState   = RCC_PLL_ON;
    osc.PLL.PLLSource  = RCC_PLLSOURCE_HSE;
    osc.PLL.PLLMUL     = RCC_PLL_MUL9;

    if (HAL_RCC_OscConfig(&osc) != HAL_OK) while (1);

    clk.ClockType      = RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK |
                         RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    clk.SYSCLKSource   = RCC_SYSCLKSOURCE_PLLCLK;
    clk.AHBCLKDivider  = RCC_SYSCLK_DIV1;
    clk.APB1CLKDivider = RCC_HCLK_DIV2;
    clk.APB2CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&clk, FLASH_LATENCY_2) != HAL_OK) while (1);
}

/* ---- Banner -------------------------------------------------- */
static void power_on_banner(void)
{
    printf("\r\n\r\n");
    printf("============================================\r\n");
    printf("  STM32F103RCT6 Board Self-Test v1.1\r\n");
    printf("============================================\r\n");
    printf("\r\nClock: 72MHz | I2C1 | ADC1 | UART1\r\n");
    printf("Sensors: DHT11 | DS3231 | MQ-135 | Light | PIR\r\n\r\n");
}

/* ---- Menu ---------------------------------------------------- */
static void print_menu(void)
{
    printf("\r\n");
    printf("------------------ MENU ------------------\r\n");
    printf("  [Board]\r\n");
    printf("  1. LED Blink       (3 rounds)\r\n");
    printf("  2. LED All ON/OFF  (toggle)\r\n");
    printf("  3. KEY Test        (press to see event)\r\n");
    printf("  4. BEEP Test       (500ms burst)\r\n");
    printf("  5. I2C Scan        (scan bus)\r\n");
    printf("  6. ADC Scan All    (all channels)\r\n");
    printf("  7. ADC Continuous  (single channel)\r\n");
    printf("  8. OLED Test       (init + demo)\r\n");
    printf("  [Comms]\r\n");
    printf("  9. WiFi Test       (ESP8266 AT)\r\n");
    printf("  w. WiFi Connect    (input SSID+PW)\r\n");
    printf("  0. BT Test         (Bluetooth AT)\r\n");
    printf("  b. BT Bridge       (PC<->Phone chat)\r\n");
    printf("  s. UART Scan       (find all modules)\r\n");
    printf("  [Sensors]\r\n");
    printf("  t. DHT11 Test      (temp & humidity)\r\n");
    printf("  d. DS3231 Test     (RTC time)\r\n");
    printf("  D. DS3231 Set Time  (input YY MM DD HH MM SS W)\r\n");
    printf("  m. MQ-135 Test     (air quality)\r\n");
    printf("  l. Light Test      (photoresistor)\r\n");
    printf("  p. PIR Test        (human detection)\r\n");
    printf("  a. Light Auto LED  (toggle ON/OFF)\r\n");
    printf("  [System]\r\n");
    printf("  ?. Print Menu\r\n");
    printf("  !. Run All Tests   (10 steps)\r\n");
    printf("-------------------------------------------\r\n");
    printf("Enter choice > ");
}

/* ---- KEY Test ------------------------------------------------ */
static void key_test_mode(void)
{
    printf("\r\n--- KEY Test (press 'q' to exit) ---\r\n");
    printf("KEY0=PA1  KEY1=PA7  KEY2=PC4  KEY3=PC5\r\n");

    uint32_t last_tick = 0;

    while (1)
    {
        int c = uart_getchar();
        if (c == 'q' || c == 'Q') break;

        if (HAL_GetTick() - last_tick < 50) continue;
        last_tick = HAL_GetTick();

        if (key_pressed(KEY0))
            printf("  [%8lu ms] KEY0 (PA1) PRESSED\r\n", HAL_GetTick());
        if (key_pressed(KEY1))
            printf("  [%8lu ms] KEY1 (PA7) PRESSED\r\n", HAL_GetTick());
        if (key_pressed(KEY2))
            printf("  [%8lu ms] KEY2 (PC4) PRESSED\r\n", HAL_GetTick());
        if (key_pressed(KEY3))
            printf("  [%8lu ms] KEY3 (PC5) PRESSED\r\n", HAL_GetTick());
    }
    printf("KEY test exited.\r\n");
}

/* ---- ADC Continuous sub-menu --------------------------------- */
static void adc_cont_mode(void)
{
    printf("\r\nSelect ADC channel:\r\n");
    printf("  0=PA0(IN0)  1=PA4(IN4)  2=PA5(IN5)  3=PA6(IN6)\r\n");
    printf("  4=PB0(IN8)  5=PB1(IN9)  6=PC0(IN10) 7=PC1(IN11)\r\n");
    printf("  8=PC2(IN12) 9=PC3(IN13)\r\n");
    printf("Enter channel (0-9) > ");

    int c = getchar();
    printf("%c\r\n", c);

    if (c >= '0' && c <= '9')
        adc_scan_channel(c - '0');
    else
        printf("Invalid.\r\n");
}

/* ---- Run All Tests ------------------------------------------- */
static void run_all_tests(void)
{
    printf("\r\n");
    printf("============================================\r\n");
    printf("         RUNNING ALL TESTS\r\n");
    printf("============================================\r\n\r\n");

    printf("[ 1/10] LED Blink...\r\n");
    led_all_self_test();

    printf("[ 2/10] BEEP...\r\n");
    beep_burst(500);

    printf("[ 3/10] I2C Scan...\r\n");
    i2c_scan();

    printf("[ 4/10] ADC Scan...\r\n");
    adc_scan_all();

    printf("[ 5/10] LED Toggle...\r\n");
    led_all_on();
    delay_ms(500);
    led_all_off();

    printf("[ 6/10] OLED Test...\r\n");
    oled_init();
    oled_test_demo();

    printf("[ 7/10] DHT11 Temp&Humi...\r\n");
    {
        dht11_data_t d;
        if (dht11_read(&d) == DHT11_OK)
            printf("        T=%dC H=%d%%\r\n", d.temp, d.humi);
        else
            printf("        FAIL!\r\n");
    }

    printf("[ 8/10] DS3231 RTC...\r\n");
    ds3231_print_time();

    printf("[ 9/10] MQ-135 Air Quality...\r\n");
    {
        float v = mq135_read_voltage();
        printf("        AO=%.2fV AQI=%s\r\n", v, mq135_aqi_str(mq135_get_aqi()));
    }

    printf("[10/10] Light Sensor...\r\n");
    {
        float v = light_read_voltage();
        printf("        %.2fV Level=%s\r\n", v, light_level_str(light_get_level()));
    }

    printf("\r\nALL TESTS DONE!\r\n");
}

/* ---- main ---------------------------------------------------- */
int main(void)
{
    int cmd;

    /* Init */
    HAL_Init();
    SystemClock_Config();

    uart_debug_init();
    led_all_init();
    key_init();
    beep_init();
    i2c_scanner_init();
    adc_test_init();
    dht11_init();
    ds3231_init();
    mq135_init(ADC_CHANNEL_8);      /* H9_8 → PB0 */
    light_init(ADC_CHANNEL_6);       /* H8_6 → PA6 */
    pir_init();

    /* Banner */
    power_on_banner();
    beep_burst(200);
    delay_ms(300);

    printf("Status:\r\n");
    printf("  UART1  @ 115200   [OK]\r\n");
    printf("  I2C1   @ 100kHz   [READY]\r\n");
    printf("  ADC1   @ 12-bit   [READY]\r\n");
    printf("  LEDs   PA11-14    [READY]\r\n");
    printf("  KEYs   x4         [READY]\r\n");
    printf("  BEEP   PA8        [READY]\r\n");
    printf("  OLED   (press 8)  [READY]\r\n");
    printf("  WiFi   (press 9)  [READY]\r\n");
    printf("  BT     (press 0)  [READY]\r\n");
    printf("  DHT11  PC1 H9_5   [READY]\r\n");
    printf("  DS3231 I2C 0x68   [READY]\r\n");
    printf("  MQ-135 H9_8 ADC1  [READY]\r\n");
    printf("  Light  H8_6 ADC1  [READY]\r\n");
    printf("  PIR    PB12 H8_11 [READY]\r\n");

    print_menu();

    while (1)
    {
        cmd = getchar();

        switch (cmd)
        {
        case '1': printf("\r\n"); led_all_self_test(); break;
        case '2': printf("\r\n"); led_all_on(); delay_ms(500); led_all_off(); printf("Done.\r\n"); break;
        case '3': printf("\r\n"); key_test_mode(); break;
        case '4': printf("\r\nBEEP 500ms...\r\n"); beep_burst(500); break;
        case '5': printf("\r\n"); i2c_scan(); break;
        case '6': printf("\r\n"); adc_scan_all(); break;
        case '7': printf("\r\n"); adc_cont_mode(); break;
        case '8': printf("\r\n"); oled_init(); oled_test_demo(); break;
        case '9': printf("\r\n"); wifi_test_init(); wifi_test_at(); break;
        case 'w': printf("\r\n"); wifi_test_init(); wifi_connect(); break;
        case '0': printf("\r\n"); bt_test_init(); bt_test_at(); break;
        case 'b': printf("\r\n"); bt_test_init(); bt_bridge(); break;
        case 's': printf("\r\n"); uart_scan_all(); break;
        /* ---- 传感器测试 ---- */
        case 't': printf("\r\n"); dht11_test(); break;
        case 'd': printf("\r\n"); ds3231_print_time(); break;
        case 'D': printf("\r\n"); ds3231_set_prompt(); break;
        case 'm': printf("\r\n"); mq135_test(); break;
        case 'l': printf("\r\n"); light_test(); break;
        case 'p': printf("\r\n"); pir_test(); break;
        case 'a': printf("\r\n");
                  { static int auto_led = 0;
                    auto_led = !auto_led;
                    light_auto_led(auto_led); }
                  break;
        case '?': printf("\r\n"); print_menu(); break;
        case '!': printf("\r\n"); run_all_tests(); break;
        case '\r': case '\n': break;
        default:
            if (cmd > 0) printf("\r\n'%c' unknown, press '?' for menu\r\n", cmd);
            break;
        }

        if (cmd != '\r' && cmd != '\n')
            printf("\r\nEnter choice > ");
    }
}
