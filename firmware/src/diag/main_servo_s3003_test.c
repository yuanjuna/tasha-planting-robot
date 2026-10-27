#include "stm32f4xx_hal.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define SERVO_PERIOD_US     20000U
#define SERVO_MIN_US        1000U
#define SERVO_MID_US        1500U
#define SERVO_MAX_US        2000U

static TIM_HandleTypeDef htim4;
static UART_HandleTypeDef huart2;

static const uint16_t servo_angles[] = {
    0U, 15U, 30U, 45U, 60U, 75U, 90U,
    105U, 120U, 135U, 150U, 165U, 180U
};

static void SystemClock_Config(void);
static void Error_Handler(void);
static void USART2_UART_Init(void);
static void Servo_TIM4_CH1_PWM_Init(void);
static void Servo_SetPulseUs(uint32_t pulse_us);
static uint32_t Servo_AngleToPulseUs(uint32_t angle);
static void Servo_Print(const char *text);
static void Servo_PrintAnglePulse(uint32_t angle, uint32_t pulse_us);
static void Servo_RunForwardSweep(void);
static void Servo_RunReverseSweep(void);

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    USART2_UART_Init();
    Servo_TIM4_CH1_PWM_Init();

    Servo_SetPulseUs(SERVO_MID_US);
    HAL_Delay(2000U);
    Servo_Print("S3003 SERVO TEST START\r\n");

    while (1)
    {
        Servo_RunForwardSweep();
        Servo_RunReverseSweep();
    }
}

static void Servo_TIM4_CH1_PWM_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    TIM_ClockConfigTypeDef sClockSourceConfig = {0};
    TIM_MasterConfigTypeDef sMasterConfig = {0};
    TIM_OC_InitTypeDef sConfigOC = {0};

    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_TIM4_CLK_ENABLE();

    GPIO_InitStruct.Pin = GPIO_PIN_6;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF2_TIM4;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    htim4.Instance = TIM4;
    htim4.Init.Prescaler = 83U;
    htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim4.Init.Period = SERVO_PERIOD_US - 1U;
    htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    if (HAL_TIM_Base_Init(&htim4) != HAL_OK) Error_Handler();

    sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
    if (HAL_TIM_ConfigClockSource(&htim4, &sClockSourceConfig) != HAL_OK) Error_Handler();
    if (HAL_TIM_PWM_Init(&htim4) != HAL_OK) Error_Handler();

    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig) != HAL_OK) Error_Handler();

    sConfigOC.OCMode = TIM_OCMODE_PWM1;
    sConfigOC.Pulse = SERVO_MID_US;
    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
    if (HAL_TIM_PWM_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_1) != HAL_OK) Error_Handler();

    if (HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_1) != HAL_OK) Error_Handler();
    Servo_SetPulseUs(SERVO_MID_US);
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

    __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_1, pulse_us);
}

static uint32_t Servo_AngleToPulseUs(uint32_t angle)
{
    if (angle > 180U)
    {
        angle = 180U;
    }

    return SERVO_MIN_US + (angle * (SERVO_MAX_US - SERVO_MIN_US)) / 180U;
}

static void Servo_RunForwardSweep(void)
{
    uint32_t i;

    for (i = 0U; i < (sizeof(servo_angles) / sizeof(servo_angles[0])); i++)
    {
        uint32_t angle = servo_angles[i];
        uint32_t pulse_us = Servo_AngleToPulseUs(angle);

        Servo_PrintAnglePulse(angle, pulse_us);
        Servo_SetPulseUs(pulse_us);
        HAL_Delay(1000U);
    }
}

static void Servo_RunReverseSweep(void)
{
    int32_t i;

    for (i = (int32_t)((sizeof(servo_angles) / sizeof(servo_angles[0])) - 1U); i >= 0; i--)
    {
        uint32_t angle = servo_angles[i];
        uint32_t pulse_us = Servo_AngleToPulseUs(angle);

        Servo_PrintAnglePulse(angle, pulse_us);
        Servo_SetPulseUs(pulse_us);
        HAL_Delay(1000U);
    }
}

static void USART2_UART_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_USART2_CLK_ENABLE();

    GPIO_InitStruct.Pin = GPIO_PIN_2 | GPIO_PIN_3;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    huart2.Instance = USART2;
    huart2.Init.BaudRate = 115200U;
    huart2.Init.WordLength = UART_WORDLENGTH_8B;
    huart2.Init.StopBits = UART_STOPBITS_1;
    huart2.Init.Parity = UART_PARITY_NONE;
    huart2.Init.Mode = UART_MODE_TX_RX;
    huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart2.Init.OverSampling = UART_OVERSAMPLING_16;
    if (HAL_UART_Init(&huart2) != HAL_OK) Error_Handler();
}

static void Servo_Print(const char *text)
{
    HAL_UART_Transmit(&huart2, (uint8_t *)text, (uint16_t)strlen(text), 100U);
}

static void Servo_PrintAnglePulse(uint32_t angle, uint32_t pulse_us)
{
    char text[32];
    uint32_t len = 0U;

    len += (uint32_t)snprintf(text + len, sizeof(text) - len,
                             "ANGLE %lu PULSE %lu\r\n",
                             (unsigned long)angle,
                             (unsigned long)pulse_us);
    if (len >= sizeof(text))
    {
        len = sizeof(text) - 1U;
    }

    HAL_UART_Transmit(&huart2, (uint8_t *)text, (uint16_t)len, 100U);
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
