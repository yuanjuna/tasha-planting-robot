#include "app.h"

void Actuator_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOG_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOF_CLK_ENABLE();
    __HAL_RCC_GPIOE_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    HAL_GPIO_WritePin(DRILL_IN1_GPIO_PORT, DRILL_IN1_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(DRILL_IN2_GPIO_PORT, DRILL_IN2_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(DRILL_ROD_IN1_GPIO_PORT, DRILL_ROD_IN1_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(DRILL_ROD_IN2_GPIO_PORT, DRILL_ROD_IN2_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(CONVEYOR_IN1_GPIO_PORT, CONVEYOR_IN1_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(CONVEYOR_IN2_GPIO_PORT, CONVEYOR_IN2_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(COVER_IN1_GPIO_PORT, COVER_IN1_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(COVER_IN2_GPIO_PORT, COVER_IN2_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(PUMP_RELAY_GPIO_PORT, PUMP_RELAY_PIN, PUMP_RELAY_OFF_LEVEL);

    GPIO_InitStruct.Pin = DRILL_IN1_PIN | DRILL_IN2_PIN |
                          DRILL_ROD_IN1_PIN | DRILL_ROD_IN2_PIN |
                          CONVEYOR_IN1_PIN | CONVEYOR_IN2_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = COVER_IN1_PIN;
    HAL_GPIO_Init(COVER_IN1_GPIO_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = COVER_IN2_PIN;
    HAL_GPIO_Init(COVER_IN2_GPIO_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = PUMP_RELAY_PIN;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    HAL_GPIO_Init(PUMP_RELAY_GPIO_PORT, &GPIO_InitStruct);
}

void Drill_On(void)
{
    HAL_GPIO_WritePin(DRILL_IN1_GPIO_PORT, DRILL_IN1_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(DRILL_IN2_GPIO_PORT, DRILL_IN2_PIN, GPIO_PIN_RESET);
}

void Drill_Off(void)
{
    HAL_GPIO_WritePin(DRILL_IN1_GPIO_PORT, DRILL_IN1_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(DRILL_IN2_GPIO_PORT, DRILL_IN2_PIN, GPIO_PIN_RESET);
}

void DrillRod_Down(void)
{
    HAL_GPIO_WritePin(DRILL_ROD_IN1_GPIO_PORT, DRILL_ROD_IN1_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(DRILL_ROD_IN2_GPIO_PORT, DRILL_ROD_IN2_PIN, GPIO_PIN_SET);
}

void DrillRod_Up(void)
{
    HAL_GPIO_WritePin(DRILL_ROD_IN1_GPIO_PORT, DRILL_ROD_IN1_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(DRILL_ROD_IN2_GPIO_PORT, DRILL_ROD_IN2_PIN, GPIO_PIN_RESET);
}

void DrillRod_Stop(void)
{
    HAL_GPIO_WritePin(DRILL_ROD_IN1_GPIO_PORT, DRILL_ROD_IN1_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(DRILL_ROD_IN2_GPIO_PORT, DRILL_ROD_IN2_PIN, GPIO_PIN_RESET);
}

void Conveyor_Forward(void)
{
    HAL_GPIO_WritePin(CONVEYOR_IN1_GPIO_PORT, CONVEYOR_IN1_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(CONVEYOR_IN2_GPIO_PORT, CONVEYOR_IN2_PIN, GPIO_PIN_RESET);
}

void Conveyor_Reverse(void)
{
    HAL_GPIO_WritePin(CONVEYOR_IN1_GPIO_PORT, CONVEYOR_IN1_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(CONVEYOR_IN2_GPIO_PORT, CONVEYOR_IN2_PIN, GPIO_PIN_SET);
}

void Conveyor_Stop(void)
{
    HAL_GPIO_WritePin(CONVEYOR_IN1_GPIO_PORT, CONVEYOR_IN1_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(CONVEYOR_IN2_GPIO_PORT, CONVEYOR_IN2_PIN, GPIO_PIN_RESET);
}

void Cover_Forward(void)
{
    HAL_GPIO_WritePin(COVER_IN1_GPIO_PORT, COVER_IN1_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(COVER_IN2_GPIO_PORT, COVER_IN2_PIN, GPIO_PIN_SET);
}

void Cover_Reverse(void)
{
    HAL_GPIO_WritePin(COVER_IN1_GPIO_PORT, COVER_IN1_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(COVER_IN2_GPIO_PORT, COVER_IN2_PIN, GPIO_PIN_RESET);
}

void Cover_Stop(void)
{
    HAL_GPIO_WritePin(COVER_IN1_GPIO_PORT, COVER_IN1_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(COVER_IN2_GPIO_PORT, COVER_IN2_PIN, GPIO_PIN_RESET);
}

void Pump_On(void)
{
    HAL_GPIO_WritePin(PUMP_RELAY_GPIO_PORT, PUMP_RELAY_PIN, PUMP_RELAY_ON_LEVEL);
}

void Pump_Off(void)
{
    HAL_GPIO_WritePin(PUMP_RELAY_GPIO_PORT, PUMP_RELAY_PIN, PUMP_RELAY_OFF_LEVEL);
}

void Actuator_AllStop(void)
{
    Drill_Off();
    DrillRod_Stop();
    Conveyor_Stop();
    Cover_Stop();
    Pump_Off();
}

static void Actuator_DelayWithHold(void (*hold_action)(void), uint32_t run_ms)
{
    uint32_t elapsed = 0U;

    while (elapsed < run_ms)
    {
        uint32_t start;
        uint32_t slice = run_ms - elapsed;

        if (slice > 50U)
        {
            slice = 50U;
        }

        App_WaitIfPaused();
        if (hold_action != NULL)
        {
            hold_action();
        }
        start = HAL_GetTick();
        App_DelayWithService(slice);
        elapsed += HAL_GetTick() - start;
    }
}

static void Drill_PwmRun(uint16_t duty_x1000, uint32_t run_ms, void (*hold_action)(void))
{
    const uint32_t period_us = 20000U;
    uint32_t elapsed = 0U;

    if (duty_x1000 >= 1000U)
    {
        Drill_On();
        Actuator_DelayWithHold(hold_action, run_ms);
        return;
    }
    if (duty_x1000 == 0U)
    {
        Drill_Off();
        Actuator_DelayWithHold(hold_action, run_ms);
        return;
    }

    while (elapsed < run_ms)
    {
        uint32_t start;
        uint32_t on_us = (period_us * (uint32_t)duty_x1000) / 1000U;

        App_WaitIfPaused();
        if (hold_action != NULL)
        {
            hold_action();
        }
        start = HAL_GetTick();
        Drill_On();
        delay_us(on_us);
        Drill_Off();
        delay_us(period_us - on_us);
        App_ServiceTasks();
        elapsed += HAL_GetTick() - start;
    }
}

static void Drill_Ramp(uint8_t ramp_up, uint32_t ramp_ms, void (*hold_action)(void))
{
    const uint32_t slice_ms = 20U;
    uint32_t elapsed = 0U;

    while (elapsed < ramp_ms)
    {
        uint32_t slice_start;
        uint16_t duty;

        if (ramp_up != 0U)
        {
            duty = (uint16_t)((elapsed * 1000U) / ramp_ms);
        }
        else
        {
            duty = (uint16_t)(1000U - ((elapsed * 1000U) / ramp_ms));
        }
        App_WaitIfPaused();
        (void)slice_start;
        Drill_PwmRun(duty, slice_ms, hold_action);
        elapsed += slice_ms;
    }
}

void DrillRod_RunCycle(void)
{
    Drill_Off();
    DrillRod_Stop();
    App_DelayWithService(500U);

    DrillRod_Down();
    Actuator_DelayWithHold(DrillRod_Down, WORK_DRILL_ROD_DOWN_ONLY_MS);

    Drill_On();
    DrillRod_Down();
    Actuator_DelayWithHold(DrillRod_Down, WORK_DRILL_WITH_DOWN_MS);

    DrillRod_Up();
    Actuator_DelayWithHold(DrillRod_Up, WORK_DRILL_WITH_UP_MS);

    Drill_Off();
    DrillRod_Up();
    Actuator_DelayWithHold(DrillRod_Up, WORK_DRILL_ROD_UP_ONLY_MS);

    Actuator_AllStop();
}

void Conveyor_RunCycle(void)
{
    Conveyor_Forward();
    Actuator_DelayWithHold(Conveyor_Forward, CONVEYOR_FIRST_FORWARD_MS);

    Conveyor_Stop();
    App_DelayWithService(CONVEYOR_STOP_MS);

    Conveyor_Forward();
    Actuator_DelayWithHold(Conveyor_Forward, CONVEYOR_NEXT_FORWARD_MS);

    Conveyor_Stop();
    App_DelayWithService(CONVEYOR_STOP_MS);

    Conveyor_Forward();
    Actuator_DelayWithHold(Conveyor_Forward, CONVEYOR_NEXT_FORWARD_MS);

    Conveyor_Stop();
    App_DelayWithService(CONVEYOR_STOP_MS);

    Conveyor_Reverse();
    Actuator_DelayWithHold(Conveyor_Reverse, CONVEYOR_REVERSE_MS);

    Conveyor_Stop();
    App_DelayWithService(CONVEYOR_STOP_MS);
}

void Cover_RunCycle(void)
{
    Cover_Forward();
    Actuator_DelayWithHold(Cover_Forward, COVER_FORWARD_MS);

    Cover_Stop();
    App_DelayWithService(COVER_STOP_MS);

    Cover_Reverse();
    Actuator_DelayWithHold(Cover_Reverse, COVER_REVERSE_MS);

    Cover_Stop();
}

void Pump_RunCycle(void)
{
    Pump_On();
    Actuator_DelayWithHold(Pump_On, PUMP_ON_MS);
    Pump_Off();
    App_DelayWithService(PUMP_OFF_MS);
}
