#include "app.h"

void Seedling_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    __HAL_RCC_GPIOB_CLK_ENABLE();
    HAL_GPIO_WritePin(SEEDLING_STEP_GPIO_PORT, SEEDLING_STEP_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(SEEDLING_DIR_GPIO_PORT, SEEDLING_DIR_PIN, DIR_FORWARD_LEVEL);
    GPIO_InitStruct.Pin = SEEDLING_STEP_PIN | SEEDLING_DIR_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

void Mobile_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOC_CLK_ENABLE();
    HAL_GPIO_WritePin(MOBILE_STEP_GPIO_PORT, MOBILE_STEP_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(MOBILE_DIR_GPIO_PORT, MOBILE_DIR_PIN, DIR_FORWARD_LEVEL);

    GPIO_InitStruct.Pin = MOBILE_STEP_PIN | MOBILE_DIR_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
}

void DWT_Delay_Init(void)
{
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

void delay_us(uint32_t us)
{
    uint32_t start = DWT->CYCCNT;
    uint32_t ticks = us * (HAL_RCC_GetHCLKFreq() / 1000000U);
    while ((DWT->CYCCNT - start) < ticks)
    {
    }
}

void Seedling_Stop(void)
{
    HAL_GPIO_WritePin(SEEDLING_STEP_GPIO_PORT, SEEDLING_STEP_PIN, GPIO_PIN_RESET);
    HAL_Delay(STOP_STABLE_MS);
}

void Seedling_SetDir(uint8_t dir)
{
    Seedling_Stop();
    HAL_GPIO_WritePin(SEEDLING_DIR_GPIO_PORT, SEEDLING_DIR_PIN,
                      (dir == FORWARD) ? DIR_FORWARD_LEVEL : DIR_REVERSE_LEVEL);
    HAL_Delay(DIR_STABLE_MS);
}

void Seedling_OnePulse(uint32_t period_us)
{
    if (period_us <= STEP_HIGH_US + 10U) period_us = STEP_HIGH_US + 10U;
    HAL_GPIO_WritePin(SEEDLING_STEP_GPIO_PORT, SEEDLING_STEP_PIN, GPIO_PIN_SET);
    delay_us(STEP_HIGH_US);
    HAL_GPIO_WritePin(SEEDLING_STEP_GPIO_PORT, SEEDLING_STEP_PIN, GPIO_PIN_RESET);
    delay_us(period_us - STEP_HIGH_US);
}

void Seedling_RunMs(uint8_t dir, uint32_t run_ms)
{
    uint32_t elapsed = 0U;
    Seedling_SetDir(dir);
    while (elapsed < run_ms)
    {
        uint32_t start_tick;
        uint32_t period_us = 1000000U / SEEDLING_TESTED_FREQ_HZ;

        App_WaitIfPaused();
        start_tick = HAL_GetTick();
        Seedling_OnePulse(period_us);
        elapsed += HAL_GetTick() - start_tick;
    }
    Seedling_Stop();
}

void Mobile_Stop(void)
{
    HAL_GPIO_WritePin(MOBILE_STEP_GPIO_PORT, MOBILE_STEP_PIN, GPIO_PIN_SET);
    HAL_Delay(STOP_STABLE_MS);
}

void Mobile_SetDir(uint8_t dir)
{
    Mobile_Stop();
    HAL_GPIO_WritePin(MOBILE_DIR_GPIO_PORT, MOBILE_DIR_PIN,
                      (dir == FORWARD) ? GPIO_PIN_RESET : GPIO_PIN_SET);
    HAL_Delay(DIR_STABLE_MS);
}

void Mobile_OnePulse(uint32_t period_us)
{
    if (period_us <= MOBILE_STEP_LOW_US + 10U) period_us = MOBILE_STEP_LOW_US + 10U;
    HAL_GPIO_WritePin(MOBILE_STEP_GPIO_PORT, MOBILE_STEP_PIN, GPIO_PIN_RESET);
    delay_us(MOBILE_STEP_LOW_US);
    HAL_GPIO_WritePin(MOBILE_STEP_GPIO_PORT, MOBILE_STEP_PIN, GPIO_PIN_SET);
    delay_us(period_us - MOBILE_STEP_LOW_US);
}

void Mobile_RunMs(uint8_t dir, uint32_t run_ms)
{
    uint32_t elapsed = 0U;
    Mobile_SetDir(dir);
    while (elapsed < run_ms)
    {
        uint32_t start_tick;
        uint32_t period_us = 1000000U / MOBILE_TESTED_FREQ_HZ;

        App_WaitIfPaused();
        start_tick = HAL_GetTick();
        Mobile_OnePulse(period_us);
        elapsed += HAL_GetTick() - start_tick;
    }
    Mobile_Stop();
}

