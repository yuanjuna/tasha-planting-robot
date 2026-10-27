#include "app.h"

static uint32_t car_gpio_ch1_center_us = RC_NEUTRAL_US;
static uint32_t car_gpio_ch2_center_us = RC_NEUTRAL_US;
static uint32_t car_gpio_calib_start_ms = 0U;
static uint32_t car_gpio_calib_ch1_sum = 0U;
static uint32_t car_gpio_calib_ch2_sum = 0U;
static uint32_t car_gpio_calib_count = 0U;
static uint8_t car_gpio_calib_active = 0U;
static uint8_t car_gpio_calib_done = 0U;
static TIM_HandleTypeDef car_motor_pwm_tim;
static uint8_t car_motor_pwm_ready = 0U;

/* Manual car speed PWM:
 *   PD12 / TIM4_CH1 -> left motor speed command through the 5V level shifter.
 *   PD13 / TIM4_CH2 -> right motor speed command through the 5V level shifter.
 * The joystick leaves the deadband at 10% duty and reaches 90% at full stick.
 */
#define CAR_MOTOR_PWM_PERIOD_US 1000U
#define CAR_MOTOR_PWM_MIN_DUTY_PERCENT 10U
#define CAR_MOTOR_PWM_MAX_DUTY_PERCENT 90U
#define CAR_MOTOR_PWM_FULL_SCALE 1000L

static uint32_t car_motor_default_run_duty_percent = CAR_MOTOR_PWM_MAX_DUTY_PERCENT;

/* Manual group / auto speed input capture: PE6 / EXTI6.
 * This avoids PA8/TIM1 and PA1/RMII_REF_CLK on the Explorer board.
 */
void Car_Speed_InputCapture_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOE_CLK_ENABLE();

    GPIO_InitStruct.Pin = MANUAL_MODE_INPUT_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(MANUAL_MODE_INPUT_GPIO_PORT, &GPIO_InitStruct);

    HAL_NVIC_SetPriority(EXTI9_5_IRQn, 1U, 0U);
    HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
}

/* Remote car CH1 input capture: PA0 / TIM2_CH1. */
void Car_RC1_InputCapture_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    TIM_ClockConfigTypeDef sClockSourceConfig = {0};
    TIM_MasterConfigTypeDef sMasterConfig = {0};
    TIM_IC_InitTypeDef sConfigIC = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_TIM2_CLK_ENABLE();

    GPIO_InitStruct.Pin = GPIO_PIN_0;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF1_TIM2;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    htim2.Instance = TIM2;
    htim2.Init.Prescaler = 83U;
    htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim2.Init.Period = 0xFFFFFFFFU;
    htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    if (HAL_TIM_Base_Init(&htim2) != HAL_OK) Error_Handler();
    sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
    if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK) Error_Handler();
    if (HAL_TIM_IC_Init(&htim2) != HAL_OK) Error_Handler();
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK) Error_Handler();
    sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_RISING;
    sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
    sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
    sConfigIC.ICFilter = 0U;
    if (HAL_TIM_IC_ConfigChannel(&htim2, &sConfigIC, TIM_CHANNEL_1) != HAL_OK) Error_Handler();
    HAL_NVIC_SetPriority(TIM2_IRQn, 1U, 0U);
    HAL_NVIC_EnableIRQ(TIM2_IRQn);
}

/* Extra display-only switch input capture: PB14 / TIM12_CH1. */
void Car_RowContinue_InputCapture_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    TIM_ClockConfigTypeDef sClockSourceConfig = {0};
    TIM_IC_InitTypeDef sConfigIC = {0};

    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_TIM12_CLK_ENABLE();

    GPIO_InitStruct.Pin = GPIO_PIN_14;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF9_TIM12;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    htim12.Instance = TIM12;
    htim12.Init.Prescaler = 83U;
    htim12.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim12.Init.Period = 0xFFFFU;
    htim12.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim12.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    if (HAL_TIM_Base_Init(&htim12) != HAL_OK) Error_Handler();
    sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
    if (HAL_TIM_ConfigClockSource(&htim12, &sClockSourceConfig) != HAL_OK) Error_Handler();
    if (HAL_TIM_IC_Init(&htim12) != HAL_OK) Error_Handler();

    sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_RISING;
    sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
    sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
    sConfigIC.ICFilter = 8U;
    if (HAL_TIM_IC_ConfigChannel(&htim12, &sConfigIC, TIM_CHANNEL_1) != HAL_OK) Error_Handler();
    HAL_NVIC_SetPriority(TIM8_BRK_TIM12_IRQn, 1U, 0U);
    HAL_NVIC_EnableIRQ(TIM8_BRK_TIM12_IRQn);
}

/* Remote car CH2 input capture: PC6 / TIM8_CH1. */
void Car_RC2_InputCapture_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    TIM_ClockConfigTypeDef sClockSourceConfig = {0};
    TIM_MasterConfigTypeDef sMasterConfig = {0};
    TIM_IC_InitTypeDef sConfigIC = {0};

    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_TIM8_CLK_ENABLE();

    GPIO_InitStruct.Pin = GPIO_PIN_6;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF3_TIM8;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_7;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    htim8.Instance = TIM8;
    htim8.Init.Prescaler = 167U;
    htim8.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim8.Init.Period = 0xFFFFU;
    htim8.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim8.Init.RepetitionCounter = 0U;
    htim8.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    if (HAL_TIM_Base_Init(&htim8) != HAL_OK) Error_Handler();
    sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
    if (HAL_TIM_ConfigClockSource(&htim8, &sClockSourceConfig) != HAL_OK) Error_Handler();
    if (HAL_TIM_IC_Init(&htim8) != HAL_OK) Error_Handler();
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(&htim8, &sMasterConfig) != HAL_OK) Error_Handler();
    sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_RISING;
    sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
    sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
    sConfigIC.ICFilter = 15U;
    if (HAL_TIM_IC_ConfigChannel(&htim8, &sConfigIC, TIM_CHANNEL_1) != HAL_OK) Error_Handler();
    if (HAL_TIM_IC_ConfigChannel(&htim8, &sConfigIC, TIM_CHANNEL_2) != HAL_OK) Error_Handler();
    HAL_NVIC_SetPriority(TIM8_CC_IRQn, 1U, 0U);
    HAL_NVIC_EnableIRQ(TIM8_CC_IRQn);
}

/* Work switch input capture only: PC7 / TIM8_CH2. */
void Car_WorkSwitch_InputCapture_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    TIM_ClockConfigTypeDef sClockSourceConfig = {0};
    TIM_MasterConfigTypeDef sMasterConfig = {0};
    TIM_IC_InitTypeDef sConfigIC = {0};

    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_TIM8_CLK_ENABLE();

    GPIO_InitStruct.Pin = GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF3_TIM8;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    htim8.Instance = TIM8;
    htim8.Init.Prescaler = 167U;
    htim8.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim8.Init.Period = 0xFFFFU;
    htim8.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim8.Init.RepetitionCounter = 0U;
    htim8.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    if (HAL_TIM_Base_Init(&htim8) != HAL_OK) Error_Handler();
    sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
    if (HAL_TIM_ConfigClockSource(&htim8, &sClockSourceConfig) != HAL_OK) Error_Handler();
    if (HAL_TIM_IC_Init(&htim8) != HAL_OK) Error_Handler();
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(&htim8, &sMasterConfig) != HAL_OK) Error_Handler();
    sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_RISING;
    sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
    sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
    sConfigIC.ICFilter = 8U;
    if (HAL_TIM_IC_ConfigChannel(&htim8, &sConfigIC, TIM_CHANNEL_2) != HAL_OK) Error_Handler();
    HAL_NVIC_SetPriority(TIM8_CC_IRQn, 1U, 0U);
    HAL_NVIC_EnableIRQ(TIM8_CC_IRQn);
}

/* Remote car PWM output: PA6 / PA7, TIM3_CH1 / TIM3_CH2. */
void Car_PWM_Output_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    TIM_ClockConfigTypeDef sClockSourceConfig = {0};
    TIM_MasterConfigTypeDef sMasterConfig = {0};
    TIM_OC_InitTypeDef sConfigOC = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_TIM3_CLK_ENABLE();

    GPIO_InitStruct.Pin = GPIO_PIN_6 | GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF2_TIM3;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    htim3.Instance = TIM3;
    htim3.Init.Prescaler = 83U;
    htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim3.Init.Period = 19999U;
    htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    if (HAL_TIM_Base_Init(&htim3) != HAL_OK) Error_Handler();
    sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
    if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK) Error_Handler();
    if (HAL_TIM_PWM_Init(&htim3) != HAL_OK) Error_Handler();
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK) Error_Handler();
    sConfigOC.OCMode = TIM_OCMODE_PWM1;
    sConfigOC.Pulse = RC_NEUTRAL_US;
    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
    if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_1) != HAL_OK) Error_Handler();
    if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_2) != HAL_OK) Error_Handler();
}

/* Initialize one remote-control input channel. */
void Car_RC_ChannelInit(CarRc_Channel_t *rc)
{
    rc->rise = 0;
    rc->pulse_us = RC_NEUTRAL_US;
    rc->last_ms = 0;
    rc->last_capture = 0;
    rc->raw_width_us = 0;
    rc->candidate_width_us = 0;
    rc->edge_count = 0;
    rc->capture_count = 0;
    rc->candidate_count = 0;
    rc->wait_fall = 0;
}

/* Start remote-control input capture and car PWM output. */
void Car_Start(void)
{
    Car_RC_ChannelInit(&car_rc_ch1);
    Car_RC_ChannelInit(&car_rc_ch2);
    Car_RC_ChannelInit(&work_switch_rc);
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);
    Car_PWM_OutputNeutral();
    __HAL_TIM_SET_CAPTUREPOLARITY(&htim2, TIM_CHANNEL_1, TIM_INPUTCHANNELPOLARITY_RISING);
    __HAL_TIM_SET_CAPTUREPOLARITY(&htim8, TIM_CHANNEL_1, TIM_INPUTCHANNELPOLARITY_RISING);
    __HAL_TIM_SET_CAPTUREPOLARITY(&htim8, TIM_CHANNEL_2, TIM_INPUTCHANNELPOLARITY_RISING);
    HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_1);
    HAL_TIM_IC_Start_IT(&htim8, TIM_CHANNEL_1);
    HAL_TIM_IC_Start_IT(&htim8, TIM_CHANNEL_2);
}

void Car_StartRowContinueInput(void)
{
    Car_RC_ChannelInit(&row_continue_rc);
    __HAL_TIM_SET_CAPTUREPOLARITY(&htim12, TIM_CHANNEL_1, TIM_INPUTCHANNELPOLARITY_RISING);
    HAL_TIM_IC_Start_IT(&htim12, TIM_CHANNEL_1);
}

void Car_StartSpeedInput(void)
{
    Car_RC_ChannelInit(&speed_rc);
}

void Car_AutoMode_Start(void)
{
    Car_RC_ChannelInit(&car_rc_ch1);
    Car_RC_ChannelInit(&car_rc_ch2);
    car_gpio_ch1_center_us = RC_NEUTRAL_US;
    car_gpio_ch2_center_us = RC_NEUTRAL_US;
    car_gpio_calib_start_ms = 0U;
    car_gpio_calib_ch1_sum = 0U;
    car_gpio_calib_ch2_sum = 0U;
    car_gpio_calib_count = 0U;
    car_gpio_calib_active = 0U;
    car_gpio_calib_done = 0U;
    __HAL_TIM_SET_CAPTUREPOLARITY(&htim2, TIM_CHANNEL_1, TIM_INPUTCHANNELPOLARITY_RISING);
    __HAL_TIM_SET_CAPTUREPOLARITY(&htim8, TIM_CHANNEL_1, TIM_INPUTCHANNELPOLARITY_RISING);
    HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_1);
    HAL_TIM_IC_Start_IT(&htim8, TIM_CHANNEL_1);
}

static void Car_MotorPWM_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    TIM_ClockConfigTypeDef sClockSourceConfig = {0};
    TIM_MasterConfigTypeDef sMasterConfig = {0};
    TIM_OC_InitTypeDef sConfigOC = {0};

    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_TIM4_CLK_ENABLE();

    /* TIM4 PWM pins are unused by the robot workflow and are easy to wire out. */
    GPIO_InitStruct.Pin = GPIO_PIN_12 | GPIO_PIN_13;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF2_TIM4;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

    car_motor_pwm_tim.Instance = TIM4;
    car_motor_pwm_tim.Init.Prescaler = 83U;
    car_motor_pwm_tim.Init.CounterMode = TIM_COUNTERMODE_UP;
    car_motor_pwm_tim.Init.Period = CAR_MOTOR_PWM_PERIOD_US - 1U;
    car_motor_pwm_tim.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    car_motor_pwm_tim.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    if (HAL_TIM_Base_Init(&car_motor_pwm_tim) != HAL_OK) Error_Handler();

    sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
    if (HAL_TIM_ConfigClockSource(&car_motor_pwm_tim, &sClockSourceConfig) != HAL_OK) Error_Handler();
    if (HAL_TIM_PWM_Init(&car_motor_pwm_tim) != HAL_OK) Error_Handler();

    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(&car_motor_pwm_tim, &sMasterConfig) != HAL_OK)
    {
        Error_Handler();
    }

    sConfigOC.OCMode = TIM_OCMODE_PWM1;
    sConfigOC.Pulse = 0U;
    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
    if (HAL_TIM_PWM_ConfigChannel(&car_motor_pwm_tim, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
    {
        Error_Handler();
    }
    if (HAL_TIM_PWM_ConfigChannel(&car_motor_pwm_tim, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
    {
        Error_Handler();
    }

    if (HAL_TIM_PWM_Start(&car_motor_pwm_tim, TIM_CHANNEL_1) != HAL_OK) Error_Handler();
    if (HAL_TIM_PWM_Start(&car_motor_pwm_tim, TIM_CHANNEL_2) != HAL_OK) Error_Handler();

    __HAL_TIM_SET_COMPARE(&car_motor_pwm_tim, TIM_CHANNEL_1, 0U);
    __HAL_TIM_SET_COMPARE(&car_motor_pwm_tim, TIM_CHANNEL_2, 0U);
    car_motor_pwm_ready = 1U;
}

static uint32_t Car_MotorPWM_DutyToPulse(uint32_t duty_percent)
{
    if (duty_percent > 100U)
    {
        duty_percent = 100U;
    }
    return (CAR_MOTOR_PWM_PERIOD_US * duty_percent) / 100U;
}

static void Car_MotorPWM_SetLeftDuty(uint32_t duty_percent)
{
    if (car_motor_pwm_ready == 0U)
    {
        return;
    }
    __HAL_TIM_SET_COMPARE(&car_motor_pwm_tim,
                          TIM_CHANNEL_1,
                          Car_MotorPWM_DutyToPulse(duty_percent));
}

static void Car_MotorPWM_SetRightDuty(uint32_t duty_percent)
{
    if (car_motor_pwm_ready == 0U)
    {
        return;
    }
    __HAL_TIM_SET_COMPARE(&car_motor_pwm_tim,
                          TIM_CHANNEL_2,
                          Car_MotorPWM_DutyToPulse(duty_percent));
}

static void Car_MotorPWM_AllStop(void)
{
    Car_MotorPWM_SetLeftDuty(0U);
    Car_MotorPWM_SetRightDuty(0U);
}

static uint32_t Car_MotorPWM_ClampRunDuty(uint32_t duty_percent)
{
    if (duty_percent < CAR_MOTOR_PWM_MIN_DUTY_PERCENT)
    {
        return CAR_MOTOR_PWM_MIN_DUTY_PERCENT;
    }
    if (duty_percent > CAR_MOTOR_PWM_MAX_DUTY_PERCENT)
    {
        return CAR_MOTOR_PWM_MAX_DUTY_PERCENT;
    }
    return duty_percent;
}

void Car_Motor_SetDefaultRunDuty(uint32_t duty_percent)
{
    car_motor_default_run_duty_percent = Car_MotorPWM_ClampRunDuty(duty_percent);
}

uint32_t Car_Motor_GetDefaultRunDuty(void)
{
    return car_motor_default_run_duty_percent;
}

void Car_Motor_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    HAL_GPIO_WritePin(CAR_GPIO_LEFT_FWD_GPIO_PORT, CAR_GPIO_LEFT_FWD_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(CAR_GPIO_LEFT_REV_GPIO_PORT, CAR_GPIO_LEFT_REV_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(CAR_GPIO_RIGHT_FWD_GPIO_PORT, CAR_GPIO_RIGHT_FWD_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(CAR_GPIO_RIGHT_REV_GPIO_PORT, CAR_GPIO_RIGHT_REV_PIN, GPIO_PIN_SET);

    GPIO_InitStruct.Pin = CAR_GPIO_LEFT_FWD_PIN | CAR_GPIO_RIGHT_REV_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = CAR_GPIO_LEFT_REV_PIN | CAR_GPIO_RIGHT_FWD_PIN;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    Car_MotorPWM_Init();
}

static int8_t Car_GPIO_ClampDir(int32_t dir)
{
    if (dir > 0) return 1;
    if (dir < 0) return -1;
    return 0;
}

static void Car_Motor_SetLeftDirOnly(int8_t dir)
{
    HAL_GPIO_WritePin(CAR_GPIO_LEFT_FWD_GPIO_PORT, CAR_GPIO_LEFT_FWD_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(CAR_GPIO_LEFT_REV_GPIO_PORT, CAR_GPIO_LEFT_REV_PIN, GPIO_PIN_SET);

    if (dir > 0)
    {
        HAL_GPIO_WritePin(CAR_GPIO_LEFT_REV_GPIO_PORT, CAR_GPIO_LEFT_REV_PIN, GPIO_PIN_RESET);
    }
    else if (dir < 0)
    {
        HAL_GPIO_WritePin(CAR_GPIO_LEFT_FWD_GPIO_PORT, CAR_GPIO_LEFT_FWD_PIN, GPIO_PIN_RESET);
    }
}

static void Car_Motor_SetRightDirOnly(int8_t dir)
{
    HAL_GPIO_WritePin(CAR_GPIO_RIGHT_FWD_GPIO_PORT, CAR_GPIO_RIGHT_FWD_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(CAR_GPIO_RIGHT_REV_GPIO_PORT, CAR_GPIO_RIGHT_REV_PIN, GPIO_PIN_SET);

    if (dir > 0)
    {
        HAL_GPIO_WritePin(CAR_GPIO_RIGHT_REV_GPIO_PORT, CAR_GPIO_RIGHT_REV_PIN, GPIO_PIN_RESET);
    }
    else if (dir < 0)
    {
        HAL_GPIO_WritePin(CAR_GPIO_RIGHT_FWD_GPIO_PORT, CAR_GPIO_RIGHT_FWD_PIN, GPIO_PIN_RESET);
    }
}

void Car_Motor_SetLeft(int8_t dir)
{
    Car_Motor_SetLeftDirOnly(dir);
    /* Non-joystick callers, such as automatic row changes, use the saved speed gear. */
    Car_MotorPWM_SetLeftDuty((dir == 0) ? 0U : car_motor_default_run_duty_percent);
}

void Car_Motor_SetRight(int8_t dir)
{
    Car_Motor_SetRightDirOnly(dir);
    /* Non-joystick callers, such as automatic row changes, use the saved speed gear. */
    Car_MotorPWM_SetRightDuty((dir == 0) ? 0U : car_motor_default_run_duty_percent);
}

void Car_Motor_Stop(void)
{
    Car_Motor_SetLeftDirOnly(0);
    Car_Motor_SetRightDirOnly(0);
    Car_MotorPWM_AllStop();
}

static uint8_t Car_GPIO_HasFreshInput(uint32_t now)
{
    return (((now - car_rc_ch1.last_ms) < RC_TIMEOUT_MS) &&
            ((now - car_rc_ch2.last_ms) < RC_TIMEOUT_MS));
}

static void Car_GPIO_UpdateCenterCalib(uint32_t now)
{
    if (car_gpio_calib_done != 0U) return;

    Car_Motor_Stop();

    if (Car_GPIO_HasFreshInput(now) == 0U)
    {
        car_gpio_calib_active = 0U;
        car_gpio_calib_start_ms = 0U;
        car_gpio_calib_ch1_sum = 0U;
        car_gpio_calib_ch2_sum = 0U;
        car_gpio_calib_count = 0U;
        return;
    }

    if (car_gpio_calib_active == 0U)
    {
        car_gpio_calib_active = 1U;
        car_gpio_calib_start_ms = now;
    }

    car_gpio_calib_ch1_sum += car_rc_ch1.pulse_us;
    car_gpio_calib_ch2_sum += car_rc_ch2.pulse_us;
    car_gpio_calib_count++;

    if ((now - car_gpio_calib_start_ms) >= RC_CENTER_CALIB_MS)
    {
        if (car_gpio_calib_count > 0U)
        {
            car_gpio_ch1_center_us = car_gpio_calib_ch1_sum / car_gpio_calib_count;
            car_gpio_ch2_center_us = car_gpio_calib_ch2_sum / car_gpio_calib_count;
        }
        car_gpio_calib_done = 1U;
    }
}

static int32_t Car_GPIO_RCToSigned(uint32_t pulse_us, uint32_t center_us)
{
    int32_t value;
    uint32_t span;

    if (pulse_us > (center_us + RC_CENTER_DEADBAND_US))
    {
        span = RC_MAX_US - (center_us + RC_CENTER_DEADBAND_US);
        if (span == 0U)
        {
            return 0L;
        }
        value = ((int32_t)(pulse_us - center_us - RC_CENTER_DEADBAND_US) *
                 CAR_MOTOR_PWM_FULL_SCALE) / (int32_t)span;
    }
    else if ((pulse_us + RC_CENTER_DEADBAND_US) < center_us)
    {
        span = (center_us - RC_CENTER_DEADBAND_US) - RC_MIN_US;
        if (span == 0U)
        {
            return 0L;
        }
        value = -(((int32_t)(center_us - RC_CENTER_DEADBAND_US - pulse_us) *
                   CAR_MOTOR_PWM_FULL_SCALE) / (int32_t)span);
    }
    else
    {
        return 0L;
    }

    if (value > CAR_MOTOR_PWM_FULL_SCALE)
    {
        value = CAR_MOTOR_PWM_FULL_SCALE;
    }
    if (value < -CAR_MOTOR_PWM_FULL_SCALE)
    {
        value = -CAR_MOTOR_PWM_FULL_SCALE;
    }
    return value;
}

static uint32_t Car_MotorPWM_ValueToDuty(int32_t value)
{
    if (value < 0L)
    {
        value = -value;
    }
    if (value == 0L)
    {
        return 0U;
    }
    if (value > CAR_MOTOR_PWM_FULL_SCALE)
    {
        value = CAR_MOTOR_PWM_FULL_SCALE;
    }

    return CAR_MOTOR_PWM_MIN_DUTY_PERCENT +
           (uint32_t)((value *
                       (int32_t)(CAR_MOTOR_PWM_MAX_DUTY_PERCENT -
                                 CAR_MOTOR_PWM_MIN_DUTY_PERCENT)) /
                      CAR_MOTOR_PWM_FULL_SCALE);
}

void Car_AutoMode_Update(void)
{
    uint32_t now = HAL_GetTick();
    int32_t throttle;
    int32_t steering;
    int32_t throttle_mag;
    int32_t turn_scaled;
    int32_t left_value;
    int32_t right_value;
    int8_t left_dir;
    int8_t right_dir;

    Car_GPIO_UpdateCenterCalib(now);
    if (car_gpio_calib_done == 0U) return;

    if (Car_GPIO_HasFreshInput(now) == 0U)
    {
        Car_Motor_Stop();
        return;
    }

    /* PA0 steering direction is reversed here to match the real joystick/car wiring. */
    steering = -Car_GPIO_RCToSigned(car_rc_ch1.pulse_us, car_gpio_ch1_center_us);
    throttle = Car_GPIO_RCToSigned(car_rc_ch2.pulse_us, car_gpio_ch2_center_us);

    if (throttle != 0L)
    {
        /* Curvature-style differential mixing:
         * PA0 chooses the left/right arc. PC6 only chooses forward/reverse speed,
         * so right-up and right-down both curve right, and left-up/left-down both
         * curve left.
         */
        throttle_mag = (throttle < 0L) ? -throttle : throttle;
        turn_scaled = (throttle_mag * steering) / CAR_MOTOR_PWM_FULL_SCALE;
        left_value = throttle + turn_scaled;
        right_value = throttle - turn_scaled;
    }
    else
    {
        left_value = steering;
        right_value = -steering;
    }

    if (left_value > CAR_MOTOR_PWM_FULL_SCALE) left_value = CAR_MOTOR_PWM_FULL_SCALE;
    if (left_value < -CAR_MOTOR_PWM_FULL_SCALE) left_value = -CAR_MOTOR_PWM_FULL_SCALE;
    if (right_value > CAR_MOTOR_PWM_FULL_SCALE) right_value = CAR_MOTOR_PWM_FULL_SCALE;
    if (right_value < -CAR_MOTOR_PWM_FULL_SCALE) right_value = -CAR_MOTOR_PWM_FULL_SCALE;

    left_dir = Car_GPIO_ClampDir(left_value);
    right_dir = Car_GPIO_ClampDir(right_value);

    Car_Motor_SetLeftDirOnly(left_dir);
    Car_Motor_SetRightDirOnly(right_dir);
    Car_MotorPWM_SetLeftDuty(Car_MotorPWM_ValueToDuty(left_value));
    Car_MotorPWM_SetRightDuty(Car_MotorPWM_ValueToDuty(right_value));
}

/* Start PWM output plus the PC7 work switch, without PA0/PC6 manual captures. */
void Car_StartWorkSwitchOnly(void)
{
    Car_RC_ChannelInit(&car_rc_ch1);
    Car_RC_ChannelInit(&car_rc_ch2);
    Car_RC_ChannelInit(&work_switch_rc);
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);
    Car_PWM_OutputNeutral();
    __HAL_TIM_SET_CAPTUREPOLARITY(&htim8, TIM_CHANNEL_2, TIM_INPUTCHANNELPOLARITY_RISING);
    HAL_TIM_IC_Start_IT(&htim8, TIM_CHANNEL_2);
}

static int32_t Car_Abs32(int32_t value)
{
    return (value < 0) ? -value : value;
}

static void Car_ConvertRcToCarCoord(uint32_t ch1_us, uint32_t ch2_us,
                                    int32_t *y_value, int32_t *x_value)
{
    int32_t ch1 = (int32_t)ch1_us - (int32_t)RC_NEUTRAL_US;
    int32_t ch2 = (int32_t)ch2_us - (int32_t)RC_NEUTRAL_US;

    *x_value = ((ch2 - ch1) * 707L) / 1000L;
    *y_value = ((ch2 + ch1) * 707L) / 1000L;
}

static uint32_t Car_RC_SignedToPulse(int32_t value)
{
    int32_t mag;

    if (value >= -(int32_t)RC_DEAD_ZONE && value <= (int32_t)RC_DEAD_ZONE)
    {
        return RC_NEUTRAL_US;
    }

    if (value > 0)
    {
        uint32_t span = RC_MANUAL_HIGH_LIMIT_US - RC_MANUAL_FORWARD_START_US;

        if (value > RC_MANUAL_COORD_MAX) value = RC_MANUAL_COORD_MAX;
        mag = value - (int32_t)RC_DEAD_ZONE;
        if (mag < 0) mag = 0;
        return RC_MANUAL_FORWARD_START_US +
               ((uint32_t)mag * span) /
               (uint32_t)(RC_MANUAL_COORD_MAX - (int32_t)RC_DEAD_ZONE);
    }

    if (value < -RC_MANUAL_COORD_MAX) value = -RC_MANUAL_COORD_MAX;
    mag = -value - (int32_t)RC_DEAD_ZONE;
    if (mag < 0) mag = 0;
    return RC_MANUAL_REVERSE_START_US -
           ((uint32_t)mag * (RC_MANUAL_REVERSE_START_US - RC_MANUAL_LOW_LIMIT_US)) /
           (uint32_t)(RC_MANUAL_COORD_MAX - (int32_t)RC_DEAD_ZONE);
}

/* Limit RC pulse width before forwarding it to car PWM output. */
uint32_t Car_RC_Limit(uint32_t value)
{
    uint32_t active;
    uint32_t input_span;
    uint32_t output_span;

    if (value < RC_MIN_US || value > RC_MAX_US) return RC_NEUTRAL_US;
    if (value >= (RC_NEUTRAL_US - RC_DEAD_ZONE) &&
        value <= (RC_NEUTRAL_US + RC_DEAD_ZONE)) return RC_NEUTRAL_US;
    if (value < RC_NEUTRAL_US) return (value < RC_MANUAL_LOW_LIMIT_US) ? RC_MANUAL_LOW_LIMIT_US : value;

    if (value > RC_MANUAL_HIGH_LIMIT_US)
    {
        return RC_MANUAL_HIGH_LIMIT_US;
    }

    active = value - (RC_NEUTRAL_US + RC_DEAD_ZONE);
    input_span = RC_MANUAL_HIGH_LIMIT_US - (RC_NEUTRAL_US + RC_DEAD_ZONE);
    output_span = RC_MANUAL_HIGH_LIMIT_US - RC_MANUAL_FORWARD_START_US;
    if (input_span == 0U)
    {
        return RC_MANUAL_FORWARD_START_US;
    }

    return RC_MANUAL_FORWARD_START_US + (active * output_span) / input_span;
}

void Car_RC_CalcManualOutput(uint32_t *left_us, uint32_t *right_us)
{
    uint32_t now = HAL_GetTick();
    uint32_t ch1_right = RC_NEUTRAL_US;
    uint32_t ch2_left = RC_NEUTRAL_US;

    if (((now - car_rc_ch1.last_ms) < RC_TIMEOUT_MS) &&
        ((now - car_rc_ch2.last_ms) < RC_TIMEOUT_MS))
    {
        int32_t y;
        int32_t x;
        int32_t right_cmd;
        int32_t left_cmd;
        int32_t max_cmd;

        Car_ConvertRcToCarCoord(car_rc_ch1.pulse_us, car_rc_ch2.pulse_us, &y, &x);
        right_cmd = y - x;
        left_cmd = y + x;

        max_cmd = Car_Abs32(right_cmd);
        if (Car_Abs32(left_cmd) > max_cmd) max_cmd = Car_Abs32(left_cmd);
        if (max_cmd > RC_MANUAL_COORD_MAX)
        {
            right_cmd = (right_cmd * RC_MANUAL_COORD_MAX) / max_cmd;
            left_cmd = (left_cmd * RC_MANUAL_COORD_MAX) / max_cmd;
        }

        right_cmd = (right_cmd * RC_MANUAL_SPEED_PERCENT) / 100L;
        left_cmd = (left_cmd * RC_MANUAL_SPEED_PERCENT) / 100L;

        ch1_right = Car_RC_SignedToPulse(right_cmd);
        ch2_left = Car_RC_SignedToPulse(left_cmd);
    }

    *left_us = ch1_right;
    *right_us = ch2_left;
}

static uint8_t Car_WorkSwitchAutoActive(void)
{
    uint32_t now = HAL_GetTick();

    if ((now - work_switch_rc.last_ms) >= RC_TIMEOUT_MS)
    {
        return 0U;
    }

    return (work_switch_rc.pulse_us >= WORK_SWITCH_THRESHOLD_US) ? 1U : 0U;
}

uint8_t Car_RC_OverrideActive(void)
{
    uint32_t ch1 = RC_NEUTRAL_US;
    uint32_t ch2 = RC_NEUTRAL_US;

    if (Car_WorkSwitchAutoActive() != 0U)
    {
        return 0U;
    }

    Car_RC_CalcManualOutput(&ch1, &ch2);
    return (ch1 != RC_NEUTRAL_US || ch2 != RC_NEUTRAL_US) ? 1U : 0U;
}

void Car_PWM_OutputNeutral(void)
{
    Car_PWM_OutputSet(RC_NEUTRAL_US, RC_NEUTRAL_US);
}

void Car_PWM_OutputSet(uint32_t ch1_us, uint32_t ch2_us)
{
    if (ch1_us < RC_MIN_US) ch1_us = RC_MIN_US;
    if (ch1_us > RC_MAX_US) ch1_us = RC_MAX_US;
    if (ch2_us < RC_MIN_US) ch2_us = RC_MIN_US;
    if (ch2_us > RC_MAX_US) ch2_us = RC_MAX_US;
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, ch2_us);
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, ch1_us);
}

void Car_PWM_OutputUpdate(void)
{
    uint32_t out1 = RC_NEUTRAL_US;
    uint32_t out2 = RC_NEUTRAL_US;

    if (Car_WorkSwitchAutoActive() != 0U)
    {
        return;
    }

    Car_RC_CalcManualOutput(&out1, &out2);
    Car_PWM_OutputSet(out1, out2);
}

void Car_PWM_OutputUpdateManual(void)
{
    uint32_t out1 = RC_NEUTRAL_US;
    uint32_t out2 = RC_NEUTRAL_US;

    Car_RC_CalcManualOutput(&out1, &out2);
    Car_PWM_OutputSet(out1, out2);
}

static uint32_t Car_RCPulseDiff(uint32_t a, uint32_t b)
{
    return (a > b) ? (a - b) : (b - a);
}

static void Car_RC_AcceptPulse(CarRc_Channel_t *rc, uint32_t width, uint32_t now)
{
    rc->pulse_us = width;
    rc->last_ms = now;
    rc->capture_count++;
    rc->candidate_width_us = 0U;
    rc->candidate_count = 0U;
}

void Car_RC_ProcessCapture(TIM_HandleTypeDef *htim, uint32_t tim_channel, CarRc_Channel_t *rc)
{
    uint32_t cap = HAL_TIM_ReadCapturedValue(htim, tim_channel);
    rc->last_capture = cap;
    rc->edge_count++;

    if (rc->wait_fall == 0U)
    {
        rc->rise = cap;
        rc->wait_fall = 1U;
        __HAL_TIM_SET_CAPTUREPOLARITY(htim, tim_channel, TIM_INPUTCHANNELPOLARITY_FALLING);
    }
    else
    {
        uint32_t width;
        uint32_t arr = __HAL_TIM_GET_AUTORELOAD(htim);

        if (cap >= rc->rise) width = cap - rc->rise;
        else width = (arr + 1U - rc->rise) + cap;
        rc->wait_fall = 0U;
        __HAL_TIM_SET_CAPTUREPOLARITY(htim, tim_channel, TIM_INPUTCHANNELPOLARITY_RISING);
        rc->raw_width_us = width;
        if (width >= RC_MIN_US && width <= RC_MAX_US)
        {
            uint32_t now = HAL_GetTick();

            if (rc->last_ms == 0U ||
                ((now - rc->last_ms) >= RC_TIMEOUT_MS) ||
                (Car_RCPulseDiff(width, rc->pulse_us) <= RC_GLITCH_REJECT_US))
            {
                Car_RC_AcceptPulse(rc, width, now);
            }
            else if (rc->candidate_count == 0U ||
                     Car_RCPulseDiff(width, rc->candidate_width_us) > RC_GLITCH_REJECT_US)
            {
                rc->candidate_width_us = width;
                rc->candidate_count = 1U;
            }
            else
            {
                rc->candidate_count++;
                if (rc->candidate_count >= RC_GLITCH_CONFIRM_COUNT)
                {
                    Car_RC_AcceptPulse(rc, width, now);
                }
            }
        }
    }
}


