#include "app.h"

static uint16_t work_step;

#define WORK_PLANT_TOTAL_COUNT 9U
#define WORK_PLANT_CAR_FORWARD_MS 500U
#define WORK_PLANT_SPEED_INPUT_MIN_US 1500U
#define WORK_PLANT_SPEED_INPUT_MAX_US 2000U
#define WORK_PLANT_SPEED_OUTPUT_MIN_US 1520U
#define WORK_PLANT_SPEED_OUTPUT_MAX_US RC_HIGH_LIMIT_US

void Workflow_SetStatus(uint16_t step_index, const char *text)
{
    __disable_irq();
    workflow_step_index = step_index;
    snprintf(workflow_status_buf, WORKFLOW_STATUS_BUF_SIZE, "%03u %s",
             (unsigned int)step_index, (text != NULL) ? text : "");
    __enable_irq();

    LCD_ShowValue(32, 722, 420, workflow_status_buf, LCD_TITLE_COLOR);
}

static void Work_NextStep(const char *text)
{
    work_step++;
    Workflow_SetStatus(work_step, text);
}

static void Work_DrillHole(void)
{
    Work_NextStep("Drill down10 motor10 up10");
    DrillRod_RunCycle();
}

static void Work_MobileForward(void)
{
    Work_NextStep("Mobile forward 8s");
    Mobile_RunMs(FORWARD, WORK_MOBILE_RUN_MS);
}

static void Work_MobileReverse(void)
{
    Work_NextStep("Mobile reverse 8s");
    Mobile_RunMs(REVERSE, WORK_MOBILE_RUN_MS);
}

static void Work_SeedlingForward7s(void)
{
    Work_NextStep("Seedling forward 7s");
    Seedling_RunMs(FORWARD, WORK_SEEDLING_FORWARD_MS);
}

static void Work_SeedlingForwardMs(uint32_t run_ms)
{
    char text[40];

    snprintf(text, sizeof(text), "Seedling forward %lu.%02lus",
             (unsigned long)(run_ms / 1000U), (unsigned long)((run_ms % 1000U) / 10U));
    Work_NextStep(text);
    Seedling_RunMs(FORWARD, run_ms);
}

static uint32_t Work_SeedlingForwardMsForPlant(uint8_t plant_index)
{
    if (plant_index == 1U)
    {
        return 9000U;
    }
    if ((plant_index == 4U) || (plant_index == 7U))
    {
        return 6450U;
    }
    return WORK_SEEDLING_FORWARD_MS;
}

static void Work_SeedlingReverse28s(void)
{
    Work_NextStep("Seedling reverse 28s");
    Seedling_RunMs(REVERSE, WORK_SEEDLING_REVERSE_MS);
}

static void Work_ConveyorForward(uint32_t run_ms)
{
    char text[40];

    snprintf(text, sizeof(text), "Conveyor forward %lu.%02lus",
             (unsigned long)(run_ms / 1000U), (unsigned long)((run_ms % 1000U) / 10U));
    Work_NextStep(text);
    Conveyor_Forward();
    App_DelayWithService(run_ms);
    Conveyor_Stop();
}

static void Work_ConveyorReverse(void)
{
    Work_NextStep("Conveyor reverse 2.50s");
    Conveyor_Reverse();
    App_DelayWithService(WORK_CONVEYOR_REVERSE_MS);
    Conveyor_Stop();
    App_DelayWithService(CONVEYOR_STOP_MS);
}

static void Work_Pump10s(void)
{
    uint32_t elapsed = 0U;

    Work_NextStep("Pump on 10s safe");

    /*
     * 水泵阶段强制锁车：
     * 1. 小车PWM保持中位
     * 2. 移苗步进停止
     * 3. 送苗步进停止
     * 4. 输送带、覆土、钻杆保持停止
     */
    Car_PWM_OutputNeutral();
    Mobile_Stop();
    Seedling_Stop();
    Conveyor_Stop();
    Cover_Stop();
    DrillRod_Stop();
    Drill_Off();

    while (elapsed < 10000U)
    {
        uint32_t start;

        App_WaitIfPaused();
        start = HAL_GetTick();

        /*
         * 10秒内反复保持水泵打开，
         * 同时反复把轮子PWM压回中位，防止轮子乱动。
         */
        Pump_On();
        Car_PWM_OutputNeutral();
        Mobile_Stop();
        Seedling_Stop();

        App_DelayWithService(20U);
        elapsed += HAL_GetTick() - start;
    }

    Pump_Off();

    Car_PWM_OutputNeutral();
    Mobile_Stop();
    Seedling_Stop();

}

static void Work_CoverSoil(void)
{
    Work_NextStep("Cover extend10 stop1 retract10");
    Cover_Forward();
    App_DelayWithService(WORK_COVER_FORWARD_MS);
    Cover_Stop();
    App_DelayWithService(WORK_COVER_STOP_MS);
    Cover_Reverse();
    App_DelayWithService(WORK_COVER_REVERSE_MS);
    Cover_Stop();
    App_DelayWithService(WORK_COVER_STOP_MS);
}

static void Work_CarForwardMs(uint32_t run_ms, const char *text)
{
    uint32_t elapsed = 0U;

    Work_NextStep(text);

    while (elapsed < run_ms)
    {
        uint32_t start;
        uint32_t phase_elapsed = elapsed;
        uint32_t pulse = WORK_CAR_NEUTRAL_US;

        App_WaitIfPaused();
        start = HAL_GetTick();

        if (phase_elapsed < WORK_CAR_RAMP_MS)
        {
            pulse = WORK_CAR_NEUTRAL_US +
                    ((WORK_CAR_FORWARD_US - WORK_CAR_NEUTRAL_US) * phase_elapsed) / WORK_CAR_RAMP_MS;
        }
        else if (phase_elapsed < WORK_CAR_HOLD_END_MS)
        {
            pulse = WORK_CAR_FORWARD_US;
        }
        else
        {
            uint32_t down_elapsed = phase_elapsed - WORK_CAR_HOLD_END_MS;
            pulse = WORK_CAR_FORWARD_US -
                    ((WORK_CAR_FORWARD_US - WORK_CAR_NEUTRAL_US) * down_elapsed) / WORK_CAR_RAMP_MS;
        }

        Car_PWM_OutputSet(pulse, pulse);
        App_ServiceTasks();
        HAL_Delay(20U);
        elapsed += HAL_GetTick() - start;
    }

    Car_PWM_OutputNeutral();
}

static void Work_CarForward10s(void)
{
    Work_CarForwardMs(WORK_CAR_FORWARD_MS, "Car forward 10s");
}

static void Work_CarForward2s(void)
{
    Work_CarForwardMs(WORK_ROW_FORWARD_MS, "Car forward 2s");
}

static uint32_t Work_PlantForwardPulseUs(void)
{
    uint32_t now = HAL_GetTick();
    uint32_t speed_us;

    if ((now - speed_rc.last_ms) >= RC_TIMEOUT_MS)
    {
        return WORK_PLANT_SPEED_OUTPUT_MIN_US;
    }

    speed_us = speed_rc.pulse_us;
    if (speed_us < WORK_PLANT_SPEED_INPUT_MIN_US)
    {
        speed_us = WORK_PLANT_SPEED_INPUT_MIN_US;
    }
    if (speed_us > WORK_PLANT_SPEED_INPUT_MAX_US)
    {
        speed_us = WORK_PLANT_SPEED_INPUT_MAX_US;
    }

    return WORK_PLANT_SPEED_OUTPUT_MIN_US +
           ((speed_us - WORK_PLANT_SPEED_INPUT_MIN_US) *
            (WORK_PLANT_SPEED_OUTPUT_MAX_US - WORK_PLANT_SPEED_OUTPUT_MIN_US)) /
           (WORK_PLANT_SPEED_INPUT_MAX_US - WORK_PLANT_SPEED_INPUT_MIN_US);
}

static void Work_PlantCarForwardStep(uint8_t plant_index)
{
    uint32_t elapsed = 0U;
    char text[40];

    snprintf(text, sizeof(text), "Plant %u car forward 0.5s", (unsigned int)plant_index);
    Work_NextStep(text);

    while (elapsed < WORK_PLANT_CAR_FORWARD_MS)
    {
        uint32_t start;

        App_WaitIfPaused();
        start = HAL_GetTick();
        Car_PWM_OutputSet(Work_PlantForwardPulseUs(), Work_PlantForwardPulseUs());
        App_ServiceTasks();
        HAL_Delay(20U);
        elapsed += HAL_GetTick() - start;
    }

    Car_PWM_OutputNeutral();
}

static int32_t Work_NormalizeYawDelta(int32_t current_x100, int32_t base_x100)
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

static int32_t Work_WaitYawValid(void)
{
    while (imu_i2c_data_valid == 0U)
    {
        App_WaitIfPaused();
        Car_PWM_OutputNeutral();
        App_ServiceTasks();
        HAL_Delay(10U);
    }

    return imu_yaw_x100;
}

static int32_t Work_CarTurnRight90(int32_t base_yaw_x100)
{
    int32_t current_yaw_x100 = base_yaw_x100;

    Work_NextStep("Turn right Y +90deg");
    while (Work_NormalizeYawDelta(current_yaw_x100, base_yaw_x100) < WORK_ROW_TURN_DEG_X100)
    {
        App_WaitIfPaused();
        Car_PWM_OutputSet(WORK_CAR_NEUTRAL_US, WORK_ROW_TURN_RIGHT_US);
        App_ServiceTasks();
        current_yaw_x100 = imu_yaw_x100;
        HAL_Delay(10U);
    }

    Car_PWM_OutputNeutral();
    return current_yaw_x100;
}

static void Work_RowChangeByImu(void)
{
    int32_t initial_yaw_x100;
    int32_t adjusted_yaw_x100;

    Work_CarForward2s();
    initial_yaw_x100 = Work_WaitYawValid();
    adjusted_yaw_x100 = Work_CarTurnRight90(initial_yaw_x100);
    Work_CarForward2s();
    (void)Work_CarTurnRight90(adjusted_yaw_x100);
    Work_CarForward2s();
}

static void Work_PlantingGroup(uint8_t round_index, uint8_t group_index)
{
    __disable_irq();
    workflow_round_index = round_index;
    workflow_group_index = group_index;
    __enable_irq();

    Work_DrillHole();
    Work_MobileForward();
    if (group_index == 1U)
    {
        Work_SeedlingForwardMs(Work_SeedlingForwardMsForPlant((uint8_t)(((round_index - 1U) * 3U) + 1U)));
        Work_ConveyorForward(CONVEYOR_FIRST_FORWARD_MS);
    }
    else
    {
        Work_ConveyorForward(CONVEYOR_NEXT_FORWARD_MS);
    }
    Work_Pump10s();
    Work_CoverSoil();
    Work_MobileReverse();
    if (group_index != 3U)
    {
        Work_CarForward10s();
    }
}

static void Work_PlantingPoint(uint8_t plant_index)
{
    uint8_t group_index = (uint8_t)(((plant_index - 1U) % 3U) + 1U);

    __disable_irq();
    workflow_round_index = (uint8_t)(((plant_index - 1U) / 3U) + 1U);
    workflow_group_index = group_index;
    __enable_irq();

    Work_DrillHole();
    Work_MobileForward();
    if (group_index == 1U)
    {
        Work_SeedlingForward7s();
        Work_ConveyorForward(CONVEYOR_FIRST_FORWARD_MS);
    }
    else
    {
        Work_ConveyorForward(CONVEYOR_NEXT_FORWARD_MS);
    }
    Work_Pump10s();
    Work_CoverSoil();
    Work_MobileReverse();
}

static void Work_Round(uint8_t round_index, uint8_t run_last_car_forward)
{
    Work_PlantingGroup(round_index, 1U);
    Work_PlantingGroup(round_index, 2U);
    Work_PlantingGroup(round_index, 3U);
    Work_ConveyorReverse();
    if (run_last_car_forward != 0U)
    {
        Work_RowChangeByImu();
    }
}

void Workflow_RunOnce(void)
{
    work_step = 0U;
    workflow_completed = 0U;
    __disable_irq();
    workflow_round_index = 0U;
    workflow_group_index = 0U;
    __enable_irq();

    Workflow_SetStatus(0U, "Legacy workflow disabled");
    Actuator_AllStop();
    Seedling_Stop();
    Mobile_Stop();
    Car_PWM_OutputNeutral();
    Workflow_SetStatus(work_step, "Use manual_car_control snake workflow");
    workflow_completed = 1U;
}
