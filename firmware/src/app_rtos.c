#include "app.h"

#if defined(APP_USE_FREERTOS) && (APP_USE_FREERTOS != 0)
#include "FreeRTOS.h"
#include "task.h"

static void WorkflowTask(void *argument);
static void CarTask(void *argument);
static void UartTask(void *argument);
static void SafetyTask(void *argument);
static void DisplayTask(void *argument);

void App_RTOS_StartScheduler(void)
{
    BaseType_t ok = pdPASS;

    ok &= xTaskCreate(WorkflowTask, "WorkflowTask", 1536U, NULL, 1U, NULL);
    ok &= xTaskCreate(CarTask, "CarTask", 384U, NULL, 4U, NULL);
    ok &= xTaskCreate(UartTask, "UartTask", 384U, NULL, 3U, NULL);
    ok &= xTaskCreate(SafetyTask, "SafetyTask", 384U, NULL, 5U, NULL);
    ok &= xTaskCreate(DisplayTask, "DisplayTask", 1024U, NULL, 2U, NULL);

    if (ok != pdPASS)
    {
        Error_Handler();
    }

    vTaskStartScheduler();
    Error_Handler();
}

static void WorkflowTask(void *argument)
{
    (void)argument;

    Workflow_RunOnce();

    for (;;)
    {
        Actuator_AllStop();
        Seedling_Stop();
        Mobile_Stop();
        Car_PWM_OutputNeutral();
        vTaskDelay(pdMS_TO_TICKS(1000U));
    }
}

static void CarTask(void *argument)
{
    TickType_t last_wake = xTaskGetTickCount();

    (void)argument;

    for (;;)
    {
        if (Car_RC_OverrideActive() != 0U)
        {
            Car_PWM_OutputUpdate();
        }
        vTaskDelayUntil(&last_wake, pdMS_TO_TICKS(20U));
    }
}

static void UartTask(void *argument)
{
    TickType_t last_wake = xTaskGetTickCount();

    (void)argument;

    for (;;)
    {
        USART1_ServiceEcho();
        USART2_ServiceEcho();
        USART3_ServiceEcho();
        vTaskDelayUntil(&last_wake, pdMS_TO_TICKS(5U));
    }
}

static void SafetyTask(void *argument)
{
    TickType_t last_wake = xTaskGetTickCount();

    (void)argument;

    for (;;)
    {
        App_SafetyService();
        vTaskDelayUntil(&last_wake, pdMS_TO_TICKS(10U));
    }
}

static void DisplayTask(void *argument)
{
    TickType_t last_wake = xTaskGetTickCount();
    uint32_t last_imu_ms = 0U;
    uint32_t last_lcd_ms = 0U;
    uint32_t last_ultrasonic_ms = 0U;
    uint32_t last_scan_ms = 0U;

    (void)argument;

    for (;;)
    {
        uint32_t now = HAL_GetTick();

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

        vTaskDelayUntil(&last_wake, pdMS_TO_TICKS(20U));
    }
}

void vApplicationMallocFailedHook(void)
{
    Error_Handler();
}

void vApplicationStackOverflowHook(TaskHandle_t task, char *task_name)
{
    (void)task;
    (void)task_name;
    Error_Handler();
}

void HAL_Delay(uint32_t delay_ms)
{
    uint32_t tick_start;
    uint32_t wait;

    if (delay_ms == 0U)
    {
        return;
    }

    if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED)
    {
        vTaskDelay(pdMS_TO_TICKS(delay_ms));
        return;
    }

    tick_start = HAL_GetTick();
    wait = delay_ms + (uint32_t)HAL_GetTickFreq();
    while ((HAL_GetTick() - tick_start) < wait)
    {
    }
}
#else
void App_RTOS_StartScheduler(void)
{
    Workflow_RunOnce();
}
#endif
