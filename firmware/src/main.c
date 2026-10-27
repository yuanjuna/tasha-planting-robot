#include "app.h"

#if defined(APP_USE_FREERTOS) && (APP_USE_FREERTOS != 0)
#include "FreeRTOS.h"
#include "task.h"

extern void vPortSVCHandler(void);
extern void xPortPendSVHandler(void);
extern void xPortSysTickHandler(void);
#endif

static void Pump_EarlyOff_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOC_CLK_ENABLE();

    HAL_GPIO_WritePin(PUMP_RELAY_GPIO_PORT, PUMP_RELAY_PIN, PUMP_RELAY_OFF_LEVEL);

    GPIO_InitStruct.Pin = PUMP_RELAY_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(PUMP_RELAY_GPIO_PORT, &GPIO_InitStruct);

    HAL_GPIO_WritePin(PUMP_RELAY_GPIO_PORT, PUMP_RELAY_PIN, PUMP_RELAY_OFF_LEVEL);
}

#if defined(LINEAR_ACTUATOR_TEST_ONLY) && (LINEAR_ACTUATOR_TEST_ONLY != 0)
#define LINEAR_ACT_EN_GPIO_PORT   GPIOA
#define LINEAR_ACT_EN_PIN         GPIO_PIN_0
#define LINEAR_ACT_PWM1_GPIO_PORT GPIOA
#define LINEAR_ACT_PWM1_PIN       GPIO_PIN_1
#define LINEAR_ACT_PWM2_GPIO_PORT GPIOG
#define LINEAR_ACT_PWM2_PIN       GPIO_PIN_2

static void LinearActuator_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOG_CLK_ENABLE();

    HAL_GPIO_WritePin(LINEAR_ACT_EN_GPIO_PORT, LINEAR_ACT_EN_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LINEAR_ACT_PWM1_GPIO_PORT, LINEAR_ACT_PWM1_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LINEAR_ACT_PWM2_GPIO_PORT, LINEAR_ACT_PWM2_PIN, GPIO_PIN_RESET);

    GPIO_InitStruct.Pin = LINEAR_ACT_EN_PIN | LINEAR_ACT_PWM1_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = LINEAR_ACT_PWM2_PIN;
    HAL_GPIO_Init(LINEAR_ACT_PWM2_GPIO_PORT, &GPIO_InitStruct);
}

void Actuator_Extend(void)
{
    HAL_GPIO_WritePin(LINEAR_ACT_EN_GPIO_PORT, LINEAR_ACT_EN_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(LINEAR_ACT_PWM1_GPIO_PORT, LINEAR_ACT_PWM1_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(LINEAR_ACT_PWM2_GPIO_PORT, LINEAR_ACT_PWM2_PIN, GPIO_PIN_RESET);
}

void Actuator_Retract(void)
{
    HAL_GPIO_WritePin(LINEAR_ACT_EN_GPIO_PORT, LINEAR_ACT_EN_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(LINEAR_ACT_PWM1_GPIO_PORT, LINEAR_ACT_PWM1_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LINEAR_ACT_PWM2_GPIO_PORT, LINEAR_ACT_PWM2_PIN, GPIO_PIN_SET);
}

void Actuator_Stop(void)
{
    HAL_GPIO_WritePin(LINEAR_ACT_PWM1_GPIO_PORT, LINEAR_ACT_PWM1_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LINEAR_ACT_PWM2_GPIO_PORT, LINEAR_ACT_PWM2_PIN, GPIO_PIN_RESET);
}
#endif

#if defined(LINEAR_ACTUATOR_PG4_PG5_TEST_ONLY) && (LINEAR_ACTUATOR_PG4_PG5_TEST_ONLY != 0)
#define LINEAR_PG45_GPIO_PORT GPIOG
#define LINEAR_PG45_EXTEND_PIN GPIO_PIN_4
#define LINEAR_PG45_RETRACT_PIN GPIO_PIN_5
#define LINEAR_PG45_RUN_MS 5000U
#define LINEAR_PG45_STOP_MS 2000U

static void LinearPG45_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOG_CLK_ENABLE();

    HAL_GPIO_WritePin(LINEAR_PG45_GPIO_PORT,
                      LINEAR_PG45_EXTEND_PIN | LINEAR_PG45_RETRACT_PIN,
                      GPIO_PIN_RESET);

    GPIO_InitStruct.Pin = LINEAR_PG45_EXTEND_PIN | LINEAR_PG45_RETRACT_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(LINEAR_PG45_GPIO_PORT, &GPIO_InitStruct);
}

static void LinearPG45_Extend(void)
{
    HAL_GPIO_WritePin(LINEAR_PG45_GPIO_PORT, LINEAR_PG45_EXTEND_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(LINEAR_PG45_GPIO_PORT, LINEAR_PG45_RETRACT_PIN, GPIO_PIN_RESET);
}

static void LinearPG45_Retract(void)
{
    HAL_GPIO_WritePin(LINEAR_PG45_GPIO_PORT, LINEAR_PG45_EXTEND_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LINEAR_PG45_GPIO_PORT, LINEAR_PG45_RETRACT_PIN, GPIO_PIN_SET);
}

static void LinearPG45_Stop(void)
{
    HAL_GPIO_WritePin(LINEAR_PG45_GPIO_PORT,
                      LINEAR_PG45_EXTEND_PIN | LINEAR_PG45_RETRACT_PIN,
                      GPIO_PIN_RESET);
}
#endif

#if defined(PB8_TOGGLE_TEST_ONLY) && (PB8_TOGGLE_TEST_ONLY != 0)
static void PB8_ToggleTest_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOB_CLK_ENABLE();

    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_RESET);

    GPIO_InitStruct.Pin = GPIO_PIN_8;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}
#endif

#if defined(SERVO51_PB0_DIAG_TEST_ONLY) && (SERVO51_PB0_DIAG_TEST_ONLY != 0)
#define SERVO51_DIAG_START_MS        3000U
#define SERVO51_DIAG_PULSES_PER_SLOT 125U
#define SERVO51_DIAG_LOW_US          1000U
#define SERVO51_DIAG_RELEASE_US      4000U
#define SERVO51_DIAG_SLOT_STOP_MS    1000U

static void Servo51PB0Diag_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOF_CLK_ENABLE();

    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOF, GPIO_PIN_9 | GPIO_PIN_10, GPIO_PIN_SET);

    GPIO_InitStruct.Pin = GPIO_PIN_0;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_9 | GPIO_PIN_10;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);
}

static void Servo51PB0Diag_BlinkStep(uint8_t count)
{
    uint8_t i;

    for (i = 0U; i < count; i++)
    {
        HAL_GPIO_WritePin(GPIOF, GPIO_PIN_9, GPIO_PIN_RESET);
        HAL_Delay(180U);
        HAL_GPIO_WritePin(GPIOF, GPIO_PIN_9, GPIO_PIN_SET);
        HAL_Delay(180U);
    }
    HAL_Delay(500U);
}

static void Servo51PB0Diag_OutputStepPulse(void)
{
    uint32_t pulse;

    Servo51PB0Diag_BlinkStep(1U);
    HAL_GPIO_WritePin(GPIOF, GPIO_PIN_10, GPIO_PIN_RESET);

    for (pulse = 0U; pulse < SERVO51_DIAG_PULSES_PER_SLOT; pulse++)
    {
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);
        delay_us(SERVO51_DIAG_LOW_US);
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);
        delay_us(SERVO51_DIAG_RELEASE_US);
    }

    HAL_GPIO_WritePin(GPIOF, GPIO_PIN_10, GPIO_PIN_SET);
    HAL_Delay(SERVO51_DIAG_SLOT_STOP_MS);
}
#endif

#if defined(CAR_PIN_DIAG_TEST_ONLY) && (CAR_PIN_DIAG_TEST_ONLY != 0)
#define CAR_PIN_DIAG_ACTIVE_MS 3000U
#define CAR_PIN_DIAG_STOP_MS   2000U
#define CAR_PIN_DIAG_START_MS  3000U

static void CarPinDiag_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOF_CLK_ENABLE();

    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6 | GPIO_PIN_7, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1 | GPIO_PIN_12, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOF, GPIO_PIN_9 | GPIO_PIN_10, GPIO_PIN_SET);

    GPIO_InitStruct.Pin = GPIO_PIN_6 | GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_1 | GPIO_PIN_12;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_9 | GPIO_PIN_10;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);
}

static void CarPinDiag_AllStop(void)
{
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6 | GPIO_PIN_7, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1 | GPIO_PIN_12, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOF, GPIO_PIN_9 | GPIO_PIN_10, GPIO_PIN_SET);
}

static void CarPinDiag_BlinkStep(uint8_t count)
{
    uint8_t i;

    for (i = 0U; i < count; i++)
    {
        HAL_GPIO_WritePin(GPIOF, GPIO_PIN_9, GPIO_PIN_RESET);
        HAL_Delay(180U);
        HAL_GPIO_WritePin(GPIOF, GPIO_PIN_9, GPIO_PIN_SET);
        HAL_Delay(180U);
    }
    HAL_Delay(500U);
}

static void CarPinDiag_TestPin(GPIO_TypeDef *port, uint16_t pin, uint8_t step)
{
    CarPinDiag_AllStop();
    CarPinDiag_BlinkStep(step);
    HAL_GPIO_WritePin(port, pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOF, GPIO_PIN_10, GPIO_PIN_RESET);
    HAL_Delay(CAR_PIN_DIAG_ACTIVE_MS);
    HAL_GPIO_WritePin(GPIOF, GPIO_PIN_10, GPIO_PIN_SET);
    CarPinDiag_AllStop();
    HAL_Delay(CAR_PIN_DIAG_STOP_MS);
}
#endif

#if defined(CAR_LEFT_DIAG_TEST_ONLY) && (CAR_LEFT_DIAG_TEST_ONLY != 0)
#define CAR_LEFT_DIAG_ACTIVE_MS 5000U
#define CAR_LEFT_DIAG_STOP_MS   2000U
#define CAR_LEFT_DIAG_START_MS  3000U

static void CarLeftDiag_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOF_CLK_ENABLE();

    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOF, GPIO_PIN_9 | GPIO_PIN_10, GPIO_PIN_SET);

    GPIO_InitStruct.Pin = GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_1;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_9 | GPIO_PIN_10;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);
}

static void CarLeftDiag_Set(GPIO_PinState pa7, GPIO_PinState pb1)
{
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, pa7);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, pb1);
}

static void CarLeftDiag_StopHigh(void)
{
    CarLeftDiag_Set(GPIO_PIN_SET, GPIO_PIN_SET);
}

static void CarLeftDiag_BlinkStep(uint8_t count)
{
    uint8_t i;

    for (i = 0U; i < count; i++)
    {
        HAL_GPIO_WritePin(GPIOF, GPIO_PIN_9, GPIO_PIN_RESET);
        HAL_Delay(180U);
        HAL_GPIO_WritePin(GPIOF, GPIO_PIN_9, GPIO_PIN_SET);
        HAL_Delay(180U);
    }
    HAL_Delay(500U);
}

static void CarLeftDiag_Test(GPIO_PinState pa7, GPIO_PinState pb1, uint8_t step)
{
    CarLeftDiag_StopHigh();
    HAL_Delay(CAR_LEFT_DIAG_STOP_MS);
    CarLeftDiag_BlinkStep(step);
    CarLeftDiag_Set(pa7, pb1);
    HAL_GPIO_WritePin(GPIOF, GPIO_PIN_10, GPIO_PIN_RESET);
    HAL_Delay(CAR_LEFT_DIAG_ACTIVE_MS);
    HAL_GPIO_WritePin(GPIOF, GPIO_PIN_10, GPIO_PIN_SET);
    CarLeftDiag_StopHigh();
    HAL_Delay(CAR_LEFT_DIAG_STOP_MS);
}
#endif

#if defined(CAR_TURN_TEST_ONLY) && (CAR_TURN_TEST_ONLY != 0)
#define CAR_TURN_TEST_ACTIVE_MS 4000U
#define CAR_TURN_TEST_STOP_MS   2000U
#define CAR_TURN_TEST_START_MS  3000U

static void CarTurnTest_Stop(void)
{
    Car_Motor_Stop();
    HAL_GPIO_WritePin(GPIOF, GPIO_PIN_9 | GPIO_PIN_10, GPIO_PIN_SET);
}

static void CarTurnTest_Right(void)
{
    Car_Motor_SetLeft(1);
    Car_Motor_SetRight(-1);
    HAL_GPIO_WritePin(GPIOF, GPIO_PIN_9, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOF, GPIO_PIN_10, GPIO_PIN_SET);
}

static void CarTurnTest_Left(void)
{
    Car_Motor_SetLeft(-1);
    Car_Motor_SetRight(1);
    HAL_GPIO_WritePin(GPIOF, GPIO_PIN_9, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOF, GPIO_PIN_10, GPIO_PIN_RESET);
}
#endif

#if defined(UART1_ECHO_ONLY) && (UART1_ECHO_ONLY != 0)
#define UART1_DIAG_RING_SIZE 512U

static uint8_t uart1_diag_rx_byte;
static volatile uint8_t uart1_diag_ring[UART1_DIAG_RING_SIZE];
static volatile uint16_t uart1_diag_head;
static volatile uint16_t uart1_diag_tail;
static volatile uint32_t uart1_diag_rx_count;
static volatile uint32_t uart1_diag_drop_count;
static volatile uint32_t uart1_diag_error_count;
static volatile uint32_t uart1_diag_last_error;

static void UART1_DiagStartReceive(void)
{
    if (HAL_UART_Receive_IT(&huart1, &uart1_diag_rx_byte, 1U) != HAL_OK)
    {
        uart1_diag_error_count++;
    }
}

static void UART1_DiagPushByte(uint8_t data)
{
    uint16_t next_head = uart1_diag_head + 1U;

    if (next_head >= UART1_DIAG_RING_SIZE)
    {
        next_head = 0U;
    }

    if (next_head == uart1_diag_tail)
    {
        uart1_diag_drop_count++;
        return;
    }

    uart1_diag_ring[uart1_diag_head] = data;
    uart1_diag_head = next_head;
    uart1_diag_rx_count++;
}

static uint8_t UART1_DiagPopByte(uint8_t *data)
{
    if (uart1_diag_tail == uart1_diag_head)
    {
        return 0U;
    }

    *data = uart1_diag_ring[uart1_diag_tail];
    uart1_diag_tail++;
    if (uart1_diag_tail >= UART1_DIAG_RING_SIZE)
    {
        uart1_diag_tail = 0U;
    }
    return 1U;
}

static void UART_DiagPrint(const char *text)
{
    HAL_UART_Transmit(&huart2, (uint8_t *)text, (uint16_t)strlen(text), 100U);
}
#endif

#if defined(MOBILE_TEST_ONLY) && (MOBILE_TEST_ONLY != 0)
static void Mobile_TestPrint(const char *text)
{
    HAL_UART_Transmit(&huart2, (uint8_t *)text, (uint16_t)strlen(text), 100U);
}

static void Mobile_TB6600Pulse(uint32_t pulse_us, uint32_t period_us)
{
    HAL_GPIO_WritePin(MOBILE_STEP_GPIO_PORT, MOBILE_STEP_PIN, GPIO_PIN_RESET);
    delay_us(pulse_us);
    HAL_GPIO_WritePin(MOBILE_STEP_GPIO_PORT, MOBILE_STEP_PIN, GPIO_PIN_SET);
    delay_us(period_us - pulse_us);
}

static void Mobile_TestRun(uint8_t dir, uint32_t run_ms, uint32_t freq_hz)
{
    uint32_t start_tick;
    uint32_t period_us;

    if (freq_hz == 0U)
    {
        return;
    }

    period_us = 1000000U / freq_hz;
    HAL_GPIO_WritePin(MOBILE_STEP_GPIO_PORT, MOBILE_STEP_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(MOBILE_DIR_GPIO_PORT, MOBILE_DIR_PIN,
                      (dir == FORWARD) ? GPIO_PIN_RESET : GPIO_PIN_SET);
    HAL_Delay(500U);
    start_tick = HAL_GetTick();
    while ((HAL_GetTick() - start_tick) < run_ms)
    {
        Mobile_TB6600Pulse(100U, period_us);
    }
    HAL_GPIO_WritePin(MOBILE_STEP_GPIO_PORT, MOBILE_STEP_PIN, GPIO_PIN_SET);
    HAL_Delay(500U);
}

static void Mobile_TestRunDrillEvent(uint8_t dir,
                                     uint32_t run_ms,
                                     uint32_t freq_hz,
                                     uint32_t drill_event_ms,
                                     uint8_t drill_on)
{
    uint32_t start_tick;
    uint32_t period_us;
    uint8_t event_done = 0U;

    if (freq_hz == 0U)
    {
        return;
    }

    period_us = 1000000U / freq_hz;
    HAL_GPIO_WritePin(MOBILE_STEP_GPIO_PORT, MOBILE_STEP_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(MOBILE_DIR_GPIO_PORT, MOBILE_DIR_PIN,
                      (dir == FORWARD) ? GPIO_PIN_RESET : GPIO_PIN_SET);
    HAL_Delay(500U);
    start_tick = HAL_GetTick();
    while ((HAL_GetTick() - start_tick) < run_ms)
    {
        if (event_done == 0U && (HAL_GetTick() - start_tick) >= drill_event_ms)
        {
            if (drill_on != 0U)
            {
                Pump_On();
            }
            else
            {
                Pump_Off();
            }
            event_done = 1U;
        }
        Mobile_TB6600Pulse(100U, period_us);
    }
    HAL_GPIO_WritePin(MOBILE_STEP_GPIO_PORT, MOBILE_STEP_PIN, GPIO_PIN_SET);
    HAL_Delay(500U);
}
#endif

#if defined(CAR_GPIO_MANUAL_TEST_ONLY) && (CAR_GPIO_MANUAL_TEST_ONLY != 0)
#define MANUAL_SERVO_STOP_US           1500U
#define MANUAL_SERVO_RUN_US            2500U
#define MANUAL_SERVO_STEP_MS           66U
#define MANUAL_SERVO_STOP_BETWEEN_MS   1000U
#define MANUAL_SERVO_STEP_COUNT        13U
#define MANUAL_SERVO_ROUND_STOP_MS     2000U
#define MANUAL_SERVO_PWM_PERIOD_US     20000U
#define MANUAL_SERVO13_PUL_GPIO_PORT GPIOB
#define MANUAL_SERVO13_PUL_PIN       GPIO_PIN_0
#define MANUAL_SERVO13_START_DELAY_MS 5000U
#define MANUAL_SERVO13_SLOT_COUNT     13U
#define MANUAL_SERVO13_PULSES_PER_SLOT 125U
#define MANUAL_SERVO13_SEGMENT_STOP_MS 1000U
#define MANUAL_SERVO13_REV_STOP_MS     3000U
#define MANUAL_SERVO13_PULSE_LOW_US    1000U
#define MANUAL_SERVO13_PULSE_RELEASE_US 4000U
#define MANUAL_MOBILE_RUN_MS           8000U
#define MANUAL_MOBILE_STOP_MS          2000U
#define MANUAL_MOBILE_FREQ_HZ          MOBILE_TESTED_FREQ_HZ
#define MANUAL_MOBILE_STEP_HIGH_US     100U
#define MANUAL_MOBILE_STEP_PERIOD_US   (1000000U / MANUAL_MOBILE_FREQ_HZ)
#define MANUAL_COVER_A_IN1_GPIO_PORT   GPIOG
#define MANUAL_COVER_A_IN1_PIN         GPIO_PIN_2
#define MANUAL_COVER_A_IN2_GPIO_PORT   GPIOG
#define MANUAL_COVER_A_IN2_PIN         GPIO_PIN_3
#define MANUAL_COVER_B_IN1_GPIO_PORT   GPIOG
#define MANUAL_COVER_B_IN1_PIN         GPIO_PIN_4
#define MANUAL_COVER_B_IN2_GPIO_PORT   GPIOG
#define MANUAL_COVER_B_IN2_PIN         GPIO_PIN_5
#define MANUAL_COVER_FORWARD_MS        10000U
#define MANUAL_COVER_REVERSE_MS        10000U
#define MANUAL_COVER_STOP_MS           1000U
/* L298N water pump driver:
 * PE5 -> IN1, IN2 -> GND, ENA jumper installed.
 * Keep the original polarity: PE5 high runs the pump, PE5 low stops it.
 */
#define MANUAL_WATER_PUMP_GPIO_PORT    GPIOE
#define MANUAL_WATER_PUMP_PIN          GPIO_PIN_5
#define MANUAL_WATER_PUMP_ON_LEVEL     GPIO_PIN_SET
#define MANUAL_WATER_PUMP_OFF_LEVEL    GPIO_PIN_RESET
#define MANUAL_WATER_PUMP_ON_MS        10000U
#define MANUAL_WATER_PUMP_OFF_MS       2000U
#define SNAKE_TOTAL_SEEDLINGS          12U
#define SNAKE_DRILL_DOWN_MS            8000U
#define SNAKE_DRILL_ON_AFTER_DOWN_MS   5000U
#define SNAKE_DRILL_BOTTOM_HOLD_MS     2000U
#define SNAKE_DRILL_UP_MS              8000U
#define SNAKE_DRILL_OFF_AFTER_UP_MS    4000U
#define SNAKE_CAR_AFTER_DRILL_MS       2000U
#define SNAKE_CAR_NEXT_POINT_MS        2000U
#define SNAKE_CAR_ROW_MOVE_MS          2000U
#define SNAKE_CAR_ROW_TURN_FIRST_MS    4000U
#define SNAKE_CAR_ROW_TURN_SECOND_MS   5000U
#define SNAKE_COVER_EXTEND_MS          10000U
#define SNAKE_COVER_STOP_MS            1000U
#define SNAKE_COVER_RETRACT_MS         10000U
#define SNAKE_WATER_MS                 3000U
#define SNAKE_MODE_MANUAL_MAX_US       1300U
#define SNAKE_MODE_AUTO_MIN_US         1700U
#define MANUAL_MODE_IDLE_MAX_US        1300U
#define MANUAL_MODE_WORK_MAX_US        1700U
#define MANUAL_RANGE_STOP_MAX_US       1300U
#define MANUAL_RANGE_EXTEND_MAX_US     1700U
#define MANUAL_SOLAR_EXTEND_MAX_US     1450U
#define MANUAL_SOLAR_STOP_MAX_US       1550U
#define MANUAL_DRILL_DOWN_MIN_US       1250U
#define MANUAL_DRILL_UP_MIN_US         1750U
#define MANUAL_COVER_RESET_MAX_US      1250U
#define MANUAL_COVER_DOWN_MIN_US       1250U
#define MANUAL_COVER_UP_MIN_US         1750U
#define MANUAL_SEED_PUMP_LOW_MAX_US    1250U
#define MANUAL_SEED_PUMP_CENTER_MIN_US 1300U
#define MANUAL_SEED_PUMP_CENTER_MAX_US 1700U
#define MANUAL_SEED_PUMP_HIGH_MIN_US   1750U
#define MANUAL_AUTO_SPEED_DEFAULT_PERCENT 20U
#define MANUAL_AUTO_SPEED_MIN_PERCENT     10U
#define MANUAL_AUTO_SPEED_MAX_PERCENT     90U
#define MANUAL_AUTO_SPEED_STEP_PERCENT    1U
#define MANUAL_AUTO_SPEED_DEC_MAX_US      1400U
#define MANUAL_AUTO_SPEED_CENTER_MIN_US   1450U
#define MANUAL_AUTO_SPEED_CENTER_MAX_US   1550U
#define MANUAL_AUTO_SPEED_INC_MIN_US      1600U
#define MANUAL_SEED_PUMP_INPUT_GPIO_PORT GPIOF
#define MANUAL_SEED_PUMP_INPUT_PIN       GPIO_PIN_11

static volatile uint8_t manual_mobile_stepper_active = 0U;
static volatile uint8_t manual_mobile_stepper_low = 0U;
static TIM_HandleTypeDef manual_servo13_tim;
static TIM_HandleTypeDef manual_seed_pump_tim;
static CarRc_Channel_t manual_seed_pump_rc;
static volatile uint32_t manual_mode_rise_cycles = 0U;
static volatile uint8_t manual_mode_wait_fall = 0U;
static volatile uint32_t manual_seed_pump_rise_cycles = 0U;
static volatile uint8_t manual_seed_pump_wait_fall = 0U;
static volatile uint8_t manual_servo13_active = 0U;
static volatile uint8_t manual_servo13_low = 0U;
static volatile uint32_t manual_servo13_pulse_count = 0U;
static uint8_t manual_snake_abort_requested = 0U;
/* 1..12 = next seedling to run; 0 = all 12 finished for this cycle. */
static uint8_t manual_snake_resume_seed = 1U;
typedef enum
{
    MANUAL_SNAKE_PHASE_DRILL = 0,
    MANUAL_SNAKE_PHASE_AFTER_DRILL_CAR,
    MANUAL_SNAKE_PHASE_SEED,
    MANUAL_SNAKE_PHASE_COVER,
    MANUAL_SNAKE_PHASE_WATER,
    MANUAL_SNAKE_PHASE_NEXT_CAR,
    MANUAL_SNAKE_PHASE_DONE
} ManualSnakePhase_t;
static ManualSnakePhase_t manual_snake_resume_phase = MANUAL_SNAKE_PHASE_DRILL;
static uint8_t manual_auto_speed_adjust_armed = 1U;
static uint32_t manual_snake_last_imu_ms = 0U;
static uint32_t manual_snake_last_lcd_ms = 0U;
static uint32_t manual_snake_last_scan_ms = 0U;

static void ManualCar_AllStop(void);

typedef enum
{
    MANUAL_CONTROL_MODE_IDLE = 0,
    MANUAL_CONTROL_MODE_SEEDLING,
    MANUAL_CONTROL_MODE_AUTO_SPEED
} ManualControlMode_t;

static void ManualCar_Servo13PB0_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOB_CLK_ENABLE();

    GPIO_InitStruct.Pin = MANUAL_SERVO13_PUL_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(MANUAL_SERVO13_PUL_GPIO_PORT, &GPIO_InitStruct);

    HAL_GPIO_WritePin(MANUAL_SERVO13_PUL_GPIO_PORT,
                      MANUAL_SERVO13_PUL_PIN,
                      GPIO_PIN_SET);
}

static void ManualCar_SeedPumpPF11_InputCapture_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOF_CLK_ENABLE();

    GPIO_InitStruct.Pin = MANUAL_SEED_PUMP_INPUT_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(MANUAL_SEED_PUMP_INPUT_GPIO_PORT, &GPIO_InitStruct);

    Car_RC_ChannelInit(&manual_seed_pump_rc);
    manual_seed_pump_rise_cycles = 0U;
    manual_seed_pump_wait_fall = 0U;

    HAL_NVIC_SetPriority(EXTI15_10_IRQn, 1U, 0U);
    HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
}

static void ManualCar_Servo13PB0Update(void)
{
    enum
    {
        SERVO13_PHASE_START_DELAY = 0,
        SERVO13_PHASE_RUN,
        SERVO13_PHASE_SEGMENT_STOP,
        SERVO13_PHASE_REV_STOP
    };
    static uint8_t phase = SERVO13_PHASE_START_DELAY;
    static uint32_t phase_start_ms = 0U;
    static uint32_t slot = 0U;
    uint32_t now_ms = HAL_GetTick();

    if (phase_start_ms == 0U)
    {
        phase_start_ms = now_ms;
        manual_servo13_active = 0U;
        HAL_GPIO_WritePin(MANUAL_SERVO13_PUL_GPIO_PORT,
                          MANUAL_SERVO13_PUL_PIN,
                          GPIO_PIN_SET);
        return;
    }

    if (phase == SERVO13_PHASE_START_DELAY)
    {
        if ((now_ms - phase_start_ms) >= MANUAL_SERVO13_START_DELAY_MS)
        {
            slot = 0U;
            manual_servo13_pulse_count = 0U;
            manual_servo13_low = 0U;
            manual_servo13_active = 1U;
            phase = SERVO13_PHASE_RUN;
        }
        return;
    }

    if (phase == SERVO13_PHASE_RUN)
    {
        if (manual_servo13_pulse_count >= MANUAL_SERVO13_PULSES_PER_SLOT)
        {
            manual_servo13_active = 0U;
            manual_servo13_low = 0U;
            HAL_GPIO_WritePin(MANUAL_SERVO13_PUL_GPIO_PORT,
                              MANUAL_SERVO13_PUL_PIN,
                              GPIO_PIN_SET);
            slot++;
            phase_start_ms = now_ms;
            if (slot >= MANUAL_SERVO13_SLOT_COUNT)
            {
                phase = SERVO13_PHASE_REV_STOP;
            }
            else
            {
                phase = SERVO13_PHASE_SEGMENT_STOP;
            }
        }
        return;
    }

    if (phase == SERVO13_PHASE_SEGMENT_STOP)
    {
        if ((now_ms - phase_start_ms) >= MANUAL_SERVO13_SEGMENT_STOP_MS)
        {
            manual_servo13_pulse_count = 0U;
            manual_servo13_low = 0U;
            manual_servo13_active = 1U;
            phase = SERVO13_PHASE_RUN;
        }
        return;
    }

    if (phase == SERVO13_PHASE_REV_STOP)
    {
        if ((now_ms - phase_start_ms) >= MANUAL_SERVO13_REV_STOP_MS)
        {
            slot = 0U;
            manual_servo13_pulse_count = 0U;
            manual_servo13_low = 0U;
            manual_servo13_active = 1U;
            phase = SERVO13_PHASE_RUN;
        }
    }
}

static void ManualCar_MobileStepper_Init(void)
{
    TIM_ClockConfigTypeDef sClockSourceConfig = {0};
    TIM_MasterConfigTypeDef sMasterConfig = {0};

    Mobile_GPIO_Init();
    HAL_GPIO_WritePin(MOBILE_STEP_GPIO_PORT, MOBILE_STEP_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(MOBILE_DIR_GPIO_PORT, MOBILE_DIR_PIN, GPIO_PIN_RESET);

    __HAL_RCC_TIM3_CLK_ENABLE();

    htim3.Instance = TIM3;
    htim3.Init.Prescaler = 83U;
    htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim3.Init.Period = MANUAL_MOBILE_STEP_HIGH_US - 1U;
    htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    if (HAL_TIM_Base_Init(&htim3) != HAL_OK) Error_Handler();

    sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
    if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK) Error_Handler();

    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK) Error_Handler();

    HAL_NVIC_SetPriority(TIM3_IRQn, 0U, 0U);
    HAL_NVIC_EnableIRQ(TIM3_IRQn);
    if (HAL_TIM_Base_Start_IT(&htim3) != HAL_OK) Error_Handler();
}

static void ManualCar_MobileStepperUpdate(void)
{
    enum
    {
        MOBILE_PHASE_FORWARD = 0,
        MOBILE_PHASE_FORWARD_STOP,
        MOBILE_PHASE_REVERSE,
        MOBILE_PHASE_REVERSE_STOP
    };
    static uint8_t phase = MOBILE_PHASE_FORWARD;
    static uint32_t phase_start_ms = 0U;
    uint32_t now_ms = HAL_GetTick();

    if (phase_start_ms == 0U)
    {
        phase_start_ms = now_ms;
        HAL_GPIO_WritePin(MOBILE_DIR_GPIO_PORT, MOBILE_DIR_PIN, GPIO_PIN_RESET);
        manual_mobile_stepper_active = 1U;
        return;
    }

    if (phase == MOBILE_PHASE_FORWARD_STOP || phase == MOBILE_PHASE_REVERSE_STOP)
    {
        manual_mobile_stepper_active = 0U;
        if ((now_ms - phase_start_ms) >= MANUAL_MOBILE_STOP_MS)
        {
            if (phase == MOBILE_PHASE_FORWARD_STOP)
            {
                phase = MOBILE_PHASE_REVERSE;
                HAL_GPIO_WritePin(MOBILE_DIR_GPIO_PORT, MOBILE_DIR_PIN, GPIO_PIN_SET);
            }
            else
            {
                phase = MOBILE_PHASE_FORWARD;
                HAL_GPIO_WritePin(MOBILE_DIR_GPIO_PORT, MOBILE_DIR_PIN, GPIO_PIN_RESET);
            }
            phase_start_ms = now_ms;
            manual_mobile_stepper_low = 0U;
            manual_mobile_stepper_active = 1U;
        }
        return;
    }

    if ((now_ms - phase_start_ms) >= MANUAL_MOBILE_RUN_MS)
    {
        phase = (phase == MOBILE_PHASE_FORWARD) ? MOBILE_PHASE_FORWARD_STOP : MOBILE_PHASE_REVERSE_STOP;
        phase_start_ms = now_ms;
        manual_mobile_stepper_active = 0U;
        manual_mobile_stepper_low = 0U;
        HAL_GPIO_WritePin(MOBILE_STEP_GPIO_PORT, MOBILE_STEP_PIN, GPIO_PIN_SET);
        return;
    }
}

static void ManualCar_Cover4Pin_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOG_CLK_ENABLE();

    HAL_GPIO_WritePin(GPIOG,
                      MANUAL_COVER_A_IN1_PIN | MANUAL_COVER_A_IN2_PIN |
                      MANUAL_COVER_B_IN1_PIN | MANUAL_COVER_B_IN2_PIN,
                      GPIO_PIN_RESET);

    GPIO_InitStruct.Pin = MANUAL_COVER_A_IN1_PIN | MANUAL_COVER_A_IN2_PIN |
                          MANUAL_COVER_B_IN1_PIN | MANUAL_COVER_B_IN2_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);
}

static void ManualCar_Cover4PinStop(void)
{
    HAL_GPIO_WritePin(GPIOG,
                      MANUAL_COVER_A_IN1_PIN | MANUAL_COVER_A_IN2_PIN |
                      MANUAL_COVER_B_IN1_PIN | MANUAL_COVER_B_IN2_PIN,
                      GPIO_PIN_RESET);
}

static void ManualCar_Cover4PinForward(void)
{
    HAL_GPIO_WritePin(MANUAL_COVER_A_IN1_GPIO_PORT, MANUAL_COVER_A_IN1_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(MANUAL_COVER_A_IN2_GPIO_PORT, MANUAL_COVER_A_IN2_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(MANUAL_COVER_B_IN1_GPIO_PORT, MANUAL_COVER_B_IN1_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(MANUAL_COVER_B_IN2_GPIO_PORT, MANUAL_COVER_B_IN2_PIN, GPIO_PIN_SET);
}

static void ManualCar_Cover4PinReverse(void)
{
    HAL_GPIO_WritePin(MANUAL_COVER_A_IN1_GPIO_PORT, MANUAL_COVER_A_IN1_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(MANUAL_COVER_A_IN2_GPIO_PORT, MANUAL_COVER_A_IN2_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(MANUAL_COVER_B_IN1_GPIO_PORT, MANUAL_COVER_B_IN1_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(MANUAL_COVER_B_IN2_GPIO_PORT, MANUAL_COVER_B_IN2_PIN, GPIO_PIN_RESET);
}

static void ManualCar_Cover4PinUpdate(void)
{
    enum
    {
        COVER_PHASE_FORWARD = 0,
        COVER_PHASE_FORWARD_STOP,
        COVER_PHASE_REVERSE,
        COVER_PHASE_REVERSE_STOP
    };
    static uint8_t phase = COVER_PHASE_FORWARD;
    static uint32_t phase_start_ms = 0U;
    uint32_t now = HAL_GetTick();
    uint32_t duration;

    if (phase_start_ms == 0U)
    {
        phase_start_ms = now;
        ManualCar_Cover4PinForward();
        return;
    }

    if (phase == COVER_PHASE_FORWARD)
    {
        duration = MANUAL_COVER_FORWARD_MS;
    }
    else if (phase == COVER_PHASE_REVERSE)
    {
        duration = MANUAL_COVER_REVERSE_MS;
    }
    else
    {
        duration = MANUAL_COVER_STOP_MS;
    }

    if ((now - phase_start_ms) < duration)
    {
        return;
    }

    if (phase == COVER_PHASE_FORWARD)
    {
        phase = COVER_PHASE_FORWARD_STOP;
        ManualCar_Cover4PinStop();
    }
    else if (phase == COVER_PHASE_FORWARD_STOP)
    {
        phase = COVER_PHASE_REVERSE;
        ManualCar_Cover4PinReverse();
    }
    else if (phase == COVER_PHASE_REVERSE)
    {
        phase = COVER_PHASE_REVERSE_STOP;
        ManualCar_Cover4PinStop();
    }
    else
    {
        phase = COVER_PHASE_FORWARD;
        ManualCar_Cover4PinForward();
    }
    phase_start_ms = now;
}

static void ManualCar_PumpCycleUpdate(void)
{
    static uint32_t last_switch_ms = 0U;
    static uint8_t pump_is_on = 0U;
    uint32_t now = HAL_GetTick();
    uint32_t interval = (pump_is_on != 0U) ? PUMP_ON_MS : PUMP_OFF_MS;

    if (last_switch_ms == 0U)
    {
        pump_is_on = 1U;
        last_switch_ms = now;
        Pump_On();
        HAL_GPIO_WritePin(GPIOF, GPIO_PIN_9, GPIO_PIN_RESET);
        return;
    }

    if ((now - last_switch_ms) < interval)
    {
        return;
    }

    pump_is_on = (pump_is_on == 0U) ? 1U : 0U;
    last_switch_ms = now;
    if (pump_is_on != 0U)
    {
        Pump_On();
        HAL_GPIO_WritePin(GPIOF, GPIO_PIN_9, GPIO_PIN_RESET);
    }
    else
    {
        Pump_Off();
        HAL_GPIO_WritePin(GPIOF, GPIO_PIN_9, GPIO_PIN_SET);
    }
}

static void ManualCar_WaterPumpPE5_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOE_CLK_ENABLE();

    HAL_GPIO_WritePin(MANUAL_WATER_PUMP_GPIO_PORT,
                      MANUAL_WATER_PUMP_PIN,
                      MANUAL_WATER_PUMP_OFF_LEVEL);

    GPIO_InitStruct.Pin = MANUAL_WATER_PUMP_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(MANUAL_WATER_PUMP_GPIO_PORT, &GPIO_InitStruct);

    HAL_GPIO_WritePin(MANUAL_WATER_PUMP_GPIO_PORT,
                      MANUAL_WATER_PUMP_PIN,
                      MANUAL_WATER_PUMP_OFF_LEVEL);
}

static void ManualCar_WaterPumpPE5Update(void)
{
    static uint32_t last_switch_ms = 0U;
    static uint8_t pump_is_on = 0U;
    uint32_t now = HAL_GetTick();
    uint32_t interval = (pump_is_on != 0U) ? MANUAL_WATER_PUMP_ON_MS : MANUAL_WATER_PUMP_OFF_MS;

    if (last_switch_ms == 0U)
    {
        pump_is_on = 1U;
        last_switch_ms = now;
        HAL_GPIO_WritePin(MANUAL_WATER_PUMP_GPIO_PORT,
                          MANUAL_WATER_PUMP_PIN,
                          MANUAL_WATER_PUMP_ON_LEVEL);
        return;
    }

    if ((now - last_switch_ms) < interval)
    {
        return;
    }

    pump_is_on = (pump_is_on == 0U) ? 1U : 0U;
    last_switch_ms = now;
    HAL_GPIO_WritePin(MANUAL_WATER_PUMP_GPIO_PORT,
                      MANUAL_WATER_PUMP_PIN,
                      (pump_is_on != 0U) ? MANUAL_WATER_PUMP_ON_LEVEL : MANUAL_WATER_PUMP_OFF_LEVEL);
}

static void ManualCar_BackgroundService(void)
{
    uint32_t now = HAL_GetTick();

    if ((now - manual_snake_last_imu_ms) >= 100U)
    {
        manual_snake_last_imu_ms = now;
        IMU_I2C_ReadAngles();
    }

    if (imu_i2c_data_valid == 0U && (now - manual_snake_last_scan_ms) >= 1000U)
    {
        manual_snake_last_scan_ms = now;
        IMU_I2C_Scan();
    }

    if ((now - manual_snake_last_lcd_ms) >= 200U)
    {
        manual_snake_last_lcd_ms = now;
        LCD_UI_Update();
    }
}

static ManualControlMode_t ManualCar_ReadControlMode(void)
{
    uint32_t now = HAL_GetTick();
    uint32_t pulse_us;

    if ((now - speed_rc.last_ms) >= RC_TIMEOUT_MS)
    {
        return MANUAL_CONTROL_MODE_IDLE;
    }

    pulse_us = speed_rc.pulse_us;
    if (pulse_us <= MANUAL_MODE_IDLE_MAX_US)
    {
        return MANUAL_CONTROL_MODE_IDLE;
    }
    if (pulse_us <= MANUAL_MODE_WORK_MAX_US)
    {
        return MANUAL_CONTROL_MODE_SEEDLING;
    }
    return MANUAL_CONTROL_MODE_AUTO_SPEED;
}

static void ManualCar_AutoSpeedConfigUpdate(void)
{
    uint32_t now = HAL_GetTick();
    uint32_t pulse_us;
    uint32_t duty_percent;

    if ((now - car_rc_ch2.last_ms) >= RC_TIMEOUT_MS)
    {
        manual_auto_speed_adjust_armed = 1U;
        return;
    }

    pulse_us = car_rc_ch2.pulse_us;
    if (pulse_us >= MANUAL_AUTO_SPEED_CENTER_MIN_US &&
        pulse_us <= MANUAL_AUTO_SPEED_CENTER_MAX_US)
    {
        manual_auto_speed_adjust_armed = 1U;
        return;
    }

    if (manual_auto_speed_adjust_armed == 0U)
    {
        return;
    }

    duty_percent = Car_Motor_GetDefaultRunDuty();
    if (pulse_us >= MANUAL_AUTO_SPEED_INC_MIN_US)
    {
        if (duty_percent < MANUAL_AUTO_SPEED_MAX_PERCENT)
        {
            duty_percent += MANUAL_AUTO_SPEED_STEP_PERCENT;
            if (duty_percent > MANUAL_AUTO_SPEED_MAX_PERCENT)
            {
                duty_percent = MANUAL_AUTO_SPEED_MAX_PERCENT;
            }
            Car_Motor_SetDefaultRunDuty(duty_percent);
        }
        manual_auto_speed_adjust_armed = 0U;
        return;
    }

    if (pulse_us <= MANUAL_AUTO_SPEED_DEC_MAX_US)
    {
        if (duty_percent > MANUAL_AUTO_SPEED_MIN_PERCENT)
        {
            if (duty_percent > (MANUAL_AUTO_SPEED_MIN_PERCENT + MANUAL_AUTO_SPEED_STEP_PERCENT))
            {
                duty_percent -= MANUAL_AUTO_SPEED_STEP_PERCENT;
            }
            else
            {
                duty_percent = MANUAL_AUTO_SPEED_MIN_PERCENT;
            }
            Car_Motor_SetDefaultRunDuty(duty_percent);
        }
        manual_auto_speed_adjust_armed = 0U;
    }
}

static uint8_t ManualCar_ModeAutoRequested(uint8_t current_auto)
{
    uint32_t now = HAL_GetTick();
    uint32_t pulse_us;

    if ((now - work_switch_rc.last_ms) >= RC_TIMEOUT_MS)
    {
        return current_auto;
    }

    pulse_us = work_switch_rc.pulse_us;
    if (pulse_us >= SNAKE_MODE_AUTO_MIN_US)
    {
        return 1U;
    }
    if (pulse_us <= SNAKE_MODE_MANUAL_MAX_US)
    {
        return 0U;
    }
    return current_auto;
}

static uint8_t ManualCar_DelayMs(uint32_t delay_ms)
{
    uint32_t start = HAL_GetTick();

    while ((HAL_GetTick() - start) < delay_ms)
    {
        ManualCar_BackgroundService();
        if (ManualCar_ModeAutoRequested(1U) == 0U)
        {
            manual_snake_abort_requested = 1U;
            ManualCar_AllStop();
            return 0U;
        }
        HAL_Delay(5U);
    }
    return 1U;
}

static void ManualCar_MobileStopAuto(void)
{
    manual_mobile_stepper_active = 0U;
    manual_mobile_stepper_low = 0U;
    HAL_GPIO_WritePin(MOBILE_STEP_GPIO_PORT, MOBILE_STEP_PIN, GPIO_PIN_SET);
}

static void ManualCar_MobileStartAuto(uint8_t dir)
{
    HAL_GPIO_WritePin(MOBILE_DIR_GPIO_PORT,
                      MOBILE_DIR_PIN,
                      (dir == FORWARD) ? GPIO_PIN_RESET : GPIO_PIN_SET);
    manual_mobile_stepper_low = 0U;
    manual_mobile_stepper_active = 1U;
}

static void ManualCar_Servo51Stop(void)
{
    manual_servo13_active = 0U;
    manual_servo13_low = 0U;
    HAL_GPIO_WritePin(MANUAL_SERVO13_PUL_GPIO_PORT,
                      MANUAL_SERVO13_PUL_PIN,
                      GPIO_PIN_SET);
}

static uint8_t ManualCar_Servo51PulseSegment(uint8_t abort_if_manual)
{
    uint32_t pulse;

    if (abort_if_manual != 0U && ManualCar_ModeAutoRequested(1U) == 0U)
    {
        manual_snake_abort_requested = 1U;
        ManualCar_Servo51Stop();
        return 0U;
    }

    manual_servo13_active = 1U;
    manual_servo13_low = 0U;
    manual_servo13_pulse_count = 0U;
    HAL_GPIO_WritePin(GPIOF, GPIO_PIN_10, GPIO_PIN_RESET);

    for (pulse = 0U; pulse < MANUAL_SERVO13_PULSES_PER_SLOT; pulse++)
    {
        HAL_GPIO_WritePin(MANUAL_SERVO13_PUL_GPIO_PORT,
                          MANUAL_SERVO13_PUL_PIN,
                          GPIO_PIN_RESET);
        delay_us(MANUAL_SERVO13_PULSE_LOW_US);

        HAL_GPIO_WritePin(MANUAL_SERVO13_PUL_GPIO_PORT,
                          MANUAL_SERVO13_PUL_PIN,
                          GPIO_PIN_SET);
        manual_servo13_pulse_count++;

        delay_us(MANUAL_SERVO13_PULSE_RELEASE_US);
    }

    HAL_GPIO_WritePin(GPIOF, GPIO_PIN_10, GPIO_PIN_SET);
    ManualCar_Servo51Stop();

    if (abort_if_manual != 0U && ManualCar_ModeAutoRequested(1U) == 0U)
    {
        manual_snake_abort_requested = 1U;
        return 0U;
    }

    return 1U;
}

static void ManualCar_ManualDrillControlUpdate(void)
{
    enum
    {
        DRILL_MANUAL_IDLE = 0,
        DRILL_MANUAL_DOWN,
        DRILL_MANUAL_UP
    };
    static uint8_t state = DRILL_MANUAL_IDLE;
    uint32_t now = HAL_GetTick();
    uint32_t pulse_us;

    if ((now - row_continue_rc.last_ms) >= RC_TIMEOUT_MS)
    {
        state = DRILL_MANUAL_IDLE;
        ManualCar_MobileStopAuto();
        Pump_Off();
        return;
    }

    pulse_us = row_continue_rc.pulse_us;
    if (pulse_us <= MANUAL_DRILL_DOWN_MIN_US)
    {
        state = DRILL_MANUAL_IDLE;
        ManualCar_MobileStopAuto();
        Pump_Off();
        return;
    }

    if (state == DRILL_MANUAL_IDLE)
    {
        state = (pulse_us > MANUAL_DRILL_UP_MIN_US) ? DRILL_MANUAL_UP : DRILL_MANUAL_DOWN;
    }
    else if (state == DRILL_MANUAL_DOWN && pulse_us > MANUAL_DRILL_UP_MIN_US)
    {
        state = DRILL_MANUAL_UP;
    }

    Pump_On();
    if (state == DRILL_MANUAL_UP)
    {
        ManualCar_MobileStartAuto(REVERSE);
    }
    else
    {
        ManualCar_MobileStartAuto(FORWARD);
    }
}

static void ManualCar_CarStopAuto(void)
{
    Car_Motor_Stop();
}

static void ManualCar_CarForwardAuto(void)
{
    Car_Motor_SetLeft(1);
    Car_Motor_SetRight(1);
}

static void ManualCar_CarRightAuto(void)
{
    Car_Motor_SetLeft(1);
    Car_Motor_SetRight(-1);
}

static void ManualCar_CarLeftAuto(void)
{
    Car_Motor_SetLeft(-1);
    Car_Motor_SetRight(1);
}

static uint8_t ManualCar_CarRun(void (*move)(void), uint32_t run_ms)
{
    if (move != NULL)
    {
        move();
    }
    if (ManualCar_DelayMs(run_ms) == 0U)
    {
        ManualCar_CarStopAuto();
        return 0U;
    }
    ManualCar_CarStopAuto();
    return 1U;
}

static void ManualCar_WaterPumpSet(uint8_t on)
{
    HAL_GPIO_WritePin(MANUAL_WATER_PUMP_GPIO_PORT,
                      MANUAL_WATER_PUMP_PIN,
                      (on != 0U) ? MANUAL_WATER_PUMP_ON_LEVEL : MANUAL_WATER_PUMP_OFF_LEVEL);
}

static void ManualCar_CoverOnlyStop(void)
{
    HAL_GPIO_WritePin(MANUAL_COVER_A_IN1_GPIO_PORT, MANUAL_COVER_A_IN1_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(MANUAL_COVER_A_IN2_GPIO_PORT, MANUAL_COVER_A_IN2_PIN, GPIO_PIN_RESET);
}

static void ManualCar_CoverOnlyExtend(void)
{
    HAL_GPIO_WritePin(MANUAL_COVER_A_IN1_GPIO_PORT, MANUAL_COVER_A_IN1_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(MANUAL_COVER_A_IN2_GPIO_PORT, MANUAL_COVER_A_IN2_PIN, GPIO_PIN_SET);
}

static void ManualCar_CoverOnlyRetract(void)
{
    HAL_GPIO_WritePin(MANUAL_COVER_A_IN1_GPIO_PORT, MANUAL_COVER_A_IN1_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(MANUAL_COVER_A_IN2_GPIO_PORT, MANUAL_COVER_A_IN2_PIN, GPIO_PIN_RESET);
}

static void ManualCar_ManualCoverPB14Update(void)
{
    uint32_t now = HAL_GetTick();
    uint32_t pulse_us;

    if ((now - row_continue_rc.last_ms) >= RC_TIMEOUT_MS)
    {
        ManualCar_CoverOnlyStop();
        return;
    }

    pulse_us = row_continue_rc.pulse_us;
    if (pulse_us <= MANUAL_RANGE_STOP_MAX_US)
    {
        ManualCar_CoverOnlyStop();
        return;
    }

    if (pulse_us <= MANUAL_RANGE_EXTEND_MAX_US)
    {
        ManualCar_CoverOnlyExtend();
    }
    else
    {
        ManualCar_CoverOnlyRetract();
    }
}

static void ManualCar_ManualSeedPumpControlUpdate(void)
{
    enum
    {
        SEED_MANUAL_IDLE = 0,
        SEED_MANUAL_RUN,
        SEED_MANUAL_STOP
    };
    static uint8_t seed_state = SEED_MANUAL_IDLE;
    static uint32_t seed_stop_start_ms = 0U;
    uint32_t now = HAL_GetTick();
    uint32_t pulse_us;

    if ((now - manual_seed_pump_rc.last_ms) >= RC_TIMEOUT_MS)
    {
        seed_state = SEED_MANUAL_IDLE;
        ManualCar_Servo51Stop();
        ManualCar_WaterPumpSet(0U);
        return;
    }

    pulse_us = manual_seed_pump_rc.pulse_us;
    if (pulse_us >= MANUAL_SEED_PUMP_CENTER_MIN_US &&
        pulse_us <= MANUAL_SEED_PUMP_CENTER_MAX_US)
    {
        seed_state = SEED_MANUAL_IDLE;
        ManualCar_Servo51Stop();
        ManualCar_WaterPumpSet(0U);
        return;
    }

    if (pulse_us >= MANUAL_SEED_PUMP_HIGH_MIN_US)
    {
        ManualCar_WaterPumpSet(0U);
        if (seed_state == SEED_MANUAL_IDLE)
        {
            (void)ManualCar_Servo51PulseSegment(0U);
            seed_stop_start_ms = HAL_GetTick();
            seed_state = SEED_MANUAL_STOP;
        }
        else if (seed_state == SEED_MANUAL_STOP)
        {
            if ((now - seed_stop_start_ms) >= MANUAL_SERVO13_SEGMENT_STOP_MS)
            {
                (void)ManualCar_Servo51PulseSegment(0U);
                seed_stop_start_ms = HAL_GetTick();
            }
        }
        return;
    }

    if (pulse_us <= MANUAL_SEED_PUMP_LOW_MAX_US)
    {
        seed_state = SEED_MANUAL_IDLE;
        ManualCar_Servo51Stop();
        ManualCar_WaterPumpSet(1U);
        return;
    }

    seed_state = SEED_MANUAL_IDLE;
    ManualCar_Servo51Stop();
    ManualCar_WaterPumpSet(0U);
}

static void ManualCar_SolarPanelStop(void)
{
    HAL_GPIO_WritePin(MANUAL_COVER_B_IN1_GPIO_PORT, MANUAL_COVER_B_IN1_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(MANUAL_COVER_B_IN2_GPIO_PORT, MANUAL_COVER_B_IN2_PIN, GPIO_PIN_RESET);
}

static void ManualCar_SolarPanelExtend(void)
{
    HAL_GPIO_WritePin(MANUAL_COVER_B_IN1_GPIO_PORT, MANUAL_COVER_B_IN1_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(MANUAL_COVER_B_IN2_GPIO_PORT, MANUAL_COVER_B_IN2_PIN, GPIO_PIN_SET);
}

static void ManualCar_SolarPanelRetract(void)
{
    HAL_GPIO_WritePin(MANUAL_COVER_B_IN1_GPIO_PORT, MANUAL_COVER_B_IN1_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(MANUAL_COVER_B_IN2_GPIO_PORT, MANUAL_COVER_B_IN2_PIN, GPIO_PIN_RESET);
}

static void ManualCar_ManualSolarPF11Update(void)
{
    uint32_t now = HAL_GetTick();
    uint32_t pulse_us;

    if ((now - manual_seed_pump_rc.last_ms) >= RC_TIMEOUT_MS)
    {
        ManualCar_SolarPanelStop();
        return;
    }

    pulse_us = manual_seed_pump_rc.pulse_us;
    if (pulse_us <= MANUAL_SOLAR_EXTEND_MAX_US)
    {
        ManualCar_SolarPanelExtend();
        return;
    }

    if (pulse_us < MANUAL_SOLAR_STOP_MAX_US)
    {
        ManualCar_SolarPanelStop();
    }
    else
    {
        ManualCar_SolarPanelRetract();
    }
}

static void ManualCar_AllStop(void)
{
    ManualCar_CarStopAuto();
    Pump_Off();
    ManualCar_MobileStopAuto();
    ManualCar_WaterPumpSet(0U);
    ManualCar_CoverOnlyStop();
    ManualCar_SolarPanelStop();
    ManualCar_Servo51Stop();
}

static void ManualCar_Servo51OneStep(void)
{
    if (ManualCar_Servo51PulseSegment(1U) == 0U)
    {
        return;
    }

    (void)ManualCar_DelayMs(100U);
}

static void ManualCar_DrillHole(void)
{
    Pump_Off();
    ManualCar_MobileStartAuto(FORWARD);
    if (ManualCar_DelayMs(SNAKE_DRILL_ON_AFTER_DOWN_MS) == 0U) return;
    Pump_On();
    if (ManualCar_DelayMs(SNAKE_DRILL_DOWN_MS - SNAKE_DRILL_ON_AFTER_DOWN_MS) == 0U) return;
    ManualCar_MobileStopAuto();
    if (ManualCar_DelayMs(SNAKE_DRILL_BOTTOM_HOLD_MS) == 0U) return;

    ManualCar_MobileStartAuto(REVERSE);
    if (ManualCar_DelayMs(SNAKE_DRILL_OFF_AFTER_UP_MS) == 0U) return;
    Pump_Off();
    if (ManualCar_DelayMs(SNAKE_DRILL_UP_MS - SNAKE_DRILL_OFF_AFTER_UP_MS) == 0U) return;
    ManualCar_MobileStopAuto();
    Pump_Off();
}

static void ManualCar_CoverSoilAuto(void)
{
    ManualCar_SolarPanelStop();
    ManualCar_CoverOnlyExtend();
    if (ManualCar_DelayMs(SNAKE_COVER_EXTEND_MS) == 0U) return;
    ManualCar_CoverOnlyStop();
    if (ManualCar_DelayMs(SNAKE_COVER_STOP_MS) == 0U) return;
    ManualCar_CoverOnlyRetract();
    if (ManualCar_DelayMs(SNAKE_COVER_RETRACT_MS) == 0U) return;
    ManualCar_CoverOnlyStop();
    ManualCar_SolarPanelStop();
}

static void ManualCar_WaterAuto(void)
{
    ManualCar_WaterPumpSet(1U);
    (void)ManualCar_DelayMs(SNAKE_WATER_MS);
    ManualCar_WaterPumpSet(0U);
}

static void ManualCar_RowChangeRight(void)
{
    if (ManualCar_CarRun(ManualCar_CarForwardAuto, SNAKE_CAR_ROW_MOVE_MS) == 0U) return;
    if (ManualCar_CarRun(ManualCar_CarRightAuto, SNAKE_CAR_ROW_TURN_FIRST_MS) == 0U) return;
    if (ManualCar_CarRun(ManualCar_CarForwardAuto, SNAKE_CAR_ROW_MOVE_MS) == 0U) return;
    if (ManualCar_CarRun(ManualCar_CarRightAuto, SNAKE_CAR_ROW_TURN_SECOND_MS) == 0U) return;
    (void)ManualCar_CarRun(ManualCar_CarForwardAuto, SNAKE_CAR_ROW_MOVE_MS);
}

static void ManualCar_RowChangeLeft(void)
{
    if (ManualCar_CarRun(ManualCar_CarForwardAuto, SNAKE_CAR_ROW_MOVE_MS) == 0U) return;
    if (ManualCar_CarRun(ManualCar_CarLeftAuto, SNAKE_CAR_ROW_TURN_FIRST_MS) == 0U) return;
    if (ManualCar_CarRun(ManualCar_CarForwardAuto, SNAKE_CAR_ROW_MOVE_MS) == 0U) return;
    if (ManualCar_CarRun(ManualCar_CarLeftAuto, SNAKE_CAR_ROW_TURN_SECOND_MS) == 0U) return;
    (void)ManualCar_CarRun(ManualCar_CarForwardAuto, SNAKE_CAR_ROW_MOVE_MS);
}

static void PlantOneSeedling(uint8_t seed_index)
{
    if (manual_snake_resume_phase <= MANUAL_SNAKE_PHASE_DRILL)
    {
        manual_snake_resume_phase = MANUAL_SNAKE_PHASE_DRILL;
        ManualCar_DrillHole();
        if (manual_snake_abort_requested != 0U) return;
        manual_snake_resume_phase = MANUAL_SNAKE_PHASE_AFTER_DRILL_CAR;
    }

    if (manual_snake_resume_phase == MANUAL_SNAKE_PHASE_AFTER_DRILL_CAR)
    {
        if (ManualCar_CarRun(ManualCar_CarForwardAuto, SNAKE_CAR_AFTER_DRILL_MS) == 0U) return;
        manual_snake_resume_phase = MANUAL_SNAKE_PHASE_SEED;
    }

    if (manual_snake_resume_phase == MANUAL_SNAKE_PHASE_SEED)
    {
        ManualCar_Servo51OneStep();
        if (manual_snake_abort_requested != 0U) return;
        manual_snake_resume_phase = MANUAL_SNAKE_PHASE_COVER;
    }

    if (manual_snake_resume_phase == MANUAL_SNAKE_PHASE_COVER)
    {
        ManualCar_CoverSoilAuto();
        if (manual_snake_abort_requested != 0U) return;
        manual_snake_resume_phase = MANUAL_SNAKE_PHASE_WATER;
    }

    if (manual_snake_resume_phase == MANUAL_SNAKE_PHASE_WATER)
    {
        ManualCar_WaterAuto();
        if (manual_snake_abort_requested != 0U) return;
        manual_snake_resume_phase = MANUAL_SNAKE_PHASE_NEXT_CAR;
    }

    if (manual_snake_resume_phase == MANUAL_SNAKE_PHASE_NEXT_CAR &&
        seed_index < SNAKE_TOTAL_SEEDLINGS)
    {
        (void)ManualCar_CarRun(ManualCar_CarForwardAuto, SNAKE_CAR_NEXT_POINT_MS);
        if (manual_snake_abort_requested != 0U) return;
    }

    manual_snake_resume_phase = MANUAL_SNAKE_PHASE_DONE;
    if (seed_index >= SNAKE_TOTAL_SEEDLINGS)
    {
        ManualCar_AllStop();
    }
}

static void ManualCar_RunSnakeWorkflow(void)
{
    uint8_t seed_index;

    manual_snake_abort_requested = 0U;
    ManualCar_AllStop();

    if (manual_snake_resume_seed == 0U || manual_snake_resume_seed > SNAKE_TOTAL_SEEDLINGS)
    {
        manual_snake_resume_seed = 1U;
        manual_snake_resume_phase = MANUAL_SNAKE_PHASE_DRILL;
    }
    if (manual_snake_resume_phase >= MANUAL_SNAKE_PHASE_DONE)
    {
        manual_snake_resume_phase = MANUAL_SNAKE_PHASE_DRILL;
    }

    for (seed_index = manual_snake_resume_seed; seed_index <= SNAKE_TOTAL_SEEDLINGS; seed_index++)
    {
        if (manual_snake_abort_requested != 0U)
        {
            manual_snake_resume_seed = seed_index;
            break;
        }

        PlantOneSeedling(seed_index);

        if (manual_snake_abort_requested != 0U)
        {
            manual_snake_resume_seed = seed_index;
            break;
        }

        manual_snake_resume_seed = (uint8_t)(seed_index + 1U);
        manual_snake_resume_phase = MANUAL_SNAKE_PHASE_DRILL;
    }

    if (manual_snake_resume_seed > SNAKE_TOTAL_SEEDLINGS)
    {
        manual_snake_resume_seed = 0U;
        manual_snake_resume_phase = MANUAL_SNAKE_PHASE_DONE;
    }

    ManualCar_AllStop();
}
#endif

#if defined(SEEDLING_TEST_ONLY) && (SEEDLING_TEST_ONLY != 0)
static void Seedling_TestRun(uint8_t dir, uint32_t run_ms, uint32_t freq_hz)
{
    uint32_t start_tick;
    uint32_t period_us;

    if (freq_hz == 0U)
    {
        return;
    }

    period_us = 1000000U / freq_hz;
    Seedling_SetDir(dir);
    start_tick = HAL_GetTick();
    while ((HAL_GetTick() - start_tick) < run_ms)
    {
        Seedling_OnePulse(period_us);
    }
    Seedling_Stop();
}
#endif

#if defined(DUAL_STEPPERS_TEST) && (DUAL_STEPPERS_TEST != 0)
static void Dual_GetSeedlingState(uint32_t tick_ms, uint8_t *active, uint8_t *dir)
{
    uint32_t phase = tick_ms % 50000U;

    if (phase < 7000U)
    {
        *active = 1U;
        *dir = FORWARD;
    }
    else if (phase < 9000U)
    {
        *active = 0U;
        *dir = FORWARD;
    }
    else if (phase < 16000U)
    {
        *active = 1U;
        *dir = FORWARD;
    }
    else if (phase < 18000U)
    {
        *active = 0U;
        *dir = FORWARD;
    }
    else if (phase < 25000U)
    {
        *active = 1U;
        *dir = FORWARD;
    }
    else if (phase < 27000U)
    {
        *active = 0U;
        *dir = REVERSE;
    }
    else if (phase < 48000U)
    {
        *active = 1U;
        *dir = REVERSE;
    }
    else
    {
        *active = 0U;
        *dir = FORWARD;
    }
}

static void Dual_GetMobileState(uint32_t tick_ms, uint8_t *active, uint8_t *dir)
{
    uint32_t phase = tick_ms % 20000U;

    if (phase < 8000U)
    {
        *active = 1U;
        *dir = FORWARD;
    }
    else if (phase < 10000U)
    {
        *active = 0U;
        *dir = FORWARD;
    }
    else if (phase < 18000U)
    {
        *active = 1U;
        *dir = REVERSE;
    }
    else
    {
        *active = 0U;
        *dir = REVERSE;
    }
}

static void Dual_SteppersService(void)
{
    static uint8_t mobile_step_divider = 0U;
    uint32_t now = HAL_GetTick();
    uint8_t seedling_active;
    uint8_t seedling_dir;
    uint8_t mobile_active;
    uint8_t mobile_dir;
    uint8_t mobile_step_active = 0U;
    const uint32_t pulse_us = 40U;
    const uint32_t period_us = 139U;

    Dual_GetSeedlingState(now, &seedling_active, &seedling_dir);
    Dual_GetMobileState(now, &mobile_active, &mobile_dir);

    HAL_GPIO_WritePin(SEEDLING_DIR_GPIO_PORT, SEEDLING_DIR_PIN,
                      (seedling_dir == FORWARD) ? DIR_FORWARD_LEVEL : DIR_REVERSE_LEVEL);
    HAL_GPIO_WritePin(MOBILE_DIR_GPIO_PORT, MOBILE_DIR_PIN,
                      (mobile_dir == FORWARD) ? GPIO_PIN_RESET : GPIO_PIN_SET);

    HAL_GPIO_WritePin(GPIOF, GPIO_PIN_9, seedling_active ? GPIO_PIN_RESET : GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOF, GPIO_PIN_10, mobile_active ? GPIO_PIN_RESET : GPIO_PIN_SET);

    if (mobile_active != 0U)
    {
        mobile_step_divider++;
        if (mobile_step_divider >= 3U)
        {
            mobile_step_divider = 0U;
            mobile_step_active = 1U;
        }
    }
    else
    {
        mobile_step_divider = 0U;
    }

    HAL_GPIO_WritePin(SEEDLING_STEP_GPIO_PORT, SEEDLING_STEP_PIN,
                      seedling_active ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(MOBILE_STEP_GPIO_PORT, MOBILE_STEP_PIN,
                      mobile_step_active ? GPIO_PIN_RESET : GPIO_PIN_SET);
    delay_us(pulse_us);
    HAL_GPIO_WritePin(SEEDLING_STEP_GPIO_PORT, SEEDLING_STEP_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(MOBILE_STEP_GPIO_PORT, MOBILE_STEP_PIN, GPIO_PIN_SET);
    delay_us(period_us - pulse_us);
}
#endif

#if (defined(CAR_TEST_ONLY) && (CAR_TEST_ONLY != 0)) || \
    (defined(CAR_IMU_ROW_TEST) && (CAR_IMU_ROW_TEST != 0))
static void Car_TestRun(uint32_t target_us, uint32_t run_ms)
{
    uint32_t start = HAL_GetTick();

    while ((HAL_GetTick() - start) < run_ms)
    {
        uint32_t elapsed = HAL_GetTick() - start;
        uint32_t pulse = WORK_CAR_NEUTRAL_US;

        if (elapsed < WORK_CAR_RAMP_MS)
        {
            if (target_us >= WORK_CAR_NEUTRAL_US)
            {
                pulse = WORK_CAR_NEUTRAL_US +
                        ((target_us - WORK_CAR_NEUTRAL_US) * elapsed) / WORK_CAR_RAMP_MS;
            }
            else
            {
                pulse = WORK_CAR_NEUTRAL_US -
                        ((WORK_CAR_NEUTRAL_US - target_us) * elapsed) / WORK_CAR_RAMP_MS;
            }
        }
        else if (elapsed < WORK_CAR_HOLD_END_MS)
        {
            pulse = target_us;
        }
        else
        {
            uint32_t down_elapsed = elapsed - WORK_CAR_HOLD_END_MS;

            if (target_us >= WORK_CAR_NEUTRAL_US)
            {
                pulse = target_us -
                        ((target_us - WORK_CAR_NEUTRAL_US) * down_elapsed) / WORK_CAR_RAMP_MS;
            }
            else
            {
                pulse = target_us +
                        ((WORK_CAR_NEUTRAL_US - target_us) * down_elapsed) / WORK_CAR_RAMP_MS;
            }
        }

        Car_PWM_OutputSet(pulse, pulse);
        HAL_Delay(20U);
    }

    Car_PWM_OutputNeutral();
}
#endif

#if (defined(DRILL_ROD_TEST_ONLY) && (DRILL_ROD_TEST_ONLY != 0)) || \
    (defined(DRILL_TEST_ONLY) && (DRILL_TEST_ONLY != 0))
static void Drill_TestPwm(uint16_t duty_x1000, uint32_t run_ms)
{
    const uint32_t period_us = 20000U;
    uint32_t start = HAL_GetTick();

    if (duty_x1000 >= 1000U)
    {
        Drill_On();
        HAL_Delay(run_ms);
        return;
    }
    if (duty_x1000 == 0U)
    {
        Drill_Off();
        HAL_Delay(run_ms);
        return;
    }

    while ((HAL_GetTick() - start) < run_ms)
    {
        uint32_t on_us = (period_us * (uint32_t)duty_x1000) / 1000U;

        Drill_On();
        delay_us(on_us);
        Drill_Off();
        delay_us(period_us - on_us);
    }
}

static void Drill_TestRamp(uint8_t ramp_up, uint32_t ramp_ms)
{
    const uint32_t period_ms = 20U;
    uint32_t start = HAL_GetTick();

    while ((HAL_GetTick() - start) < ramp_ms)
    {
        uint32_t elapsed = HAL_GetTick() - start;
        uint16_t duty;

        if (elapsed > ramp_ms)
        {
            elapsed = ramp_ms;
        }

        if (ramp_up != 0U)
        {
            duty = (uint16_t)((elapsed * 1000U) / ramp_ms);
        }
        else
        {
            duty = (uint16_t)(1000U - ((elapsed * 1000U) / ramp_ms));
        }
        Drill_TestPwm(duty, period_ms);
    }
}
#endif

#if defined(DRILL_ROD_TEST_ONLY) && (DRILL_ROD_TEST_ONLY != 0)
static void Drill_RodLoopTestCycle(void)
{
    Drill_Off();
    DrillRod_Down();
    HAL_Delay(10000U);

    DrillRod_Stop();
    HAL_Delay(500U);

    Drill_On();
    DrillRod_Down();
    HAL_Delay(5000U);
    DrillRod_Stop();
    DrillRod_Up();
    HAL_Delay(5000U);
    DrillRod_Stop();
    Drill_Off();
    HAL_Delay(500U);

    DrillRod_Up();
    HAL_Delay(10000U);
    DrillRod_Stop();
    HAL_Delay(1000U);
}
#endif

#if (defined(CAR_IMU_ROW_TEST) && (CAR_IMU_ROW_TEST != 0)) || \
    (defined(CAR_MANUAL_AUTO_TEST_ONLY) && (CAR_MANUAL_AUTO_TEST_ONLY != 0))
#define CAR_ROW_TURN_RIGHT_US 1500U
#define CAR_ROW_TURN_DEG_X100 9000L
#define CAR_ROW_FORWARD_US 1520U
#define CAR_ROW_FORWARD_MS 500U
#define CAR_ROW_BETWEEN_FORWARD_MS 5000U
#define CAR_ROW_TOTAL_STEPS 9U
#define CAR_ROW_BEFORE_TURN_MS 2000U
#define CAR_ROW_AFTER_TURN_MS 2000U
#define CAR_ROW_FORWARD_DELTA_US (CAR_ROW_FORWARD_US - WORK_CAR_NEUTRAL_US)
#define CAR_ROW_TURN_LEFT_REVERSE_US RC_LOW_LIMIT_US
#define CAR_ROW_TURN_RIGHT_FAST_US RC_HIGH_LIMIT_US
#define CAR_ROW_SPEED_INPUT_MIN_US 1500U
#define CAR_ROW_SPEED_INPUT_MAX_US 2000U
#define CAR_ROW_SPEED_OUTPUT_MIN_US CAR_ROW_FORWARD_US
#define CAR_ROW_SPEED_OUTPUT_MAX_US RC_HIGH_LIMIT_US

static int32_t Car_RowNormalizeDelta(int32_t current_x100, int32_t base_x100)
{
    int32_t delta = current_x100 - base_x100;

    while (delta > 18000L)
    {
        delta -= 36000L;
    }
    while (delta < -18000L)
    {
        delta += 36000L;
    }
    return delta;
}

static uint8_t Car_RowSwitchRunEnabled(void);
static uint8_t Car_RowMasterPauseActive(void);
static uint32_t Car_RowForwardPulseUs(void);
static void Car_RowSendYawToUSART1(uint32_t now);

static void Car_RowSendYawToUSART1(uint32_t now)
{
    static uint32_t last_tx_ms;
    char text[48];
    int32_t yaw_copy;
    uint8_t valid_copy;

    if ((now - last_tx_ms) < 200U)
    {
        return;
    }
    last_tx_ms = now;

    __disable_irq();
    yaw_copy = imu_yaw_x100;
    valid_copy = imu_i2c_data_valid;
    __enable_irq();

    if (valid_copy != 0U)
    {
        snprintf(text, sizeof(text), "YAW=%ld.%02ld\r\n",
                 (long)(yaw_copy / 100L), (long)(labs(yaw_copy) % 100L));
    }
    else
    {
        snprintf(text, sizeof(text), "YAW=INVALID\r\n");
    }
    UART1_QueueString(text);
}

static void Car_RowService(uint32_t *last_imu_ms, uint32_t *last_lcd_ms, uint32_t *last_scan_ms)
{
    uint32_t now = HAL_GetTick();
    uint8_t switch_run = Car_RowSwitchRunEnabled();
    uint8_t extra_valid = ((now - row_continue_rc.last_ms) < RC_TIMEOUT_MS) ? 1U : 0U;
    uint8_t speed_valid = ((now - speed_rc.last_ms) < RC_TIMEOUT_MS) ? 1U : 0U;
    char pb14_text[20];
    char pa8_text[20];

    __disable_irq();
    if (extra_valid != 0U)
    {
        snprintf(pb14_text, sizeof(pb14_text), "%s %lu",
                 (row_continue_rc.pulse_us >= WORK_SWITCH_THRESHOLD_US) ? "HI" : "LO",
                 (unsigned long)row_continue_rc.pulse_us);
    }
    else
    {
        snprintf(pb14_text, sizeof(pb14_text), "--");
    }
    if (speed_valid != 0U)
    {
        snprintf(pa8_text, sizeof(pa8_text), "%lu",
                 (unsigned long)speed_rc.pulse_us);
    }
    else
    {
        snprintf(pa8_text, sizeof(pa8_text), "--");
    }
    snprintf(workflow_status_buf, WORKFLOW_STATUS_BUF_SIZE,
             "SWA %s %lu SWB %s THR %s OUT %lu",
             (switch_run != 0U) ? "HI" : "LO",
             (unsigned long)work_switch_rc.pulse_us,
             pb14_text,
             pa8_text,
             (unsigned long)Car_RowForwardPulseUs());
    __enable_irq();

    USART1_ServiceEcho();
    USART2_ServiceEcho();
    USART3_ServiceEcho();
    Ultrasonic_Service();

    if ((now - *last_imu_ms) >= 100U)
    {
        *last_imu_ms = now;
        IMU_I2C_ReadAngles();
    }
    Car_RowSendYawToUSART1(now);

    if (imu_i2c_data_valid == 0U && (now - *last_scan_ms) >= 1000U)
    {
        *last_scan_ms = now;
        IMU_I2C_Scan();
    }

    if ((now - *last_lcd_ms) >= 200U)
    {
        *last_lcd_ms = now;
        LCD_UI_Update();
    }
}

static uint8_t Car_RowSwitchRunEnabled(void)
{
    uint32_t now = HAL_GetTick();

    if ((now - work_switch_rc.last_ms) >= RC_TIMEOUT_MS)
    {
        return 0U;
    }

    return (work_switch_rc.pulse_us >= WORK_SWITCH_THRESHOLD_US) ? 1U : 0U;
}

static uint8_t Car_RowMasterPauseActive(void)
{
    uint32_t now = HAL_GetTick();

    if ((now - row_continue_rc.last_ms) >= RC_TIMEOUT_MS)
    {
        return 1U;
    }

    return (row_continue_rc.pulse_us >= WORK_SWITCH_THRESHOLD_US) ? 0U : 1U;
}

static uint32_t Car_RowForwardPulseUs(void)
{
    uint32_t now = HAL_GetTick();
    uint32_t speed_us;

    if ((now - speed_rc.last_ms) >= RC_TIMEOUT_MS)
    {
        return CAR_ROW_SPEED_OUTPUT_MIN_US;
    }

    speed_us = speed_rc.pulse_us;
    if (speed_us < CAR_ROW_SPEED_INPUT_MIN_US)
    {
        speed_us = CAR_ROW_SPEED_INPUT_MIN_US;
    }
    if (speed_us > CAR_ROW_SPEED_INPUT_MAX_US)
    {
        speed_us = CAR_ROW_SPEED_INPUT_MAX_US;
    }

    return CAR_ROW_SPEED_OUTPUT_MIN_US +
           ((speed_us - CAR_ROW_SPEED_INPUT_MIN_US) *
            (CAR_ROW_SPEED_OUTPUT_MAX_US - CAR_ROW_SPEED_OUTPUT_MIN_US)) /
           (CAR_ROW_SPEED_INPUT_MAX_US - CAR_ROW_SPEED_INPUT_MIN_US);
}

static void Car_RowWaitRunSwitch(uint32_t *last_imu_ms, uint32_t *last_lcd_ms, uint32_t *last_scan_ms)
{
    while (Car_RowMasterPauseActive() != 0U)
    {
        workflow_paused = 1U;
        Car_PWM_OutputUpdateManual();
        Car_RowService(last_imu_ms, last_lcd_ms, last_scan_ms);
        HAL_Delay(10U);
    }

    Car_PWM_OutputNeutral();
    workflow_paused = 0U;
}

static void Car_RowDelayWithSwitch(uint32_t delay_ms,
                                   uint32_t *last_imu_ms,
                                   uint32_t *last_lcd_ms,
                                   uint32_t *last_scan_ms)
{
    uint32_t elapsed = 0U;

    while (elapsed < delay_ms)
    {
        uint32_t start;

        Car_RowWaitRunSwitch(last_imu_ms, last_lcd_ms, last_scan_ms);
        start = HAL_GetTick();
        Car_RowService(last_imu_ms, last_lcd_ms, last_scan_ms);
        HAL_Delay(10U);
        elapsed += HAL_GetTick() - start;
    }
}

static void Car_RowRunForward(uint32_t target_us,
                              uint32_t run_ms,
                              uint32_t *last_imu_ms,
                              uint32_t *last_lcd_ms,
                              uint32_t *last_scan_ms)
{
    uint32_t elapsed = 0U;

    while (elapsed < run_ms)
    {
        uint32_t start;
        uint32_t pulse = WORK_CAR_NEUTRAL_US;

        Car_RowWaitRunSwitch(last_imu_ms, last_lcd_ms, last_scan_ms);

        if (elapsed < WORK_CAR_RAMP_MS)
        {
            if (target_us >= WORK_CAR_NEUTRAL_US)
            {
                pulse = WORK_CAR_NEUTRAL_US +
                        ((target_us - WORK_CAR_NEUTRAL_US) * elapsed) / WORK_CAR_RAMP_MS;
            }
            else
            {
                pulse = WORK_CAR_NEUTRAL_US -
                        ((WORK_CAR_NEUTRAL_US - target_us) * elapsed) / WORK_CAR_RAMP_MS;
            }
        }
        else if (elapsed < WORK_CAR_HOLD_END_MS)
        {
            pulse = target_us;
        }
        else
        {
            uint32_t down_elapsed = elapsed - WORK_CAR_HOLD_END_MS;

            if (target_us >= WORK_CAR_NEUTRAL_US)
            {
                pulse = target_us -
                        ((target_us - WORK_CAR_NEUTRAL_US) * down_elapsed) / WORK_CAR_RAMP_MS;
            }
            else
            {
                pulse = target_us +
                        ((WORK_CAR_NEUTRAL_US - target_us) * down_elapsed) / WORK_CAR_RAMP_MS;
            }
        }

        start = HAL_GetTick();
        Car_PWM_OutputSet(pulse, pulse);
        Car_RowService(last_imu_ms, last_lcd_ms, last_scan_ms);
        HAL_Delay(20U);
        elapsed += HAL_GetTick() - start;
    }

    Car_PWM_OutputNeutral();
}

static void Car_RowRunForwardConstant(uint32_t run_ms,
                                      uint32_t *last_imu_ms,
                                      uint32_t *last_lcd_ms,
                                      uint32_t *last_scan_ms)
{
    uint32_t elapsed = 0U;

    while (elapsed < run_ms)
    {
        uint32_t start;

        Car_RowWaitRunSwitch(last_imu_ms, last_lcd_ms, last_scan_ms);
        start = HAL_GetTick();
        Car_PWM_OutputSet(Car_RowForwardPulseUs(), Car_RowForwardPulseUs());
        Car_RowService(last_imu_ms, last_lcd_ms, last_scan_ms);
        HAL_Delay(20U);
        elapsed += HAL_GetTick() - start;
    }

    Car_PWM_OutputNeutral();
}

static int32_t Car_RowWaitYawValid(uint32_t *last_imu_ms, uint32_t *last_lcd_ms, uint32_t *last_scan_ms)
{
    while (imu_i2c_data_valid == 0U)
    {
        Car_RowWaitRunSwitch(last_imu_ms, last_lcd_ms, last_scan_ms);
        Car_PWM_OutputNeutral();
        Car_RowService(last_imu_ms, last_lcd_ms, last_scan_ms);
        HAL_Delay(10U);
    }

    return imu_yaw_x100;
}

static int32_t Car_RowTurnRight90(int32_t base_yaw_x100,
                                  uint32_t *last_imu_ms,
                                  uint32_t *last_lcd_ms,
                                  uint32_t *last_scan_ms)
{
    int32_t current_yaw_x100 = base_yaw_x100;

    while (Car_RowNormalizeDelta(current_yaw_x100, base_yaw_x100) < CAR_ROW_TURN_DEG_X100)
    {
        Car_RowWaitRunSwitch(last_imu_ms, last_lcd_ms, last_scan_ms);
        Car_PWM_OutputSet(CAR_ROW_TURN_LEFT_REVERSE_US, CAR_ROW_TURN_RIGHT_FAST_US);
        Car_RowService(last_imu_ms, last_lcd_ms, last_scan_ms);
        current_yaw_x100 = imu_yaw_x100;
        HAL_Delay(10U);
    }

    Car_PWM_OutputNeutral();
    return current_yaw_x100;
}

static void Car_RowForwardStep(uint32_t *last_imu_ms,
                               uint32_t *last_lcd_ms,
                               uint32_t *last_scan_ms)
{
    Car_RowRunForwardConstant(CAR_ROW_FORWARD_MS, last_imu_ms, last_lcd_ms, last_scan_ms);
}

static void Car_RowForwardPair(uint32_t *last_imu_ms,
                               uint32_t *last_lcd_ms,
                               uint32_t *last_scan_ms)
{
    Car_RowForwardStep(last_imu_ms, last_lcd_ms, last_scan_ms);
    Car_RowDelayWithSwitch(CAR_ROW_BETWEEN_FORWARD_MS, last_imu_ms, last_lcd_ms, last_scan_ms);
    Car_RowForwardStep(last_imu_ms, last_lcd_ms, last_scan_ms);
    Car_RowDelayWithSwitch(CAR_ROW_BETWEEN_FORWARD_MS, last_imu_ms, last_lcd_ms, last_scan_ms);
}

static uint8_t Car_RowContinueStateValid(void)
{
    return ((HAL_GetTick() - row_continue_rc.last_ms) < RC_TIMEOUT_MS) ? 1U : 0U;
}

static uint8_t Car_RowContinueState(void)
{
    return (row_continue_rc.pulse_us >= WORK_SWITCH_THRESHOLD_US) ? 1U : 0U;
}

static void Car_RowWaitContinueChange(uint32_t *last_imu_ms,
                                      uint32_t *last_lcd_ms,
                                      uint32_t *last_scan_ms)
{
    Car_RowWaitRunSwitch(last_imu_ms, last_lcd_ms, last_scan_ms);
}

static void Car_RowForwardTriple(uint32_t *last_imu_ms,
                                 uint32_t *last_lcd_ms,
                                 uint32_t *last_scan_ms)
{
    Car_RowForwardStep(last_imu_ms, last_lcd_ms, last_scan_ms);
    Car_RowDelayWithSwitch(CAR_ROW_BETWEEN_FORWARD_MS, last_imu_ms, last_lcd_ms, last_scan_ms);
    Car_RowForwardStep(last_imu_ms, last_lcd_ms, last_scan_ms);
    Car_RowDelayWithSwitch(CAR_ROW_BETWEEN_FORWARD_MS, last_imu_ms, last_lcd_ms, last_scan_ms);
    Car_RowForwardStep(last_imu_ms, last_lcd_ms, last_scan_ms);
}

static void Car_RowTurnPairAndExitForward(uint32_t *last_imu_ms,
                                          uint32_t *last_lcd_ms,
                                          uint32_t *last_scan_ms)
{
    int32_t base_yaw_x100;

    Car_RowDelayWithSwitch(CAR_ROW_BEFORE_TURN_MS, last_imu_ms, last_lcd_ms, last_scan_ms);
    base_yaw_x100 = Car_RowWaitYawValid(last_imu_ms, last_lcd_ms, last_scan_ms);
    (void)Car_RowTurnRight90(base_yaw_x100, last_imu_ms, last_lcd_ms, last_scan_ms);

    Car_RowDelayWithSwitch(CAR_ROW_AFTER_TURN_MS, last_imu_ms, last_lcd_ms, last_scan_ms);
    base_yaw_x100 = Car_RowWaitYawValid(last_imu_ms, last_lcd_ms, last_scan_ms);
    (void)Car_RowTurnRight90(base_yaw_x100, last_imu_ms, last_lcd_ms, last_scan_ms);

    Car_RowDelayWithSwitch(CAR_ROW_AFTER_TURN_MS, last_imu_ms, last_lcd_ms, last_scan_ms);
    Car_RowForwardStep(last_imu_ms, last_lcd_ms, last_scan_ms);
}

static void Car_RowFullCycle(uint32_t *last_imu_ms,
                             uint32_t *last_lcd_ms,
                             uint32_t *last_scan_ms)
{
    Car_RowForwardPair(last_imu_ms, last_lcd_ms, last_scan_ms);
    Car_RowForwardStep(last_imu_ms, last_lcd_ms, last_scan_ms);
    Car_RowTurnPairAndExitForward(last_imu_ms, last_lcd_ms, last_scan_ms);
}

static void Car_RowFinalCycle(uint32_t *last_imu_ms,
                              uint32_t *last_lcd_ms,
                              uint32_t *last_scan_ms)
{
    Car_RowForwardPair(last_imu_ms, last_lcd_ms, last_scan_ms);
    Car_RowForwardStep(last_imu_ms, last_lcd_ms, last_scan_ms);
}
#endif

void SysTick_Handler(void)
{
    HAL_IncTick();
#if defined(APP_USE_FREERTOS) && (APP_USE_FREERTOS != 0)
    if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED)
    {
        xPortSysTickHandler();
    }
#endif
}

#if defined(APP_USE_FREERTOS) && (APP_USE_FREERTOS != 0)
void SVC_Handler(void)
{
    vPortSVCHandler();
}

void PendSV_Handler(void)
{
    xPortPendSVHandler();
}
#endif

void TIM2_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&htim2);
}

void TIM1_CC_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&htim1);
}

#if defined(CAR_GPIO_MANUAL_TEST_ONLY) && (CAR_GPIO_MANUAL_TEST_ONLY != 0)
void TIM5_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&manual_seed_pump_tim);
}

void EXTI15_10_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(MANUAL_SEED_PUMP_INPUT_PIN);
}
#endif

void TIM3_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&htim3);
}

#if defined(CAR_GPIO_MANUAL_TEST_ONLY) && (CAR_GPIO_MANUAL_TEST_ONLY != 0)
void TIM4_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&manual_servo13_tim);
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM4)
    {
        if (manual_servo13_active == 0U)
        {
            manual_servo13_low = 0U;
            HAL_GPIO_WritePin(MANUAL_SERVO13_PUL_GPIO_PORT,
                              MANUAL_SERVO13_PUL_PIN,
                              GPIO_PIN_SET);
            __HAL_TIM_SET_AUTORELOAD(&manual_servo13_tim, MANUAL_SERVO13_PULSE_LOW_US - 1U);
            __HAL_TIM_SET_COUNTER(&manual_servo13_tim, 0U);
            return;
        }

        if (manual_servo13_low == 0U)
        {
            manual_servo13_low = 1U;
            HAL_GPIO_WritePin(MANUAL_SERVO13_PUL_GPIO_PORT,
                              MANUAL_SERVO13_PUL_PIN,
                              GPIO_PIN_RESET);
            __HAL_TIM_SET_AUTORELOAD(&manual_servo13_tim, MANUAL_SERVO13_PULSE_LOW_US - 1U);
        }
        else
        {
            manual_servo13_low = 0U;
            HAL_GPIO_WritePin(MANUAL_SERVO13_PUL_GPIO_PORT,
                              MANUAL_SERVO13_PUL_PIN,
                              GPIO_PIN_SET);
            manual_servo13_pulse_count++;
            __HAL_TIM_SET_AUTORELOAD(&manual_servo13_tim, MANUAL_SERVO13_PULSE_RELEASE_US - 1U);
        }
        __HAL_TIM_SET_COUNTER(&manual_servo13_tim, 0U);
        return;
    }

    if (htim->Instance != TIM3)
    {
        return;
    }

    if (manual_mobile_stepper_active == 0U)
    {
        manual_mobile_stepper_low = 0U;
        HAL_GPIO_WritePin(MOBILE_STEP_GPIO_PORT, MOBILE_STEP_PIN, GPIO_PIN_SET);
        __HAL_TIM_SET_AUTORELOAD(&htim3, MANUAL_MOBILE_STEP_HIGH_US - 1U);
        __HAL_TIM_SET_COUNTER(&htim3, 0U);
        return;
    }

    if (manual_mobile_stepper_low == 0U)
    {
        manual_mobile_stepper_low = 1U;
        HAL_GPIO_WritePin(MOBILE_STEP_GPIO_PORT, MOBILE_STEP_PIN, GPIO_PIN_RESET);
        __HAL_TIM_SET_AUTORELOAD(&htim3, MANUAL_MOBILE_STEP_HIGH_US - 1U);
    }
    else
    {
        manual_mobile_stepper_low = 0U;
        HAL_GPIO_WritePin(MOBILE_STEP_GPIO_PORT, MOBILE_STEP_PIN, GPIO_PIN_SET);
        __HAL_TIM_SET_AUTORELOAD(&htim3,
                                 MANUAL_MOBILE_STEP_PERIOD_US - MANUAL_MOBILE_STEP_HIGH_US - 1U);
    }
    __HAL_TIM_SET_COUNTER(&htim3, 0U);
}
#endif

void TIM8_CC_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&htim8);
}

void TIM8_BRK_TIM12_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&htim12);
}

void EXTI9_5_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(MANUAL_MODE_INPUT_PIN);
    HAL_GPIO_EXTI_IRQHandler(ULTRASONIC_ECHO_PIN);
}

void USART1_IRQHandler(void)
{
    HAL_UART_IRQHandler(&huart1);
}

void USART2_IRQHandler(void)
{
    HAL_UART_IRQHandler(&huart2);
}

void USART3_IRQHandler(void)
{
    HAL_UART_IRQHandler(&huart3);
}

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM2 && htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1)
    {
        Car_RC_ProcessCapture(htim, TIM_CHANNEL_1, &car_rc_ch1);
    }
    else if (htim->Instance == TIM8 && htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1)
    {
        Car_RC_ProcessCapture(htim, TIM_CHANNEL_1, &car_rc_ch2);
    }
    else if (htim->Instance == TIM8 && htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2)
    {
        Car_RC_ProcessCapture(htim, TIM_CHANNEL_2, &work_switch_rc);
    }
#if defined(CAR_GPIO_MANUAL_TEST_ONLY) && (CAR_GPIO_MANUAL_TEST_ONLY != 0)
    else if (htim->Instance == TIM5 && htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2)
    {
        Car_RC_ProcessCapture(htim, TIM_CHANNEL_2, &manual_seed_pump_rc);
    }
#endif
    else if (htim->Instance == TIM12 && htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1)
    {
        Car_RC_ProcessCapture(htim, TIM_CHANNEL_1, &row_continue_rc);
    }
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
#if defined(CAR_GPIO_MANUAL_TEST_ONLY) && (CAR_GPIO_MANUAL_TEST_ONLY != 0)
    if (GPIO_Pin == MANUAL_MODE_INPUT_PIN)
    {
        uint32_t now_cycles = DWT->CYCCNT;
        uint32_t cycles_per_us = HAL_RCC_GetHCLKFreq() / 1000000U;

        speed_rc.edge_count++;
        speed_rc.last_capture = now_cycles;
        if (HAL_GPIO_ReadPin(MANUAL_MODE_INPUT_GPIO_PORT,
                             MANUAL_MODE_INPUT_PIN) == GPIO_PIN_SET)
        {
            manual_mode_rise_cycles = now_cycles;
            manual_mode_wait_fall = 1U;
        }
        else if (manual_mode_wait_fall != 0U && cycles_per_us != 0U)
        {
            uint32_t width = (now_cycles - manual_mode_rise_cycles) / cycles_per_us;
            manual_mode_wait_fall = 0U;
            speed_rc.raw_width_us = width;
            if (width >= RC_MIN_US && width <= RC_MAX_US)
            {
                speed_rc.pulse_us = width;
                speed_rc.last_ms = HAL_GetTick();
                speed_rc.capture_count++;
            }
        }
        return;
    }

    if (GPIO_Pin == MANUAL_SEED_PUMP_INPUT_PIN)
    {
        uint32_t now_cycles = DWT->CYCCNT;
        uint32_t cycles_per_us = HAL_RCC_GetHCLKFreq() / 1000000U;

        manual_seed_pump_rc.edge_count++;
        manual_seed_pump_rc.last_capture = now_cycles;
        if (HAL_GPIO_ReadPin(MANUAL_SEED_PUMP_INPUT_GPIO_PORT,
                             MANUAL_SEED_PUMP_INPUT_PIN) == GPIO_PIN_SET)
        {
            manual_seed_pump_rise_cycles = now_cycles;
            manual_seed_pump_wait_fall = 1U;
        }
        else if (manual_seed_pump_wait_fall != 0U && cycles_per_us != 0U)
        {
            uint32_t width = (now_cycles - manual_seed_pump_rise_cycles) / cycles_per_us;
            manual_seed_pump_wait_fall = 0U;
            manual_seed_pump_rc.raw_width_us = width;
            if (width >= RC_MIN_US && width <= RC_MAX_US)
            {
                manual_seed_pump_rc.pulse_us = width;
                manual_seed_pump_rc.last_ms = HAL_GetTick();
                manual_seed_pump_rc.capture_count++;
            }
        }
        return;
    }
#endif

    Ultrasonic_ProcessEcho(GPIO_Pin);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1)
    {
#if defined(UART1_ECHO_ONLY) && (UART1_ECHO_ONLY != 0)
        UART1_DiagPushByte(uart1_diag_rx_byte);
        UART1_DiagStartReceive();
#else
        UART1_PushByte(uart1_rx_byte);
        UART2_QueueEchoByte(uart1_rx_byte);
        USART1_StartReceive();
#endif
    }
    else if (huart->Instance == USART2)
    {
        UART2_PushByte(uart2_rx_byte);
        USART2_StartReceive();
    }
    else if (huart->Instance == USART3)
    {
        UART3_PushByte(uart3_rx_byte);
        UART3_QueueEchoByte(uart3_rx_byte);
        USART3_StartReceive();
    }
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1)
    {
#if defined(UART1_ECHO_ONLY) && (UART1_ECHO_ONLY != 0)
        uart1_diag_error_count++;
        uart1_diag_last_error = HAL_UART_GetError(huart);
        UART1_DiagStartReceive();
#else
        uart1_error_count++;
        uart1_last_error = HAL_UART_GetError(huart);
        USART1_StartReceive();
#endif
    }
    else if (huart->Instance == USART2)
    {
        uart2_error_count++;
        uart2_last_error = HAL_UART_GetError(huart);
        USART2_StartReceive();
    }
    else if (huart->Instance == USART3)
    {
        uart3_error_count++;
        uart3_last_error = HAL_UART_GetError(huart);
        USART3_StartReceive();
    }
}

int main(void)
{
#if defined(PB8_TOGGLE_TEST_ONLY) && (PB8_TOGGLE_TEST_ONLY != 0)
    HAL_Init();
    SystemClock_Config();
    HAL_Delay(100U);

    PB8_ToggleTest_GPIO_Init();

    while (1)
    {
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_SET);
        HAL_Delay(1000U);
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_RESET);
        HAL_Delay(1000U);
    }
#elif defined(SERVO51_PB0_DIAG_TEST_ONLY) && (SERVO51_PB0_DIAG_TEST_ONLY != 0)
    HAL_Init();
    SystemClock_Config();
    HAL_Delay(100U);

    DWT_Delay_Init();
    Servo51PB0Diag_GPIO_Init();
    HAL_Delay(SERVO51_DIAG_START_MS);

    while (1)
    {
        Servo51PB0Diag_OutputStepPulse();
    }
#elif defined(CAR_PIN_DIAG_TEST_ONLY) && (CAR_PIN_DIAG_TEST_ONLY != 0)
    HAL_Init();
    SystemClock_Config();
    HAL_Delay(100U);

    CarPinDiag_GPIO_Init();
    CarPinDiag_AllStop();
    HAL_Delay(CAR_PIN_DIAG_START_MS);

    while (1)
    {
        CarPinDiag_TestPin(GPIOA, GPIO_PIN_7, 1U);
        CarPinDiag_TestPin(GPIOB, GPIO_PIN_1, 2U);
        CarPinDiag_TestPin(GPIOB, GPIO_PIN_12, 3U);
        CarPinDiag_TestPin(GPIOA, GPIO_PIN_6, 4U);
    }
#elif defined(CAR_LEFT_DIAG_TEST_ONLY) && (CAR_LEFT_DIAG_TEST_ONLY != 0)
    HAL_Init();
    SystemClock_Config();
    HAL_Delay(100U);

    CarLeftDiag_GPIO_Init();
    CarLeftDiag_StopHigh();
    HAL_Delay(CAR_LEFT_DIAG_START_MS);

    while (1)
    {
        CarLeftDiag_Test(GPIO_PIN_RESET, GPIO_PIN_SET, 1U);
        CarLeftDiag_Test(GPIO_PIN_SET, GPIO_PIN_RESET, 2U);
        CarLeftDiag_Test(GPIO_PIN_RESET, GPIO_PIN_RESET, 3U);
        CarLeftDiag_Test(GPIO_PIN_SET, GPIO_PIN_SET, 4U);
    }
#elif defined(CAR_TURN_TEST_ONLY) && (CAR_TURN_TEST_ONLY != 0)
    HAL_Init();
    SystemClock_Config();
    HAL_Delay(100U);

    LED_Init();
    Car_Motor_GPIO_Init();
    CarTurnTest_Stop();
    HAL_Delay(CAR_TURN_TEST_START_MS);

    while (1)
    {
        CarTurnTest_Right();
        HAL_Delay(CAR_TURN_TEST_ACTIVE_MS);
        CarTurnTest_Stop();
        HAL_Delay(CAR_TURN_TEST_STOP_MS);

        CarTurnTest_Left();
        HAL_Delay(CAR_TURN_TEST_ACTIVE_MS);
        CarTurnTest_Stop();
        HAL_Delay(CAR_TURN_TEST_STOP_MS);
    }
#elif defined(LINEAR_ACTUATOR_TEST_ONLY) && (LINEAR_ACTUATOR_TEST_ONLY != 0)
    HAL_Init();
    SystemClock_Config();
    HAL_Delay(100U);

    LinearActuator_GPIO_Init();
    Actuator_Stop();

    while (1)
    {
        Actuator_Extend();
        HAL_Delay(3000U);
        Actuator_Stop();
        HAL_Delay(1000U);

        Actuator_Retract();
        HAL_Delay(3000U);
        Actuator_Stop();
        HAL_Delay(1000U);
    }
#elif defined(LINEAR_ACTUATOR_PG4_PG5_TEST_ONLY) && (LINEAR_ACTUATOR_PG4_PG5_TEST_ONLY != 0)
    HAL_Init();
    SystemClock_Config();
    HAL_Delay(100U);

    LinearPG45_GPIO_Init();
    LinearPG45_Stop();

    while (1)
    {
        LinearPG45_Extend();
        HAL_Delay(LINEAR_PG45_RUN_MS);
        LinearPG45_Stop();
        HAL_Delay(LINEAR_PG45_STOP_MS);

        LinearPG45_Retract();
        HAL_Delay(LINEAR_PG45_RUN_MS);
        LinearPG45_Stop();
        HAL_Delay(LINEAR_PG45_STOP_MS);
    }
#elif defined(CAR_GPIO_MANUAL_TEST_ONLY) && (CAR_GPIO_MANUAL_TEST_ONLY != 0)
    uint32_t last_control_ms;
    uint8_t auto_requested = 0U;

    HAL_Init();
    SystemClock_Config();
    HAL_Delay(100U);

    ManualCar_WaterPumpPE5_Init();
    LED_Init();
    DWT_Delay_Init();
    Actuator_GPIO_Init();
    Actuator_AllStop();
    Car_RC1_InputCapture_Init();
    Car_RC2_InputCapture_Init();
    Car_RowContinue_InputCapture_Init();
    Car_Speed_InputCapture_Init();
    ManualCar_SeedPumpPF11_InputCapture_Init();
    ManualCar_Servo13PB0_Init();
    ManualCar_MobileStepper_Init();
    ManualCar_Cover4Pin_Init();
    Car_Motor_GPIO_Init();
    Car_Motor_SetDefaultRunDuty(MANUAL_AUTO_SPEED_DEFAULT_PERCENT);
    manual_auto_speed_adjust_armed = 1U;
    Car_AutoMode_Start();
    Car_RC_ChannelInit(&work_switch_rc);
    __HAL_TIM_SET_CAPTUREPOLARITY(&htim8, TIM_CHANNEL_2, TIM_INPUTCHANNELPOLARITY_RISING);
    HAL_TIM_IC_Start_IT(&htim8, TIM_CHANNEL_2);
    Car_StartRowContinueInput();
    Car_StartSpeedInput();
    Car_Motor_Stop();
    I2C1_Init();

    lcd_init();
    lcd_display_dir(0);
    lcd_display_on();
    LCD_BL(1);
    LCD_UI_Init();
    IMU_I2C_Scan();
    LCD_UI_Update();

    manual_snake_last_imu_ms = HAL_GetTick();
    manual_snake_last_lcd_ms = manual_snake_last_imu_ms;
    manual_snake_last_scan_ms = manual_snake_last_imu_ms;
    last_control_ms = manual_snake_last_imu_ms;
    ManualCar_AllStop();

    while (1)
    {
        uint32_t now = HAL_GetTick();

        auto_requested = ManualCar_ModeAutoRequested(auto_requested);
        if (auto_requested != 0U)
        {
            if (manual_snake_resume_seed == 0U)
            {
                ManualCar_AllStop();
            }
            else
            {
                ManualCar_AllStop();
                ManualCar_RunSnakeWorkflow();
            }
        }
        else
        {
            manual_snake_abort_requested = 0U;
            if ((manual_snake_resume_seed == 0U) &&
                (manual_snake_resume_phase == MANUAL_SNAKE_PHASE_DONE))
            {
                manual_snake_resume_seed = 1U;
                manual_snake_resume_phase = MANUAL_SNAKE_PHASE_DRILL;
            }
            if ((now - last_control_ms) >= 10U)
            {
                ManualControlMode_t control_mode;

                last_control_ms = now;
                control_mode = ManualCar_ReadControlMode();
                if (control_mode == MANUAL_CONTROL_MODE_IDLE)
                {
                    ManualCar_AllStop();
                }
                else if (control_mode == MANUAL_CONTROL_MODE_SEEDLING)
                {
                    Car_AutoMode_Update();
                    ManualCar_CoverOnlyStop();
                    ManualCar_SolarPanelStop();
                    ManualCar_ManualDrillControlUpdate();
                    ManualCar_ManualSeedPumpControlUpdate();
                }
                else
                {
                    /* PE6 high selects the manual cover/solar group.
                     * Keep the car, drill, seedling feed, and water pump safe,
                     * while PB14 controls cover soil, PF11 controls the solar
                     * panel actuator, and PC6 still adjusts auto speed.
                     */
                    ManualCar_CarStopAuto();
                    Pump_Off();
                    ManualCar_MobileStopAuto();
                    ManualCar_WaterPumpSet(0U);
                    ManualCar_Servo51Stop();
                    ManualCar_ManualCoverPB14Update();
                    ManualCar_ManualSolarPF11Update();
                    ManualCar_AutoSpeedConfigUpdate();
                }
            }
        }

        ManualCar_BackgroundService();
        HAL_Delay(5U);
    }
#elif defined(CAR_MANUAL_AUTO_TEST_ONLY) && (CAR_MANUAL_AUTO_TEST_ONLY != 0)
    uint32_t last_imu_ms = 0U;
    uint32_t last_lcd_ms = 0U;
    uint32_t last_scan_ms = 0U;
    uint32_t car_step_index;

    HAL_Init();
    SystemClock_Config();
    HAL_Delay(100);

    LED_Init();
    Actuator_GPIO_Init();
    Actuator_AllStop();
    Seedling_GPIO_Init();
    Mobile_GPIO_Init();
    Seedling_Stop();
    Mobile_Stop();
    DWT_Delay_Init();
    Ultrasonic_GPIO_Init();

    lcd_init();
    lcd_display_dir(0);
    lcd_display_on();
    LCD_BL(1);

    Car_RC1_InputCapture_Init();
    Car_RC2_InputCapture_Init();
    Car_RowContinue_InputCapture_Init();
    Car_Speed_InputCapture_Init();
    Car_PWM_Output_Init();
    I2C1_Init();
    USART1_UART_Init();
    USART2_UART_Init();
    LCD_UI_Init();
    USART1_StartReceive();
    USART2_StartReceive();
    Car_Start();
    Car_StartRowContinueInput();
    Car_StartSpeedInput();
    Car_PWM_OutputNeutral();
    IMU_I2C_Scan();
    LCD_UI_Update();

    Car_RowWaitContinueChange(&last_imu_ms, &last_lcd_ms, &last_scan_ms);
    for (car_step_index = 0U; car_step_index < CAR_ROW_TOTAL_STEPS; car_step_index++)
    {
        workflow_step_index = (uint16_t)(car_step_index + 1U);
        Car_RowForwardStep(&last_imu_ms, &last_lcd_ms, &last_scan_ms);
        Car_RowDelayWithSwitch(CAR_ROW_BETWEEN_FORWARD_MS,
                               &last_imu_ms,
                               &last_lcd_ms,
                               &last_scan_ms);
    }
    Car_PWM_OutputNeutral();

    while (1)
    {
        Car_RowWaitRunSwitch(&last_imu_ms, &last_lcd_ms, &last_scan_ms);
        Car_RowService(&last_imu_ms, &last_lcd_ms, &last_scan_ms);
        Car_PWM_OutputNeutral();
        HAL_Delay(10U);
    }
#elif defined(CAR_IMU_ROW_TEST) && (CAR_IMU_ROW_TEST != 0)
    HAL_Init();
    Pump_EarlyOff_GPIO_Init();
    SystemClock_Config();
    Pump_EarlyOff_GPIO_Init();

    LED_Init();
    Actuator_GPIO_Init();
    Actuator_AllStop();
    Seedling_GPIO_Init();
    Mobile_GPIO_Init();
    Seedling_Stop();
    Mobile_Stop();
    DWT_Delay_Init();
    Ultrasonic_GPIO_Init();

    lcd_init();
    lcd_display_dir(0);
    lcd_display_on();
    LCD_BL(1);

    Car_RC1_InputCapture_Init();
    Car_RC2_InputCapture_Init();
    Car_RowContinue_InputCapture_Init();
    Car_Speed_InputCapture_Init();
    Car_PWM_Output_Init();
    I2C1_Init();
    USART1_UART_Init();
    USART2_UART_Init();
    LCD_UI_Init();
    USART1_StartReceive();
    USART2_StartReceive();
    Car_Start();
    Car_StartRowContinueInput();
    Car_StartSpeedInput();
    Car_PWM_OutputNeutral();
    IMU_I2C_Scan();
    LCD_UI_Update();

    Workflow_RunOnce();

    Actuator_AllStop();
    Seedling_Stop();
    Mobile_Stop();
    Car_PWM_OutputNeutral();

    while (1)
    {
        App_ServiceTasks();
        if (App_WorkflowPaused() != 0U)
        {
            Car_PWM_OutputUpdateManual();
        }
        else
        {
            Car_PWM_OutputNeutral();
        }
        HAL_Delay(10U);
    }
#elif defined(IMU_TEST_ONLY) && (IMU_TEST_ONLY != 0)
    uint32_t last_imu_ms = 0U;
    uint32_t last_lcd_ms = 0U;
    uint32_t last_scan_ms = 0U;

    HAL_Init();
    SystemClock_Config();
    HAL_Delay(100);

    LED_Init();
    Actuator_GPIO_Init();
    Actuator_AllStop();
    Seedling_GPIO_Init();
    Mobile_GPIO_Init();
    Seedling_Stop();
    Mobile_Stop();
    DWT_Delay_Init();

    lcd_init();
    lcd_display_dir(0);
    lcd_display_on();
    LCD_BL(1);

    I2C1_Init();
    USART2_UART_Init();
    USART3_UART_Init();
    LCD_UI_Init();
    USART2_StartReceive();
    USART3_StartReceive();
    LCD_UI_Update();
    last_lcd_ms = HAL_GetTick();

    while (1)
    {
        uint32_t now = HAL_GetTick();

        USART2_ServiceEcho();
        USART3_ServiceEcho();

        if ((now - last_imu_ms) >= 100U)
        {
            last_imu_ms = now;
            IMU_I2C_ReadAngles();
        }

        if (imu_i2c_data_valid == 0U && (now - last_scan_ms) >= 1000U)
        {
            last_scan_ms = now;
            IMU_I2C_Scan();
        }

        if ((now - last_lcd_ms) >= 200U)
        {
            last_lcd_ms = now;
            LCD_UI_Update();
        }

        HAL_Delay(10U);
    }
#elif defined(CAR_TEST_ONLY) && (CAR_TEST_ONLY != 0)
    const uint32_t reverse_us = WORK_CAR_NEUTRAL_US -
                                (WORK_CAR_FORWARD_US - WORK_CAR_NEUTRAL_US);

    HAL_Init();
    SystemClock_Config();
    HAL_Delay(100);

    LED_Init();
    Actuator_GPIO_Init();
    Actuator_AllStop();
    Seedling_GPIO_Init();
    Mobile_GPIO_Init();
    Seedling_Stop();
    Mobile_Stop();
    Car_RC1_InputCapture_Init();
    Car_RC2_InputCapture_Init();
    Car_RowContinue_InputCapture_Init();
    Car_Speed_InputCapture_Init();
    Car_PWM_Output_Init();
    Car_Start();
    Car_PWM_OutputNeutral();

    while (1)
    {
        HAL_GPIO_WritePin(GPIOF, GPIO_PIN_9, GPIO_PIN_RESET);
        Car_TestRun(WORK_CAR_FORWARD_US, WORK_CAR_FORWARD_MS);
        HAL_GPIO_WritePin(GPIOF, GPIO_PIN_9, GPIO_PIN_SET);
        Car_PWM_OutputNeutral();
        HAL_Delay(2000U);

        HAL_GPIO_WritePin(GPIOF, GPIO_PIN_10, GPIO_PIN_RESET);
        Car_TestRun(reverse_us, WORK_CAR_FORWARD_MS);
        HAL_GPIO_WritePin(GPIOF, GPIO_PIN_10, GPIO_PIN_SET);
        Car_PWM_OutputNeutral();
        HAL_Delay(2000U);
    }
#elif defined(COVER_TEST_ONLY) && (COVER_TEST_ONLY != 0)
    HAL_Init();
    SystemClock_Config();
    HAL_Delay(100);

    LED_Init();
    Actuator_GPIO_Init();
    Actuator_AllStop();

    while (1)
    {
        HAL_GPIO_WritePin(GPIOF, GPIO_PIN_9, GPIO_PIN_RESET);
        Cover_Forward();
        HAL_Delay(10000U);

        HAL_GPIO_WritePin(GPIOF, GPIO_PIN_9, GPIO_PIN_SET);
        Cover_Stop();
        HAL_Delay(1000U);

        HAL_GPIO_WritePin(GPIOF, GPIO_PIN_10, GPIO_PIN_RESET);
        Cover_Reverse();
        HAL_Delay(10000U);

        HAL_GPIO_WritePin(GPIOF, GPIO_PIN_10, GPIO_PIN_SET);
        Cover_Stop();
        HAL_Delay(1000U);
    }
#elif defined(PUMP_TEST_ONLY) && (PUMP_TEST_ONLY != 0)
    HAL_Init();
    SystemClock_Config();
    HAL_Delay(100);

    LED_Init();
    Actuator_GPIO_Init();
    Actuator_AllStop();

    while (1)
    {
        HAL_GPIO_WritePin(GPIOF, GPIO_PIN_9, GPIO_PIN_RESET);
        Pump_On();
        HAL_Delay(10000U);

        HAL_GPIO_WritePin(GPIOF, GPIO_PIN_9, GPIO_PIN_SET);
        Pump_Off();
        HAL_Delay(2000U);
    }
#elif defined(DRILL_TEST_ONLY) && (DRILL_TEST_ONLY != 0)
    HAL_Init();
    SystemClock_Config();
    HAL_Delay(100);

    LED_Init();
    DWT_Delay_Init();
    Actuator_GPIO_Init();
    Actuator_AllStop();

    while (1)
    {
        HAL_GPIO_WritePin(GPIOF, GPIO_PIN_9, GPIO_PIN_RESET);
        Drill_TestPwm(1000U, 5000U);
        HAL_GPIO_WritePin(GPIOF, GPIO_PIN_9, GPIO_PIN_SET);
        Drill_Off();
        HAL_Delay(2000U);
    }
#elif defined(DRILL_ROD_TEST_ONLY) && (DRILL_ROD_TEST_ONLY != 0)
    HAL_Init();
    SystemClock_Config();
    HAL_Delay(100);

    LED_Init();
    DWT_Delay_Init();
    Actuator_GPIO_Init();
    Actuator_AllStop();

    while (1)
    {
        Drill_RodLoopTestCycle();
    }
#elif defined(CONVEYOR_TEST_ONLY) && (CONVEYOR_TEST_ONLY != 0)
    HAL_Init();
    SystemClock_Config();
    HAL_Delay(100);

    LED_Init();
    Actuator_GPIO_Init();
    Actuator_AllStop();

    while (1)
    {
        HAL_GPIO_WritePin(GPIOF, GPIO_PIN_9, GPIO_PIN_RESET);
        Conveyor_Forward();
        HAL_Delay(1450U);

        HAL_GPIO_WritePin(GPIOF, GPIO_PIN_9, GPIO_PIN_SET);
        Conveyor_Stop();
        HAL_Delay(1000U);

        HAL_GPIO_WritePin(GPIOF, GPIO_PIN_9, GPIO_PIN_RESET);
        Conveyor_Forward();
        HAL_Delay(500U);

        HAL_GPIO_WritePin(GPIOF, GPIO_PIN_9, GPIO_PIN_SET);
        Conveyor_Stop();
        HAL_Delay(1000U);

        HAL_GPIO_WritePin(GPIOF, GPIO_PIN_9, GPIO_PIN_RESET);
        Conveyor_Forward();
        HAL_Delay(500U);

        HAL_GPIO_WritePin(GPIOF, GPIO_PIN_9, GPIO_PIN_SET);
        Conveyor_Stop();
        HAL_Delay(1000U);

        HAL_GPIO_WritePin(GPIOF, GPIO_PIN_10, GPIO_PIN_RESET);
        Conveyor_Reverse();
        HAL_Delay(2500U);

        HAL_GPIO_WritePin(GPIOF, GPIO_PIN_10, GPIO_PIN_SET);
        Conveyor_Stop();
        HAL_Delay(1000U);
    }
#elif defined(DUAL_STEPPERS_TEST) && (DUAL_STEPPERS_TEST != 0)
    HAL_Init();
    SystemClock_Config();
    HAL_Delay(100);

    LED_Init();
    DWT_Delay_Init();
    Seedling_GPIO_Init();
    Mobile_GPIO_Init();
    Seedling_Stop();
    HAL_GPIO_WritePin(MOBILE_STEP_GPIO_PORT, MOBILE_STEP_PIN, GPIO_PIN_SET);

    while (1)
    {
        Dual_SteppersService();
    }
#elif defined(SEEDLING_TEST_ONLY) && (SEEDLING_TEST_ONLY != 0)
    HAL_Init();
    SystemClock_Config();
    HAL_Delay(100);

    LED_Init();
    DWT_Delay_Init();
    Actuator_GPIO_Init();
    Actuator_AllStop();
    Seedling_GPIO_Init();
    Seedling_Stop();

    while (1)
    {
        HAL_GPIO_WritePin(GPIOF, GPIO_PIN_10, GPIO_PIN_RESET);
        Seedling_TestRun(REVERSE, 30000U, SEEDLING_TESTED_FREQ_HZ);
        HAL_GPIO_WritePin(GPIOF, GPIO_PIN_10, GPIO_PIN_SET);
        HAL_Delay(2000U);
    }
#elif defined(MOBILE_TEST_ONLY) && (MOBILE_TEST_ONLY != 0)
    HAL_Init();
    SystemClock_Config();
    HAL_Delay(100);

    LED_Init();
    DWT_Delay_Init();
    Actuator_GPIO_Init();
    Actuator_AllStop();
    Mobile_GPIO_Init();
    HAL_GPIO_WritePin(MOBILE_STEP_GPIO_PORT, MOBILE_STEP_PIN, GPIO_PIN_SET);
    USART2_UART_Init();
    huart2.Init.BaudRate = 115200U;
    if (HAL_UART_Init(&huart2) != HAL_OK) Error_Handler();
    Mobile_Stop();
    Pump_Off();
    HAL_GPIO_WritePin(MOBILE_STEP_GPIO_PORT, MOBILE_STEP_PIN, GPIO_PIN_SET);
    Mobile_TestPrint("MOBILE TB6600 TEST PC11=PUL- PC12=DIR- active-low 2400Hz\r\n");

    while (1)
    {
        HAL_GPIO_WritePin(GPIOF, GPIO_PIN_9, GPIO_PIN_RESET);
        Mobile_TestRunDrillEvent(FORWARD, 8000U, MOBILE_TESTED_FREQ_HZ, 5000U, 1U);
        HAL_GPIO_WritePin(GPIOF, GPIO_PIN_9, GPIO_PIN_SET);
        HAL_Delay(2000U);

        HAL_GPIO_WritePin(GPIOF, GPIO_PIN_10, GPIO_PIN_RESET);
        Mobile_TestRunDrillEvent(REVERSE, 8000U, MOBILE_TESTED_FREQ_HZ, 4000U, 0U);
        HAL_GPIO_WritePin(GPIOF, GPIO_PIN_10, GPIO_PIN_SET);
    }
#elif defined(UART1_ECHO_ONLY) && (UART1_ECHO_ONLY != 0)
    uint8_t data;
    char line[96];
    uint16_t line_len = 0U;
    uint32_t last_rx_ms = 0U;
    uint32_t last_tx_ms = 0U;

    HAL_Init();
    SystemClock_Config();
    HAL_Delay(100);

    LED_Init();
    USART1_UART_Init();
    USART2_UART_Init();
    huart2.Init.BaudRate = 115200U;
    if (HAL_UART_Init(&huart2) != HAL_OK) Error_Handler();
    UART1_DiagStartReceive();

    while (1)
    {
        while (UART1_DiagPopByte(&data) != 0U)
        {
            HAL_GPIO_TogglePin(GPIOF, GPIO_PIN_9);
            last_rx_ms = HAL_GetTick();

            if (data == '\r' || data == '\n')
            {
                if (line_len != 0U)
                {
                    line[line_len] = '\0';
                    UART_DiagPrint(line);
                    UART_DiagPrint("\r\n");
                    line_len = 0U;
                }
            }
            else if (data >= 32U && data <= 126U)
            {
                if (line_len < (sizeof(line) - 1U))
                {
                    line[line_len++] = (char)data;
                }
                else
                {
                    line_len = 0U;
                }
            }
        }

        if (line_len != 0U && (HAL_GetTick() - last_rx_ms) >= 300U)
        {
            line[line_len] = '\0';
            UART_DiagPrint(line);
            UART_DiagPrint("\r\n");
            line_len = 0U;
        }

        if ((HAL_GetTick() - last_tx_ms) >= 1000U)
        {
            last_tx_ms = HAL_GetTick();
            HAL_GPIO_TogglePin(GPIOF, GPIO_PIN_10);
        }
    }
#elif defined(MONITOR_ONLY) && (MONITOR_ONLY != 0)
    uint32_t last_imu_ms = 0U;
    uint32_t last_lcd_ms = 0U;
    uint32_t last_ultrasonic_ms = 0U;
    uint32_t last_scan_ms = 0U;

    HAL_Init();
    SystemClock_Config();
    HAL_Delay(100);

    LED_Init();
    Actuator_GPIO_Init();
    Actuator_AllStop();
    Seedling_GPIO_Init();
    Mobile_GPIO_Init();
    DWT_Delay_Init();
    Ultrasonic_GPIO_Init();

    lcd_init();
    lcd_display_dir(0);
    lcd_display_on();
    LCD_BL(1);

    Car_RC1_InputCapture_Init();
    Car_RC2_InputCapture_Init();
    Car_RowContinue_InputCapture_Init();
    Car_Speed_InputCapture_Init();
    Car_PWM_Output_Init();
    I2C1_Init();
    USART1_UART_Init();
    USART2_UART_Init();
    USART3_UART_Init();
    LCD_UI_Init();
    Car_Start();
    Car_StartRowContinueInput();
    Car_StartSpeedInput();
    Car_PWM_OutputNeutral();
    USART1_StartReceive();
    USART2_StartReceive();
    USART3_StartReceive();
    IMU_I2C_Scan();
    LCD_UI_Update();

    while (1)
    {
        uint32_t now = HAL_GetTick();

        USART1_ServiceEcho();
        USART2_ServiceEcho();
        USART3_ServiceEcho();
        Car_PWM_OutputNeutral();

        if ((now - last_imu_ms) >= 100U)
        {
            last_imu_ms = now;
            IMU_I2C_ReadAngles();
        }

        if ((now - last_ultrasonic_ms) >= ULTRASONIC_PERIOD_MS)
        {
            last_ultrasonic_ms = now;
            Ultrasonic_Service();
        }

        if (imu_i2c_data_valid == 0U && (now - last_scan_ms) >= 1000U)
        {
            last_scan_ms = now;
            IMU_I2C_Scan();
        }

        if ((now - last_lcd_ms) >= 200U)
        {
            last_lcd_ms = now;
            LCD_UI_Update();
        }

        HAL_Delay(10U);
    }
#else
    HAL_Init();
    SystemClock_Config();
    HAL_Delay(100);

    LED_Init();
    Actuator_GPIO_Init();
    Actuator_AllStop();
    Seedling_GPIO_Init();
    Mobile_GPIO_Init();
    DWT_Delay_Init();
    Ultrasonic_GPIO_Init();

    lcd_init();
    lcd_display_dir(0);
    lcd_display_on();
    LCD_BL(1);

    Car_RC1_InputCapture_Init();
    Car_RC2_InputCapture_Init();
    Car_RowContinue_InputCapture_Init();
    Car_Speed_InputCapture_Init();
    Car_PWM_Output_Init();
    I2C1_Init();
    USART1_UART_Init();
    USART2_UART_Init();
    USART3_UART_Init();
    LCD_UI_Init();
    Car_Start();
    Car_StartRowContinueInput();
    Car_StartSpeedInput();
    USART1_StartReceive();
    USART2_StartReceive();
    USART3_StartReceive();
    Actuator_AllStop();
    IMU_I2C_Scan();
    LCD_UI_Update();
    App_ServiceTasks();
    Actuator_GPIO_Init();
    Actuator_AllStop();

#if defined(APP_USE_FREERTOS) && (APP_USE_FREERTOS != 0)
    App_RTOS_StartScheduler();
#else
    Workflow_RunOnce();

    Actuator_AllStop();
    Seedling_Stop();
    Mobile_Stop();
    Car_PWM_OutputNeutral();

    while (1)
    {
        App_ServiceTasks();
        Car_PWM_OutputUpdate();
        HAL_Delay(10U);
    }
#endif
#endif
}
