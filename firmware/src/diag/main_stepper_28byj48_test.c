#include "stm32f4xx_hal.h"
#include <stdint.h>

#define STEPPER_GPIO_PORT       GPIOB
#define STEPPER_IN1_PIN         GPIO_PIN_6
#define STEPPER_IN2_PIN         GPIO_PIN_7
#define STEPPER_IN3_PIN         GPIO_PIN_8
#define STEPPER_IN4_PIN         GPIO_PIN_9
#define STEPPER_ALL_PINS        (STEPPER_IN1_PIN | STEPPER_IN2_PIN | \
                                 STEPPER_IN3_PIN | STEPPER_IN4_PIN)

#define STEPPER_FULL_TURN_STEPS 4096U
#define STEPPER_STEP_DELAY_MS   3U
#define STEPPER_STOP_MS         1000U

static const uint16_t half_step_table[8] = {
    STEPPER_IN1_PIN,
    STEPPER_IN1_PIN | STEPPER_IN2_PIN,
    STEPPER_IN2_PIN,
    STEPPER_IN2_PIN | STEPPER_IN3_PIN,
    STEPPER_IN3_PIN,
    STEPPER_IN3_PIN | STEPPER_IN4_PIN,
    STEPPER_IN4_PIN,
    STEPPER_IN4_PIN | STEPPER_IN1_PIN
};

static void SystemClock_Config(void);
static void Error_Handler(void);
static void Stepper_GPIO_Init(void);
static void Stepper_SetPhase(uint8_t phase);
static void Stepper_Off(void);
static void Stepper_Run(uint8_t reverse, uint32_t steps);

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    Stepper_GPIO_Init();
    Stepper_Off();

    while (1)
    {
        Stepper_Run(0U, STEPPER_FULL_TURN_STEPS);
        Stepper_Off();
        HAL_Delay(STEPPER_STOP_MS);

        Stepper_Run(1U, STEPPER_FULL_TURN_STEPS);
        Stepper_Off();
        HAL_Delay(STEPPER_STOP_MS);
    }
}

static void Stepper_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOB_CLK_ENABLE();

    HAL_GPIO_WritePin(STEPPER_GPIO_PORT, STEPPER_ALL_PINS, GPIO_PIN_RESET);

    GPIO_InitStruct.Pin = STEPPER_ALL_PINS;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(STEPPER_GPIO_PORT, &GPIO_InitStruct);
}

static void Stepper_SetPhase(uint8_t phase)
{
    uint16_t pins = half_step_table[phase & 0x07U];

    HAL_GPIO_WritePin(STEPPER_GPIO_PORT, STEPPER_ALL_PINS, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(STEPPER_GPIO_PORT, pins, GPIO_PIN_SET);
}

static void Stepper_Off(void)
{
    HAL_GPIO_WritePin(STEPPER_GPIO_PORT, STEPPER_ALL_PINS, GPIO_PIN_RESET);
}

static void Stepper_Run(uint8_t reverse, uint32_t steps)
{
    uint32_t i;
    uint8_t phase;

    for (i = 0U; i < steps; i++)
    {
        phase = (uint8_t)(i & 0x07U);
        if (reverse != 0U)
        {
            phase = (uint8_t)(7U - phase);
        }

        Stepper_SetPhase(phase);
        HAL_Delay(STEPPER_STEP_DELAY_MS);
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
