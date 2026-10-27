#include "stm32f4xx_hal.h"
#include <stdint.h>

#define STEPPER_GPIO_PORT       GPIOC
#define STEPPER_IN1_PIN         GPIO_PIN_0
#define STEPPER_IN2_PIN         GPIO_PIN_1
#define STEPPER_IN3_PIN         GPIO_PIN_2
#define STEPPER_IN4_PIN         GPIO_PIN_3
#define STEPPER_ALL_PINS        (STEPPER_IN1_PIN | STEPPER_IN2_PIN | \
                                 STEPPER_IN3_PIN | STEPPER_IN4_PIN)

#define LED_GPIO_PORT           GPIOF
#define LED_PINS                (GPIO_PIN_9 | GPIO_PIN_10)
#define LED_TOGGLE_MS           500U

static void SystemClock_Config(void);
static void Error_Handler(void);
static void Stepper_GPIO_Init(void);
static void LED_GPIO_Init(void);
static void Stepper_SetPins(uint16_t pins);
static void Stepper_Off(void);
static void Delay_WithLedBlink(uint32_t delay_ms);
static void GPIO_ConfirmLoop(void);

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    LED_GPIO_Init();
    Stepper_GPIO_Init();

    Stepper_SetPins(STEPPER_ALL_PINS);
    Delay_WithLedBlink(10000U);

    GPIO_ConfirmLoop();
}

static void Stepper_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOC_CLK_ENABLE();

    HAL_GPIO_WritePin(STEPPER_GPIO_PORT, STEPPER_ALL_PINS, GPIO_PIN_RESET);

    GPIO_InitStruct.Pin = STEPPER_ALL_PINS;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(STEPPER_GPIO_PORT, &GPIO_InitStruct);
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

static void Stepper_SetPins(uint16_t pins)
{
    HAL_GPIO_WritePin(STEPPER_GPIO_PORT, STEPPER_ALL_PINS, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(STEPPER_GPIO_PORT, pins & STEPPER_ALL_PINS, GPIO_PIN_SET);
}

static void Stepper_Off(void)
{
    HAL_GPIO_WritePin(STEPPER_GPIO_PORT, STEPPER_ALL_PINS, GPIO_PIN_RESET);
}

static void Delay_WithLedBlink(uint32_t delay_ms)
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

static void GPIO_ConfirmLoop(void)
{
    while (1)
    {
        Stepper_SetPins(STEPPER_IN1_PIN);
        Delay_WithLedBlink(3000U);

        Stepper_SetPins(STEPPER_IN2_PIN);
        Delay_WithLedBlink(3000U);

        Stepper_SetPins(STEPPER_IN3_PIN);
        Delay_WithLedBlink(3000U);

        Stepper_SetPins(STEPPER_IN4_PIN);
        Delay_WithLedBlink(3000U);

        Stepper_Off();
        Delay_WithLedBlink(2000U);
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
    Stepper_Off();
    __disable_irq();
    while (1)
    {
    }
}
