/*
 * 踏沙小程序通信补丁示例
 * ---------------------------------
 * 用法：
 * 1. 不要直接把本文件作为独立 .c 加入编译。
 * 2. 将本文件中的代码块复制到 src/main.c 的 CAR_GPIO_MANUAL_TEST_ONLY 分支内部，
 *    这样它能访问 ManualCar_AllStop、ManualCar_CarForwardAuto、Car_Motor_SetLeft 等 static 函数。
 * 3. 复制后按下面“插入点”说明修改 PlantOneSeedling、HAL_UART_RxCpltCallback 和主循环。
 */

/* ========================= 插入点 A：放在 main.c 静态变量区域附近 ========================= */

static volatile uint8_t mini_plant_count = 0U;
static volatile uint8_t mini_total_count = SNAKE_TOTAL_SEEDLINGS;
static volatile uint8_t mini_auto_start_requested = 0U;
static volatile uint8_t mini_pause_requested = 0U;
static volatile uint8_t mini_estop_requested = 0U;
static volatile uint8_t mini_remote_drive_active = 0U;
static volatile uint32_t mini_remote_drive_deadline_ms = 0U;
static volatile char mini_stage[16] = "idle";
static uint32_t mini_last_report_ms = 0U;
static char mini_rx_line[96];
static uint8_t mini_rx_len = 0U;

static void Mini_CopyStage(const char *stage)
{
    uint8_t i = 0U;
    if (stage == NULL) stage = "idle";
    __disable_irq();
    while (stage[i] != '\0' && i < (sizeof(mini_stage) - 1U))
    {
        mini_stage[i] = stage[i];
        i++;
    }
    mini_stage[i] = '\0';
    __enable_irq();
}

static void Mini_SetStage(const char *stage)
{
    Mini_CopyStage(stage);
}

static void Mini_SetPlantCount(uint8_t count)
{
    if (count > SNAKE_TOTAL_SEEDLINGS) count = SNAKE_TOTAL_SEEDLINGS;
    mini_plant_count = count;
}

static void Mini_TelemetryTask(void)
{
    uint32_t now = HAL_GetTick();
    char line[180];
    char stage_copy[16];
    uint8_t i;
    uint32_t ultra_mm = ultrasonic_distance_mm;
    int32_t roll100 = imu_roll_x100;
    int32_t pitch100 = imu_pitch_x100;
    int32_t yaw100 = imu_yaw_x100;
    uint32_t l_cm = radar_left_distance_cm;
    uint32_t f_cm = radar_front_distance_cm;
    uint32_t r_cm = radar_right_distance_cm;

    if ((now - mini_last_report_ms) < 200U)
    {
        return;
    }
    mini_last_report_ms = now;

    __disable_irq();
    for (i = 0U; i < sizeof(stage_copy); i++)
    {
        stage_copy[i] = mini_stage[i];
        if (mini_stage[i] == '\0') break;
    }
    stage_copy[sizeof(stage_copy) - 1U] = '\0';
    __enable_irq();

    snprintf(line, sizeof(line),
             "$TS,%lu,%s,%s,%u,%u,%lu,%ld,%ld,%ld,%lu,%lu,%lu,%u,%u\r\n",
             (unsigned long)now,
             (mini_auto_start_requested != 0U) ? "auto" : "manual",
             stage_copy,
             (unsigned int)mini_plant_count,
             (unsigned int)mini_total_count,
             (unsigned long)ultra_mm,
             (long)roll100,
             (long)pitch100,
             (long)yaw100,
             (unsigned long)l_cm,
             (unsigned long)f_cm,
             (unsigned long)r_cm,
             2400U,
             100U);

    UART2_QueueString(line);
}

static void Mini_RemoteDriveStop(void)
{
    mini_remote_drive_active = 0U;
    ManualCar_CarStopAuto();
}

static void Mini_RemoteDriveApply(char dir)
{
    mini_remote_drive_active = 1U;
    mini_remote_drive_deadline_ms = HAL_GetTick() + 350U;

    if (dir == 'F')
    {
        ManualCar_CarForwardAuto();
    }
    else if (dir == 'B')
    {
        Car_Motor_SetLeft(-1);
        Car_Motor_SetRight(-1);
    }
    else if (dir == 'L')
    {
        ManualCar_CarLeftAuto();
    }
    else if (dir == 'R')
    {
        ManualCar_CarRightAuto();
    }
    else
    {
        Mini_RemoteDriveStop();
    }
}

static void Mini_RemoteDriveTask(void)
{
    if (mini_remote_drive_active != 0U && HAL_GetTick() > mini_remote_drive_deadline_ms)
    {
        Mini_RemoteDriveStop();
    }
}

static void Mini_HandleCommandLine(const char *line)
{
    if (strncmp(line, "@CMD,MOVE,", 10) == 0)
    {
        Mini_SetStage("idle");
        Mini_RemoteDriveApply(line[10]);
    }
    else if (strcmp(line, "@CMD,STOP") == 0)
    {
        Mini_RemoteDriveStop();
    }
    else if (strcmp(line, "@CMD,AUTO,START") == 0)
    {
        mini_auto_start_requested = 1U;
        mini_pause_requested = 0U;
        mini_estop_requested = 0U;
        if (manual_snake_resume_seed == 0U)
        {
            manual_snake_resume_seed = 1U;
            manual_snake_resume_phase = MANUAL_SNAKE_PHASE_DRILL;
            mini_plant_count = 0U;
        }
    }
    else if (strcmp(line, "@CMD,AUTO,PAUSE") == 0)
    {
        mini_pause_requested = 1U;
        manual_snake_abort_requested = 1U;
        ManualCar_AllStop();
        Mini_SetStage("idle");
    }
    else if (strcmp(line, "@CMD,AUTO,RESUME") == 0)
    {
        mini_pause_requested = 0U;
        mini_auto_start_requested = 1U;
    }
    else if (strcmp(line, "@CMD,COUNT,RESET") == 0)
    {
        mini_plant_count = 0U;
        manual_snake_resume_seed = 1U;
        manual_snake_resume_phase = MANUAL_SNAKE_PHASE_DRILL;
        Mini_SetStage("idle");
    }
    else if (strcmp(line, "@CMD,ESTOP") == 0)
    {
        mini_estop_requested = 1U;
        mini_pause_requested = 1U;
        mini_auto_start_requested = 0U;
        manual_snake_abort_requested = 1U;
        ManualCar_AllStop();
        Mini_SetStage("error");
    }
}

static void Mini_Uart2PushByte(uint8_t data)
{
    if (data == '\r' || data == '\n')
    {
        if (mini_rx_len != 0U)
        {
            mini_rx_line[mini_rx_len] = '\0';
            Mini_HandleCommandLine(mini_rx_line);
            mini_rx_len = 0U;
        }
        return;
    }

    if (mini_rx_len < (sizeof(mini_rx_line) - 1U))
    {
        mini_rx_line[mini_rx_len++] = (char)data;
    }
    else
    {
        mini_rx_len = 0U;
    }
}

/* ========================= 插入点 B：改 PlantOneSeedling(seed_index) =========================
 * 在每个动作前加入 Mini_SetStage，浇水完成后 Mini_SetPlantCount(seed_index)。
 * 参考如下：
 */

/*
static void PlantOneSeedling(uint8_t seed_index)
{
    if (manual_snake_resume_phase <= MANUAL_SNAKE_PHASE_DRILL)
    {
        Mini_SetStage("drill");
        manual_snake_resume_phase = MANUAL_SNAKE_PHASE_DRILL;
        ManualCar_DrillHole();
        if (manual_snake_abort_requested != 0U) return;
        manual_snake_resume_phase = MANUAL_SNAKE_PHASE_AFTER_DRILL_CAR;
    }

    if (manual_snake_resume_phase == MANUAL_SNAKE_PHASE_AFTER_DRILL_CAR)
    {
        Mini_SetStage("forward");
        if (ManualCar_CarRun(ManualCar_CarForwardAuto, SNAKE_CAR_AFTER_DRILL_MS) == 0U) return;
        manual_snake_resume_phase = MANUAL_SNAKE_PHASE_SEED;
    }

    if (manual_snake_resume_phase == MANUAL_SNAKE_PHASE_SEED)
    {
        Mini_SetStage("seed");
        ManualCar_Servo51OneStep();
        if (manual_snake_abort_requested != 0U) return;
        manual_snake_resume_phase = MANUAL_SNAKE_PHASE_COVER;
    }

    if (manual_snake_resume_phase == MANUAL_SNAKE_PHASE_COVER)
    {
        Mini_SetStage("cover");
        ManualCar_CoverSoilAuto();
        if (manual_snake_abort_requested != 0U) return;
        manual_snake_resume_phase = MANUAL_SNAKE_PHASE_WATER;
    }

    if (manual_snake_resume_phase == MANUAL_SNAKE_PHASE_WATER)
    {
        Mini_SetStage("water");
        ManualCar_WaterAuto();
        if (manual_snake_abort_requested != 0U) return;
        Mini_SetPlantCount(seed_index);  // 关键：以浇水完成为种植结束标志
        manual_snake_resume_phase = MANUAL_SNAKE_PHASE_NEXT_CAR;
    }

    if (manual_snake_resume_phase == MANUAL_SNAKE_PHASE_NEXT_CAR &&
        seed_index < SNAKE_TOTAL_SEEDLINGS)
    {
        Mini_SetStage("forward");
        (void)ManualCar_CarRun(ManualCar_CarForwardAuto, SNAKE_CAR_NEXT_POINT_MS);
        if (manual_snake_abort_requested != 0U) return;
    }

    manual_snake_resume_phase = MANUAL_SNAKE_PHASE_DONE;
    if (seed_index >= SNAKE_TOTAL_SEEDLINGS)
    {
        Mini_SetStage("complete");
        ManualCar_AllStop();
    }
}
*/

/* ========================= 插入点 C：改 USART2 接收回调 =========================
 * 在 HAL_UART_RxCpltCallback 的 USART2 分支中加入 Mini_Uart2PushByte：
 */

/*
else if (huart->Instance == USART2)
{
    UART2_PushByte(uart2_rx_byte);
    Mini_Uart2PushByte(uart2_rx_byte);
    USART2_StartReceive();
}
*/

/* ========================= 插入点 D：主循环周期调用 =========================
 * 在 manual_car_control 分支 while(1) 循环里，每轮都调用：
 */

/*
Mini_TelemetryTask();
Mini_RemoteDriveTask();

if (mini_estop_requested != 0U)
{
    ManualCar_AllStop();
    continue;
}

if (mini_auto_start_requested != 0U && mini_pause_requested == 0U)
{
    mini_auto_start_requested = 0U;
    ManualCar_AllStop();
    ManualCar_RunSnakeWorkflow();
}
*/
