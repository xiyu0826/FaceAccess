#include "servo.h"

static TIM_HandleTypeDef htim_servo;

/*
 * 初始化舵机 PWM
 * 定时器时钟 72MHz，预分频 72-1=71 → 计数频率 1MHz（每 1us 计一次数）
 * ARR = 20000-1 → 周期 20ms = 50Hz
 * CCR = 脉宽微秒数：500=0°  1500=90°  2500=180°
 */
void servo_init(void)
{
    GPIO_InitTypeDef  gpio = {0};
    TIM_OC_InitTypeDef oc   = {0};

    // 1. 使能 GPIO 和定时器时钟
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_TIM3_CLK_ENABLE();

    // 2. GPIO 配置为复用推挽输出（PB1 = TIM3_CH4）
    gpio.Pin   = SERVO_GPIO_PIN;
    gpio.Mode  = GPIO_MODE_AF_PP;
    gpio.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(SERVO_GPIO_PORT, &gpio);

    // 3. 定时器基础配置
    htim_servo.Instance               = SERVO_TIM;
    htim_servo.Init.Prescaler         = 72 - 1;               // 1MHz
    htim_servo.Init.CounterMode       = TIM_COUNTERMODE_UP;
    htim_servo.Init.Period            = 20000 - 1;            // 20ms = 50Hz
    htim_servo.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;
    htim_servo.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
    HAL_TIM_PWM_Init(&htim_servo);

    // 4. PWM 通道配置
    oc.OCMode     = TIM_OCMODE_PWM1;
    oc.Pulse      = 1500;              // 初始 90°（供电瞬间先到中间位，避免抖动）
    oc.OCPolarity = TIM_OCPOLARITY_HIGH;
    oc.OCFastMode = TIM_OCFAST_DISABLE;
    HAL_TIM_PWM_ConfigChannel(&htim_servo, &oc, SERVO_CHANNEL);

    // 5. 启动 PWM，并回到上锁位置 (0°)
    HAL_TIM_PWM_Start(&htim_servo, SERVO_CHANNEL);
    servo_close();
}

void servo_set_angle(int angle)
{
    if (angle < 0)   angle = 0;
    if (angle > 180) angle = 180;

    // 线性映射：0°->500us，180°->2500us
    uint32_t pulse = 500 + (uint32_t)(2000UL * angle / 180);

    __HAL_TIM_SET_COMPARE(&htim_servo, SERVO_CHANNEL, pulse);
}

void servo_open(void)
{
    servo_set_angle(SERVO_ANGLE_OPEN);
}

void servo_close(void)
{
    servo_set_angle(SERVO_ANGLE_CLOSED);
}

uint32_t servo_get_pulse(void)
{
    return __HAL_TIM_GET_COMPARE(&htim_servo, SERVO_CHANNEL);
}
