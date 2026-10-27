#include "stm32f4xx_hal.h"
#include <stdint.h>
#include <string.h>

#define SERVO_GPIO_PORT     GPIOB
#define SERVO_GPIO_PINS     (GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9)
#define SERVO_PERIOD_US     20000U
#define SERVO_STOP_US       1500U
#define SERVO_REVERSE_US    900U
#define SERVO_FORWARD_US    2100U

static UART_HandleTypeDef huart2;

static void SystemClock_Config(void);
static void Error_Handler(void);
static void DWT_Delay_Init(void);
static void delay_us(uint32_t us);
static void USART2_UART_Init(void);
static void Servo_GPIO_Init(void);
static void Servo_Print(const char *text);
static void servo_soft_pwm_output(uint16_t pulse_us, uint32_t duration_ms);

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    DWT_Delay_Init();
    USART2_UART_Init();
    Servo_GPIO_Init();

    while (1)
    {
        Servo_Print("STOP 1500US\r\n");
        servo_soft_pwm_output(SERVO_STOP_US, 3000U);

        Servo_Print("REVERSE 900US\r\n");
        servo_soft_pwm_output(SERVO_REVERSE_US, 6000U);

        Servo_Print("STOP 1500US\r\n");
        servo_soft_pwm_output(SERVO_STOP_US, 3000U);

        Servo_Print("FORWARD 2100US\r\n");
        servo_soft_pwm_output(SERVO_FORWARD_US, 6000U);

        Servo_Print("STOP 1500US\r\n");
        servo_soft_pwm_output(SERVO_STOP_US, 3000U);
    }
}

static void Servo_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOB_CLK_ENABLE();

    HAL_GPIO_WritePin(SERVO_GPIO_PORT, SERVO_GPIO_PINS, GPIO_PIN_RESET);

    GPIO_InitStruct.Pin = SERVO_GPIO_PINS;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(SERVO_GPIO_PORT, &GPIO_InitStruct);
}

static void servo_soft_pwm_output(uint16_t pulse_us, uint32_t duration_ms)
{
    uint32_t start_ms;
    uint32_t low_us;

    if (pulse_us < 500U)
    {
        pulse_us = 500U;
    }
    if (pulse_us > 2500U)
    {
        pulse_us = 2500U;
    }

    low_us = SERVO_PERIOD_US - pulse_us;
    start_ms = HAL_GetTick();

    while ((HAL_GetTick() - start_ms) < duration_ms)
    {
        HAL_GPIO_WritePin(SERVO_GPIO_PORT, SERVO_GPIO_PINS, GPIO_PIN_SET);
        delay_us(pulse_us);
        HAL_GPIO_WritePin(SERVO_GPIO_PORT, SERVO_GPIO_PINS, GPIO_PIN_RESET);
        delay_us(low_us);
    }
}

static void DWT_Delay_Init(void)
{
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0U;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

static void delay_us(uint32_t us)
{
    uint32_t start = DWT->CYCCNT;
    uint32_t ticks = us * (HAL_RCC_GetHCLKFreq() / 1000000U);

    while ((DWT->CYCCNT - start) < ticks)
    {
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
