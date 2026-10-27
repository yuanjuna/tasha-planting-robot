#include "stm32f4xx_hal.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define SERVO_PERIOD_US     20000U
#define SERVO_MIN_US        500U
#define SERVO_MID_US        1500U
#define SERVO_MAX_US        2500U

#define LED_GPIO_PORT       GPIOF
#define LED_PINS            (GPIO_PIN_9 | GPIO_PIN_10)

static TIM_HandleTypeDef htim2;
static UART_HandleTypeDef huart1;

static const uint16_t angle_table[] = {
    0U, 45U, 90U, 135U, 180U, 135U, 90U, 45U
};

static void SystemClock_Config(void);
static void Error_Handler(void);
static void LED_GPIO_Init(void);
static void USART1_UART_Init(void);
static void Servo_PA1_TIM2_CH2_PWM_Init(void);
static uint32_t Servo_AngleToPulseUs(uint32_t angle);
static void Servo_SetPulseUs(uint32_t pulse_us);
static void Servo_SetAngle(uint32_t angle);
static void Delay_WithLed(uint32_t delay_ms);
static void Print_Angle(uint32_t angle, uint32_t pulse_us);

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    LED_GPIO_Init();
    USART1_UART_Init();
    Servo_PA1_TIM2_CH2_PWM_Init();

    Servo_SetPulseUs(SERVO_MID_US);
    Delay_WithLed(1000U);

    while (1)
    {
        uint32_t i;

        for (i = 0U; i < (sizeof(angle_table) / sizeof(angle_table[0])); i++)
        {
            Servo_SetAngle(angle_table[i]);
            Delay_WithLed(1000U);
        }
    }
}

static void Servo_PA1_TIM2_CH2_PWM_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    TIM_ClockConfigTypeDef sClockSourceConfig = {0};
    TIM_MasterConfigTypeDef sMasterConfig = {0};
    TIM_OC_InitTypeDef sConfigOC = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_TIM2_CLK_ENABLE();

    GPIO_InitStruct.Pin = GPIO_PIN_1;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF1_TIM2;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    htim2.Instance = TIM2;
    htim2.Init.Prescaler = 83U;
    htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim2.Init.Period = SERVO_PERIOD_US - 1U;
    htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    if (HAL_TIM_Base_Init(&htim2) != HAL_OK) Error_Handler();

    sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
    if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK) Error_Handler();
    if (HAL_TIM_PWM_Init(&htim2) != HAL_OK) Error_Handler();

    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK) Error_Handler();

    sConfigOC.OCMode = TIM_OCMODE_PWM1;
    sConfigOC.Pulse = SERVO_MID_US;
    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
    if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_2) != HAL_OK) Error_Handler();

    if (HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2) != HAL_OK) Error_Handler();
    Servo_SetPulseUs(SERVO_MID_US);
}

static uint32_t Servo_AngleToPulseUs(uint32_t angle)
{
    if (angle > 180U)
    {
        angle = 180U;
    }

    return SERVO_MIN_US + (angle * (SERVO_MAX_US - SERVO_MIN_US)) / 180U;
}

static void Servo_SetPulseUs(uint32_t pulse_us)
{
    if (pulse_us < SERVO_MIN_US)
    {
        pulse_us = SERVO_MIN_US;
    }
    if (pulse_us > SERVO_MAX_US)
    {
        pulse_us = SERVO_MAX_US;
    }

    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, pulse_us);
}

static void Servo_SetAngle(uint32_t angle)
{
    uint32_t pulse_us = Servo_AngleToPulseUs(angle);

    Servo_SetPulseUs(pulse_us);
    Print_Angle(angle, pulse_us);
}

static void LED_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOF_CLK_ENABLE();

    GPIO_InitStruct.Pin = LED_PINS;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(LED_GPIO_PORT, &GPIO_InitStruct);

    HAL_GPIO_WritePin(LED_GPIO_PORT, LED_PINS, GPIO_PIN_SET);
}

static void USART1_UART_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_USART1_CLK_ENABLE();

    GPIO_InitStruct.Pin = GPIO_PIN_9 | GPIO_PIN_10;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    huart1.Instance = USART1;
    huart1.Init.BaudRate = 115200U;
    huart1.Init.WordLength = UART_WORDLENGTH_8B;
    huart1.Init.StopBits = UART_STOPBITS_1;
    huart1.Init.Parity = UART_PARITY_NONE;
    huart1.Init.Mode = UART_MODE_TX_RX;
    huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart1.Init.OverSampling = UART_OVERSAMPLING_16;
    if (HAL_UART_Init(&huart1) != HAL_OK) Error_Handler();
}

static void Delay_WithLed(uint32_t delay_ms)
{
    uint32_t start = HAL_GetTick();
    uint32_t last_toggle = start;

    while ((HAL_GetTick() - start) < delay_ms)
    {
        uint32_t now = HAL_GetTick();

        if ((now - last_toggle) >= 500U)
        {
            last_toggle = now;
            HAL_GPIO_TogglePin(LED_GPIO_PORT, LED_PINS);
        }

        HAL_Delay(10U);
    }
}

static void Print_Angle(uint32_t angle, uint32_t pulse_us)
{
    char text[40];
    int len = snprintf(text, sizeof(text), "ANGLE %lu PULSE %lu\r\n",
                       (unsigned long)angle,
                       (unsigned long)pulse_us);

    if (len > 0)
    {
        HAL_UART_Transmit(&huart1, (uint8_t *)text, (uint16_t)len, 100U);
    }
}

static void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLM = 8U;
    RCC_OscInitStruct.PLL.PLLN = 336U;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
    RCC_OscInitStruct.PLL.PLLQ = 7U;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) Error_Handler();

    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK |
                                  RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;
    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK) Error_Handler();
}

static void Error_Handler(void)
{
    __disable_irq();
    while (1)
    {
    }
}
