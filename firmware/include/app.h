#ifndef APP_H
#define APP_H

#include "stm32f4xx_hal.h"
#include "./BSP/LCD/lcd.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef APP_USE_FREERTOS
#define APP_USE_FREERTOS 1U
#endif

/* Seedling feeding device stepper pins. */
#define SEEDLING_STEP_GPIO_PORT GPIOB
#define SEEDLING_STEP_PIN       GPIO_PIN_0
#define SEEDLING_DIR_GPIO_PORT  GPIOB
#define SEEDLING_DIR_PIN        GPIO_PIN_1

/* Moving device stepper pins. */
#define MOBILE_STEP_GPIO_PORT GPIOC
#define MOBILE_STEP_PIN       GPIO_PIN_11
#define MOBILE_DIR_GPIO_PORT  GPIOC
#define MOBILE_DIR_PIN        GPIO_PIN_12

#define FORWARD        0U
#define REVERSE        1U
#define DIR_FORWARD_LEVEL   GPIO_PIN_RESET
#define DIR_REVERSE_LEVEL   GPIO_PIN_SET
#define STEP_HIGH_US        10U
#define START_FREQ_HZ       900U
#define RUN_FREQ_HZ         6000U
#define SEEDLING_TESTED_FREQ_HZ 7200U
#define MOBILE_TESTED_FREQ_HZ   2400U
#define MOBILE_STEP_LOW_US      100U
#define ACCEL_TIME_MS       2500U
#define FORWARD_RUN_MS      10000U
#define REVERSE_RUN_MS      10000U
#define STOP_MS             2000U
#define DIR_STABLE_MS       300U
#define STOP_STABLE_MS      200U
#define STEPPER_DEMO_ENABLE 1U

#define DRILL_IN1_GPIO_PORT GPIOG
#define DRILL_IN1_PIN       GPIO_PIN_2
#define DRILL_IN2_GPIO_PORT GPIOG
#define DRILL_IN2_PIN       GPIO_PIN_3

/* Electric drill linear rod pins. */
#define DRILL_ROD_IN1_GPIO_PORT GPIOG
#define DRILL_ROD_IN1_PIN       GPIO_PIN_4
#define DRILL_ROD_IN2_GPIO_PORT GPIOG
#define DRILL_ROD_IN2_PIN       GPIO_PIN_5

#define CONVEYOR_IN1_GPIO_PORT GPIOG
#define CONVEYOR_IN1_PIN       GPIO_PIN_6
#define CONVEYOR_IN2_GPIO_PORT GPIOG
#define CONVEYOR_IN2_PIN       GPIO_PIN_7

#define COVER_IN1_GPIO_PORT GPIOC
#define COVER_IN1_PIN       GPIO_PIN_8
#define COVER_IN2_GPIO_PORT GPIOC
#define COVER_IN2_PIN       GPIO_PIN_9

#define PUMP_RELAY_GPIO_PORT GPIOC
#define PUMP_RELAY_PIN       GPIO_PIN_10
#define PUMP_RELAY_ON_LEVEL  GPIO_PIN_SET
#define PUMP_RELAY_OFF_LEVEL GPIO_PIN_RESET

#define DRILL_ROD_DOWN_MS   20000U
#define DRILL_ROD_STOP_MS   2000U
#define DRILL_ROD_UP_MS     20000U
#define CONVEYOR_FIRST_FORWARD_MS 1450U
#define CONVEYOR_NEXT_FORWARD_MS  500U
#define CONVEYOR_STOP_MS         1000U
#define CONVEYOR_REVERSE_MS      5000U
#define COVER_FORWARD_MS    10000U
#define COVER_STOP_MS       1000U
#define COVER_REVERSE_MS    10000U
#define PUMP_ON_MS 10000U
#define PUMP_OFF_MS         2000U

#define WORK_DRILL_ROD_DOWN_ONLY_MS   10000U
#define WORK_DRILL_WITH_DOWN_MS       5000U
#define WORK_DRILL_WITH_UP_MS         1000U
#define WORK_DRILL_ROD_UP_ONLY_MS     15000U

#define WORK_MOBILE_RUN_MS       8000U
#define WORK_SEEDLING_FORWARD_MS 7000U
#define WORK_SEEDLING_REVERSE_MS 28000U
#define WORK_CONVEYOR_REVERSE_MS CONVEYOR_REVERSE_MS
#define WORK_COVER_FORWARD_MS    10000U
#define WORK_COVER_STOP_MS       1000U
#define WORK_COVER_REVERSE_MS    10000U
#define WORK_CAR_FORWARD_MS      10000U
#define WORK_CAR_RAMP_MS         2000U
#define WORK_CAR_HOLD_START_MS   2000U
#define WORK_CAR_HOLD_END_MS     8000U
#define WORK_CAR_NEUTRAL_US      RC_NEUTRAL_US
#define WORK_CAR_FORWARD_US      1600U
#define WORK_ROW_FORWARD_MS      2000U
#define WORK_ROW_TURN_RIGHT_US   1500U
#define WORK_ROW_TURN_DEG_X100   9000L

#define RC_MIN_US          800U
#define RC_MAX_US          2200U
#define RC_NEUTRAL_US      1465U
#define RC_TIMEOUT_MS      500U
#define RC_GLITCH_REJECT_US 250U
#define RC_GLITCH_CONFIRM_COUNT 2U
#define RC_DEAD_ZONE       60U
#define RC_CENTER_DEADBAND_US  100U
#define RC_CENTER_CALIB_MS     1500U
#define RC_LOW_LIMIT_US    1375U
#define RC_HIGH_LIMIT_US   1625U
#define RC_MANUAL_LOW_LIMIT_US  1265U
#define RC_MANUAL_HIGH_LIMIT_US 1615U
#define RC_MANUAL_REVERSE_START_US 1365U
#define RC_MANUAL_FORWARD_START_US 1535U
#define RC_MANUAL_COORD_MAX     700L
#define RC_MANUAL_SPEED_PERCENT 100L
#define WORK_SWITCH_THRESHOLD_US 1500U
#ifndef WORK_RC_SWITCH_ENABLE
#define WORK_RC_SWITCH_ENABLE 0U
#endif

/* YMR8-F direction GPIO car control, from the manual car test project. */
#define CAR_GPIO_LEFT_FWD_GPIO_PORT   GPIOA
#define CAR_GPIO_LEFT_FWD_PIN         GPIO_PIN_5
#define CAR_GPIO_LEFT_REV_GPIO_PORT   GPIOB
#define CAR_GPIO_LEFT_REV_PIN         GPIO_PIN_1
#define CAR_GPIO_RIGHT_FWD_GPIO_PORT  GPIOB
#define CAR_GPIO_RIGHT_FWD_PIN        GPIO_PIN_12
#define CAR_GPIO_RIGHT_REV_GPIO_PORT  GPIOA
#define CAR_GPIO_RIGHT_REV_PIN        GPIO_PIN_6

#define LCD_BG_COLOR       WHITE
#define LCD_TITLE_COLOR    BLUE
#define LCD_TEXT_COLOR     BLACK
#define LCD_VALUE_COLOR    RED
#define LCD_OK_COLOR       GREEN
#define LCD_WARN_COLOR     RED

#define UART_BAUDRATE      115200U
#define UART2_BAUDRATE     115200U
#define UART1_RX_BUF_SIZE  96U
#define UART1_TX_BUF_SIZE  256U
#define WORKFLOW_STATUS_BUF_SIZE 96U
#define UART2_RX_BUF_SIZE  128U
#define UART2_TX_BUF_SIZE  256U
#define UART3_RX_BUF_SIZE  96U
#define UART3_TX_BUF_SIZE  128U
#define IMU_I2C_ADDR       0x23U
#define IMU_WIT_I2C_ADDR   0x50U
#define IMU_REG_VERSION    0x01U
#define IMU_REG_EULER      0x26U
#define IMU_WIT_REG_ANGLE  0x3DU

#define ULTRASONIC_TRIG_GPIO_PORT GPIOF
#define ULTRASONIC_TRIG_PIN       GPIO_PIN_6
#define ULTRASONIC_ECHO_GPIO_PORT GPIOF
#define ULTRASONIC_ECHO_PIN       GPIO_PIN_7
#define ULTRASONIC_PERIOD_MS      100U
#define ULTRASONIC_TIMEOUT_US     12000U
#define ULTRASONIC_VALID_MS       5000U
#define ULTRASONIC_MIN_ECHO_US    120U
#define ULTRASONIC_BLANK_US       250U
#define ULTRASONIC_FILTER_SIZE    5U

typedef struct
{
    volatile uint32_t rise;
    volatile uint32_t pulse_us;
    volatile uint32_t last_ms;
    volatile uint32_t last_capture;
    volatile uint32_t raw_width_us;
    volatile uint32_t candidate_width_us;
    volatile uint32_t edge_count;
    volatile uint32_t capture_count;
    volatile uint8_t candidate_count;
    volatile uint8_t wait_fall;
} CarRc_Channel_t;

extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim8;
extern TIM_HandleTypeDef htim12;
extern TIM_HandleTypeDef htim1;
extern I2C_HandleTypeDef hi2c1;
extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;
extern UART_HandleTypeDef huart3;
extern CarRc_Channel_t car_rc_ch1;
extern CarRc_Channel_t car_rc_ch2;
extern CarRc_Channel_t work_switch_rc;
extern CarRc_Channel_t row_continue_rc;
extern CarRc_Channel_t speed_rc;
extern volatile uint8_t workflow_paused;
extern volatile uint8_t workflow_completed;
extern volatile uint8_t lcd_ui_ready;
extern uint8_t uart1_rx_byte;
extern volatile uint32_t uart1_rx_count;
extern volatile uint32_t uart1_error_count;
extern volatile uint32_t uart1_last_error;
extern volatile uint8_t uart1_last_byte;
extern volatile uint8_t uart1_rx_event;
extern char uart1_lcd_buf[UART1_RX_BUF_SIZE];
extern volatile uint16_t uart1_rx_index;
extern uint8_t uart1_tx_buf[UART1_TX_BUF_SIZE];
extern volatile uint16_t uart1_tx_head;
extern volatile uint16_t uart1_tx_tail;
extern volatile uint32_t uart1_tx_count;
extern volatile uint32_t uart1_tx_drop_count;
extern volatile uint32_t radar_front_distance_cm;
extern volatile uint32_t radar_left_distance_cm;
extern volatile uint32_t radar_right_distance_cm;
extern volatile uint32_t radar_last_ms;
extern volatile uint8_t radar_front_valid;
extern volatile uint8_t radar_left_valid;
extern volatile uint8_t radar_right_valid;
extern char workflow_status_buf[WORKFLOW_STATUS_BUF_SIZE];
extern volatile uint16_t workflow_step_index;
extern volatile uint8_t workflow_round_index;
extern volatile uint8_t workflow_group_index;
extern uint8_t uart2_rx_byte;
extern volatile uint32_t uart2_rx_count;
extern volatile uint32_t uart2_error_count;
extern volatile uint32_t uart2_last_error;
extern volatile uint8_t uart2_last_byte;
extern volatile uint8_t uart2_rx_event;
extern char uart2_lcd_buf[UART2_RX_BUF_SIZE];
extern volatile uint16_t uart2_rx_index;
extern char uart2_tx_lcd_buf[UART2_RX_BUF_SIZE];
extern volatile uint16_t uart2_tx_lcd_index;
extern volatile uint8_t uart2_tx_event;
extern uint8_t uart2_tx_buf[UART2_TX_BUF_SIZE];
extern volatile uint16_t uart2_tx_head;
extern volatile uint16_t uart2_tx_tail;
extern volatile uint32_t uart2_tx_count;
extern volatile uint32_t uart2_tx_drop_count;
extern uint8_t uart3_rx_byte;
extern volatile uint32_t uart3_rx_count;
extern volatile uint32_t uart3_error_count;
extern volatile uint32_t uart3_last_error;
extern volatile uint8_t uart3_last_byte;
extern volatile uint8_t uart3_rx_event;
extern char uart3_lcd_buf[UART3_RX_BUF_SIZE];
extern volatile uint16_t uart3_rx_index;
extern uint8_t uart3_tx_buf[UART3_TX_BUF_SIZE];
extern volatile uint16_t uart3_tx_head;
extern volatile uint16_t uart3_tx_tail;
extern volatile uint32_t uart3_tx_count;
extern volatile uint32_t uart3_tx_drop_count;
extern volatile uint8_t imu_i2c_found_addr;
extern volatile uint8_t imu_i2c_addr_list[4];
extern volatile uint8_t imu_i2c_found_count;
extern volatile uint8_t imu_i2c_pinset;
extern volatile uint8_t imu_i2c_scl_level;
extern volatile uint8_t imu_i2c_sda_level;
extern volatile uint32_t imu_i2c_scan_count;
extern volatile uint32_t imu_i2c_read_count;
extern volatile uint32_t imu_i2c_read_error_count;
extern volatile uint8_t imu_i2c_data_valid;
extern volatile uint8_t imu_i2c_data_addr;
extern volatile uint8_t imu_i2c_read_mode;
extern volatile uint8_t imu_i2c_dbg0;
extern volatile uint8_t imu_i2c_dbg1;
extern volatile uint8_t imu_i2c_dbg2;
extern volatile int32_t imu_roll_x100;
extern volatile int32_t imu_pitch_x100;
extern volatile int32_t imu_yaw_x100;
extern volatile int32_t imu_yaw_raw_x100;
extern volatile int32_t imu_yaw_delta_x100;
extern volatile uint8_t imu_raw_valid;
extern volatile uint8_t imu_raw_addr;
extern volatile uint8_t imu_raw_whoami;
extern volatile int16_t imu_acc_x;
extern volatile int16_t imu_acc_y;
extern volatile int16_t imu_acc_z;
extern volatile int16_t imu_gyro_x;
extern volatile int16_t imu_gyro_y;
extern volatile int16_t imu_gyro_z;
extern volatile uint32_t ultrasonic_distance_mm;
extern volatile uint32_t ultrasonic_raw_distance_mm;
extern volatile uint32_t ultrasonic_echo_us;
extern volatile uint32_t ultrasonic_read_count;
extern volatile uint32_t ultrasonic_timeout_count;
extern volatile uint32_t ultrasonic_trig_count;
extern volatile uint32_t ultrasonic_reject_count;
extern volatile uint32_t ultrasonic_reject_us;
extern volatile uint32_t ultrasonic_last_valid_ms;
extern volatile uint8_t ultrasonic_data_valid;
extern volatile uint8_t ultrasonic_echo_level;
void SystemClock_Config(void);
void LED_Init(void);
void Seedling_GPIO_Init(void);
void DWT_Delay_Init(void);
void delay_us(uint32_t us);
void Seedling_Stop(void);
void Seedling_SetDir(uint8_t dir);
void Seedling_OnePulse(uint32_t period_us);
void Seedling_RunMs(uint8_t dir, uint32_t run_ms);
void Mobile_GPIO_Init(void);
void Mobile_Stop(void);
void Mobile_SetDir(uint8_t dir);
void Mobile_OnePulse(uint32_t period_us);
void Mobile_RunMs(uint8_t dir, uint32_t run_ms);
void Actuator_GPIO_Init(void);
void Drill_On(void);
void Drill_Off(void);
void DrillRod_Down(void);
void DrillRod_Up(void);
void DrillRod_Stop(void);
void Conveyor_Forward(void);
void Conveyor_Reverse(void);
void Conveyor_Stop(void);
void Cover_Forward(void);
void Cover_Reverse(void);
void Cover_Stop(void);
void Pump_On(void);
void Pump_Off(void);
void Actuator_AllStop(void);
void DrillRod_RunCycle(void);
void Conveyor_RunCycle(void);
void Cover_RunCycle(void);
void Pump_RunCycle(void);
void Car_RC1_InputCapture_Init(void);
void Car_RowContinue_InputCapture_Init(void);
/* Manual group / auto speed setting input. */
#define MANUAL_MODE_INPUT_GPIO_PORT GPIOE
#define MANUAL_MODE_INPUT_PIN       GPIO_PIN_6

void Car_Speed_InputCapture_Init(void);
void Car_RC2_InputCapture_Init(void);
void Car_WorkSwitch_InputCapture_Init(void);
void Car_PWM_Output_Init(void);
void I2C1_Init(void);
void I2C1_SelectPins(uint8_t pinset);
void IMU_I2C_Scan(void);
uint8_t IMU_I2C_ScanCurrentPins(uint8_t *first_addr);
void SoftI2C_SelectPins(uint8_t pinset);
void SoftI2C_SetSCL(uint8_t level);
void SoftI2C_SetSDA(uint8_t level);
uint8_t SoftI2C_ReadSDA(void);
uint8_t SoftI2C_ReadSCL(void);
void SoftI2C_Delay(void);
void SoftI2C_Start(void);
void SoftI2C_Stop(void);
uint8_t SoftI2C_WaitAck(void);
void SoftI2C_Ack(void);
void SoftI2C_Nack(void);
void SoftI2C_SendByte(uint8_t data);
uint8_t SoftI2C_ReadByte(uint8_t ack);
uint8_t SoftI2C_IsReady(uint8_t addr);
HAL_StatusTypeDef SoftI2C_WriteRead(uint8_t addr, const uint8_t *tx, uint16_t tx_len,
                                           uint8_t *rx, uint16_t rx_len);
HAL_StatusTypeDef SoftI2C_WriteBytes(uint8_t addr, const uint8_t *tx, uint16_t tx_len);
HAL_StatusTypeDef IMU_I2C_ReadRegister(uint8_t addr, uint8_t reg, uint8_t *data, uint16_t len);
HAL_StatusTypeDef IMU_I2C_ReadEulerOnPins(uint8_t pinset);
HAL_StatusTypeDef IMU_I2C_ReadWitAngleOnPins(uint8_t pinset);
void IMU_I2C_ReadAngles(void);
HAL_StatusTypeDef IMU_I2C_ReadRegs(uint8_t addr, uint16_t reg, uint8_t *data, uint16_t len);
int16_t IMU_MakeInt16(uint8_t low, uint8_t high);
int16_t IMU_MakeInt16BE(uint8_t high, uint8_t low);
float IMU_MakeFloatLE(const uint8_t *bytes);
int32_t IMU_DegToX100(float value);
void USART1_UART_Init(void);
void USART1_StartReceive(void);
void USART1_ServiceEcho(void);
void UART1_PushByte(uint8_t data);
void UART1_QueueEchoByte(uint8_t data);
void UART1_QueueString(const char *text);
void USART2_UART_Init(void);
void USART2_StartReceive(void);
void USART2_ServiceEcho(void);
void UART2_PushByte(uint8_t data);
void UART2_QueueEchoByte(uint8_t data);
void UART2_QueueString(const char *text);
void USART3_UART_Init(void);
void USART3_StartReceive(void);
void USART3_ServiceEcho(void);
void UART3_PushByte(uint8_t data);
void UART3_QueueEchoByte(uint8_t data);
void Ultrasonic_GPIO_Init(void);
void Ultrasonic_Service(void);
uint8_t Ultrasonic_ReadOnce(uint32_t *echo_us, uint32_t *distance_mm);
void Ultrasonic_ProcessEcho(uint16_t gpio_pin);
void Ultrasonic_UART2_Report(void);
void App_ServiceTasks(void);
void App_DelayWithService(uint32_t delay_ms);
void App_WaitIfPaused(void);
uint8_t App_WorkflowPaused(void);
void App_SafetyService(void);
void App_RTOS_StartScheduler(void);
void Car_RC_ChannelInit(CarRc_Channel_t *rc);
void Car_Start(void);
void Car_StartWorkSwitchOnly(void);
void Car_StartRowContinueInput(void);
void Car_StartSpeedInput(void);
uint32_t Car_RC_Limit(uint32_t value);
uint8_t Car_RC_OverrideActive(void);
void Car_RC_CalcManualOutput(uint32_t *left_us, uint32_t *right_us);
void Car_PWM_OutputNeutral(void);
void Car_PWM_OutputUpdate(void);
void Car_PWM_OutputUpdateManual(void);
void Car_PWM_OutputSet(uint32_t ch1_us, uint32_t ch2_us);
void Car_RC_ProcessCapture(TIM_HandleTypeDef *htim, uint32_t tim_channel, CarRc_Channel_t *rc);
void Car_AutoMode_Start(void);
void Car_AutoMode_Update(void);
void Car_Motor_GPIO_Init(void);
void Car_Motor_Stop(void);
void Car_Motor_SetLeft(int8_t dir);
void Car_Motor_SetRight(int8_t dir);
void Car_Motor_SetDefaultRunDuty(uint32_t duty_percent);
uint32_t Car_Motor_GetDefaultRunDuty(void);
void Workflow_RunOnce(void);
void Workflow_SetStatus(uint16_t step_index, const char *text);
void LCD_UI_Init(void);
void LCD_ShowValue(uint16_t x, uint16_t y, uint16_t width, const char *text, uint16_t color);
void LCD_UI_Update(void);
void Error_Handler(void);


#endif
