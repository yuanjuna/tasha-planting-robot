#include "app.h"

static volatile uint32_t ultrasonic_rise_cycle;
static volatile uint32_t ultrasonic_last_trig_cycle;
static volatile uint8_t ultrasonic_wait_echo;
static volatile uint8_t ultrasonic_seen_rise;
static uint32_t ultrasonic_last_trig_ms;
static uint32_t ultrasonic_samples[ULTRASONIC_FILTER_SIZE];
static uint8_t ultrasonic_sample_count;
static uint8_t ultrasonic_sample_index;

static uint32_t Ultrasonic_TicksPerUs(void)
{
    return HAL_RCC_GetHCLKFreq() / 1000000U;
}

static uint32_t Ultrasonic_FilterPush(uint32_t sample_mm)
{
    uint32_t sorted[ULTRASONIC_FILTER_SIZE];
    uint32_t tmp;
    uint8_t count;
    uint8_t i;
    uint8_t j;

    ultrasonic_samples[ultrasonic_sample_index] = sample_mm;
    ultrasonic_sample_index++;
    if (ultrasonic_sample_index >= ULTRASONIC_FILTER_SIZE)
    {
        ultrasonic_sample_index = 0U;
    }
    if (ultrasonic_sample_count < ULTRASONIC_FILTER_SIZE)
    {
        ultrasonic_sample_count++;
    }

    count = ultrasonic_sample_count;
    for (i = 0U; i < count; i++)
    {
        sorted[i] = ultrasonic_samples[i];
    }
    for (i = 0U; i < count; i++)
    {
        for (j = (uint8_t)(i + 1U); j < count; j++)
        {
            if (sorted[j] < sorted[i])
            {
                tmp = sorted[i];
                sorted[i] = sorted[j];
                sorted[j] = tmp;
            }
        }
    }

    return sorted[count / 2U];
}

void Ultrasonic_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOF_CLK_ENABLE();
    __HAL_RCC_SYSCFG_CLK_ENABLE();

    GPIO_InitStruct.Pin = ULTRASONIC_TRIG_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(ULTRASONIC_TRIG_GPIO_PORT, &GPIO_InitStruct);
    HAL_GPIO_WritePin(ULTRASONIC_TRIG_GPIO_PORT, ULTRASONIC_TRIG_PIN, GPIO_PIN_RESET);

    GPIO_InitStruct.Pin = ULTRASONIC_ECHO_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(ULTRASONIC_ECHO_GPIO_PORT, &GPIO_InitStruct);

    HAL_NVIC_SetPriority(EXTI9_5_IRQn, 2U, 0U);
    HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
}

uint8_t Ultrasonic_ReadOnce(uint32_t *echo_us, uint32_t *distance_mm)
{
    if (ultrasonic_data_valid == 0U)
    {
        return 0U;
    }
    if (echo_us != NULL) *echo_us = ultrasonic_echo_us;
    if (distance_mm != NULL) *distance_mm = ultrasonic_distance_mm;
    return 1U;
}

void Ultrasonic_ProcessEcho(uint16_t gpio_pin)
{
    uint32_t now_cycle;
    uint32_t since_trig_us;
    uint32_t pulse_us;
    uint32_t raw_mm;
    uint32_t ticks_per_us;

    if (gpio_pin != ULTRASONIC_ECHO_PIN)
    {
        return;
    }

    ticks_per_us = Ultrasonic_TicksPerUs();
    now_cycle = DWT->CYCCNT;

    if (HAL_GPIO_ReadPin(ULTRASONIC_ECHO_GPIO_PORT, ULTRASONIC_ECHO_PIN) == GPIO_PIN_SET)
    {
        ultrasonic_echo_level = 1U;
        if (ultrasonic_wait_echo != 0U)
        {
            since_trig_us = (now_cycle - ultrasonic_last_trig_cycle) / ticks_per_us;
            if (since_trig_us < ULTRASONIC_BLANK_US)
            {
                return;
            }
            ultrasonic_rise_cycle = now_cycle;
            ultrasonic_seen_rise = 1U;
        }
        return;
    }

    ultrasonic_echo_level = 0U;
    if (ultrasonic_wait_echo == 0U || ultrasonic_seen_rise == 0U)
    {
        return;
    }

    pulse_us = (now_cycle - ultrasonic_rise_cycle) / ticks_per_us;
    if (pulse_us >= ULTRASONIC_MIN_ECHO_US && pulse_us <= ULTRASONIC_TIMEOUT_US)
    {
        raw_mm = (pulse_us * 343U) / 2000U;
        ultrasonic_raw_distance_mm = raw_mm;
        ultrasonic_echo_us = pulse_us;
        ultrasonic_distance_mm = Ultrasonic_FilterPush(raw_mm);
        ultrasonic_read_count++;
        ultrasonic_last_valid_ms = HAL_GetTick();
        ultrasonic_data_valid = 1U;
    }
    else
    {
        ultrasonic_reject_us = pulse_us;
        ultrasonic_reject_count++;
    }

    ultrasonic_wait_echo = 0U;
    ultrasonic_seen_rise = 0U;
}

void Ultrasonic_UART2_Report(void)
{
    /* USART2 TX is reserved for forwarding USART1 RX data. */
}

void Ultrasonic_Service(void)
{
    uint32_t now_ms = HAL_GetTick();
    uint32_t elapsed_us;

    ultrasonic_echo_level =
        (HAL_GPIO_ReadPin(ULTRASONIC_ECHO_GPIO_PORT, ULTRASONIC_ECHO_PIN) == GPIO_PIN_SET) ? 1U : 0U;

    if (ultrasonic_wait_echo != 0U)
    {
        elapsed_us = (DWT->CYCCNT - ultrasonic_last_trig_cycle) / Ultrasonic_TicksPerUs();
        if (elapsed_us > ULTRASONIC_TIMEOUT_US)
        {
            ultrasonic_timeout_count++;
            ultrasonic_wait_echo = 0U;
            ultrasonic_seen_rise = 0U;
        }
        Ultrasonic_UART2_Report();
        return;
    }

    if ((now_ms - ultrasonic_last_valid_ms) > ULTRASONIC_VALID_MS)
    {
        ultrasonic_data_valid = 0U;
    }

    if ((now_ms - ultrasonic_last_trig_ms) >= ULTRASONIC_PERIOD_MS)
    {
        ultrasonic_last_trig_ms = now_ms;
        ultrasonic_last_trig_cycle = DWT->CYCCNT;
        ultrasonic_wait_echo = 1U;
        ultrasonic_seen_rise = 0U;

        HAL_GPIO_WritePin(ULTRASONIC_TRIG_GPIO_PORT, ULTRASONIC_TRIG_PIN, GPIO_PIN_RESET);
        delay_us(2U);
        HAL_GPIO_WritePin(ULTRASONIC_TRIG_GPIO_PORT, ULTRASONIC_TRIG_PIN, GPIO_PIN_SET);
        delay_us(10U);
        HAL_GPIO_WritePin(ULTRASONIC_TRIG_GPIO_PORT, ULTRASONIC_TRIG_PIN, GPIO_PIN_RESET);
        ultrasonic_trig_count++;
    }

    Ultrasonic_UART2_Report();
}
