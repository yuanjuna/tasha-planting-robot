#include "app.h"

static void App_UpdateWorkSwitch(void)
{
#if defined(WORK_RC_SWITCH_ENABLE) && (WORK_RC_SWITCH_ENABLE != 0)
    uint32_t now = HAL_GetTick();
    CarRc_Channel_t *switch_rc = &work_switch_rc;

#if defined(CAR_IMU_ROW_TEST) && (CAR_IMU_ROW_TEST != 0)
    switch_rc = &row_continue_rc;
#endif

    if ((now - switch_rc->last_ms) >= RC_TIMEOUT_MS)
    {
        workflow_paused = 1U;
        return;
    }

    if (switch_rc->pulse_us < WORK_SWITCH_THRESHOLD_US)
    {
        workflow_paused = 1U;
    }
    else
    {
        workflow_paused = 0U;
    }
#else
    workflow_paused = 0U;
#endif
}

static void App_StopMotionOutputsNow(void)
{
    Actuator_AllStop();
    HAL_GPIO_WritePin(SEEDLING_STEP_GPIO_PORT, SEEDLING_STEP_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(MOBILE_STEP_GPIO_PORT, MOBILE_STEP_PIN, GPIO_PIN_SET);
    Car_PWM_OutputNeutral();
}

void App_SafetyService(void)
{
    App_UpdateWorkSwitch();

    if (workflow_completed != 0U)
    {
        return;
    }

    if (workflow_paused != 0U)
    {
        App_StopMotionOutputsNow();
        return;
    }

    if (Car_RC_OverrideActive() != 0U)
    {
        App_StopMotionOutputsNow();
        Car_PWM_OutputUpdate();
    }
}

void App_ServiceTasks(void)
{
#if defined(APP_USE_FREERTOS) && (APP_USE_FREERTOS != 0)
    App_SafetyService();
#else
    static uint32_t last_imu_ms = 0U;
    static uint32_t last_lcd_ms = 0U;
    static uint32_t last_ultrasonic_ms = 0U;
    static uint32_t last_scan_ms = 0U;
    static uint8_t busy = 0U;
    uint32_t now;

    if (busy != 0U)
    {
        return;
    }
    busy = 1U;
    now = HAL_GetTick();

    USART1_ServiceEcho();
    USART2_ServiceEcho();
    USART3_ServiceEcho();
    App_UpdateWorkSwitch();

#if defined(CAR_IMU_ROW_TEST) && (CAR_IMU_ROW_TEST != 0)
    if (workflow_paused != 0U)
    {
        App_StopMotionOutputsNow();
        Car_PWM_OutputUpdateManual();
    }
#else
    if (Car_RC_OverrideActive() != 0U)
    {
        Car_PWM_OutputUpdate();
    }
#endif

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

    busy = 0U;
#endif
}

uint8_t App_WorkflowPaused(void)
{
    App_UpdateWorkSwitch();
    return workflow_paused;
}

void App_WaitIfPaused(void)
{
    while (App_WorkflowPaused() != 0U && workflow_completed == 0U)
    {
#if defined(APP_USE_FREERTOS) && (APP_USE_FREERTOS != 0)
        App_StopMotionOutputsNow();
#else
        Actuator_AllStop();
        HAL_GPIO_WritePin(SEEDLING_STEP_GPIO_PORT, SEEDLING_STEP_PIN, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(MOBILE_STEP_GPIO_PORT, MOBILE_STEP_PIN, GPIO_PIN_SET);
        Car_PWM_OutputUpdateManual();
#endif
        App_ServiceTasks();
        HAL_Delay(20U);
    }

    Car_PWM_OutputNeutral();
}

void App_DelayWithService(uint32_t delay_ms)
{
    uint32_t elapsed = 0U;

    while (elapsed < delay_ms)
    {
        uint32_t start;

        App_WaitIfPaused();
        start = HAL_GetTick();
        App_ServiceTasks();
        HAL_Delay(10U);
        elapsed += HAL_GetTick() - start;
    }
}
