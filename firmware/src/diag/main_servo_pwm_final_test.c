#include "stm32f4xx_hal.h"
#include <stdint.h>
#include <string.h>

#define SERVO_MIN_US       500U
#define SERVO_STOP_US      1500U
#define SERVO_MAX_US       2500U
#define SERVO_PERIOD_US    20000U

#define LED_GPIO_PORT      GPIOF
#define LED_PINS           (GPIO_PIN_9 | GPIO_PIN_10)
#define LED_TOGGLE_MS      500U

static TIM_HandleTypeDef htim2;
static UART_HandleTypeDef huart2;

static void SystemClock_Config(void);
static void Error_Handler(void);
static void LED_GPIO_Init(void);
static void USART2_UART_Init(void);
static void Servo_TIM2_CH1_PWM_Init(void);
static void Servo_SetPulseUs(uint32_t pulse_us);
static void Servo_Print(const char *text);
static void Servo_DelayWithLed(uint32_t delay_ms);
static void Servo_PrintSetDelay(const char *text, uint32_t pulse_us, uint32_t delay_ms);

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    LED_GPIO_Init();
    USART2_UART_Init();
    Servo_TIM2_CH1_PWM_Init();

    while (1)
    {
        Servo_PrintSetDelay("SERVO 500US\r\n", SERVO_MIN_US, 5000U);
        Servo_PrintSetDelay("SERVO 1500US STOP\r\n", SERVO_STOP_US, 3000U);
        Servo_PrintSetDelay("SERVO 2500US\r\n", SERVO_MAX_US, 5000U);
        Servo_PrintSetDelay("SERVO 1500US STOP\r\n", SERVO_STOP_US, 3000U);
    }
}

static void Servo_TIM2_CH1_PWM_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    TIM_ClockConfigTypeDef sClockSourceConfig = {0};
    TIM_MasterConfigTypeDef sMasterConfig = {0};
    TIM_OC_InitTypeDef sConfigOC = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_TIM2_CLK_ENABLE();

    GPIO_InitStruct.Pin = GPIO_PIN_0;
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
    sConfigOC.Pulse = SERVO_STOP_US;
    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
    if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_1) != HAL_OK) Error_Handler();

    if (HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1) != HAL_OK) Error_Handler();
    Servo_SetPulseUs(SERVO_STOP_US);
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

    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, pulse_us);
}

static void Servo_PrintSetDelay(const char *text, uint32_t pulse_us, uint32_t delay_ms)
{
    Servo_Print(text);
    Servo_SetPulseUs(pulse_us);
    Servo_DelayWithLed(delay_ms);
}

static void Servo_DelayWithLed(uint32_t delay_ms)
{
    uint32_t start = HAL_GetTick();
    uint32_t last_toggle = start;

    while ((HAL_GetTick() - start) < delay_ms)
    {
        uint32_t now = HAL_GetTick();

        if ((now - last_toggle) >= LED_TOGGLE_MS)
        {
            last_toggle = now;
            HAL_GPIO_TogglePin(LED_GPIO_PORT, LED_PINS);
        }

        HAL_Delay(10U);
    }
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
