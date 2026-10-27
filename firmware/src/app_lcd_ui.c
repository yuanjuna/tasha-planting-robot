#include "app.h"

#if defined(CAR_GPIO_MANUAL_TEST_ONLY) && (CAR_GPIO_MANUAL_TEST_ONLY != 0)
#define LCD_AUTO_SPEED_SEG_A 0x01U
#define LCD_AUTO_SPEED_SEG_B 0x02U
#define LCD_AUTO_SPEED_SEG_C 0x04U
#define LCD_AUTO_SPEED_SEG_D 0x08U
#define LCD_AUTO_SPEED_SEG_E 0x10U
#define LCD_AUTO_SPEED_SEG_F 0x20U
#define LCD_AUTO_SPEED_SEG_G 0x40U

static uint8_t lcd_auto_speed_last_gear = 0xFFU;
static uint8_t lcd_auto_speed_last_mode_gear = 0xFFU;
static uint8_t lcd_auto_speed_last_tens_digit = 0xFFU;
static uint16_t lcd_auto_speed_last_pc7 = 0xFFFFU;
static uint8_t lcd_auto_speed_last_pc7_state = 0xFFU;

static void LCD_AutoSpeedFillRect(uint16_t sx, uint16_t sy,
                                  uint16_t ex, uint16_t ey,
                                  uint16_t color)
{
    if (sx > ex || sy > ey || sx >= lcddev.width || sy >= lcddev.height)
    {
        return;
    }
    if (ex >= lcddev.width)
    {
        ex = lcddev.width - 1U;
    }
    if (ey >= lcddev.height)
    {
        ey = lcddev.height - 1U;
    }
    lcd_fill(sx, sy, ex, ey, color);
}

static void LCD_AutoSpeedDrawSegment(uint8_t on,
                                     uint16_t sx, uint16_t sy,
                                     uint16_t ex, uint16_t ey)
{
    if (on != 0U)
    {
        LCD_AutoSpeedFillRect(sx, sy, ex, ey, BLACK);
    }
}

static void LCD_AutoSpeedDrawGear(uint8_t gear)
{
    static const uint8_t gear_segments[10] =
    {
        LCD_AUTO_SPEED_SEG_A | LCD_AUTO_SPEED_SEG_B | LCD_AUTO_SPEED_SEG_C |
        LCD_AUTO_SPEED_SEG_D | LCD_AUTO_SPEED_SEG_E | LCD_AUTO_SPEED_SEG_F,
        LCD_AUTO_SPEED_SEG_B | LCD_AUTO_SPEED_SEG_C,
        LCD_AUTO_SPEED_SEG_A | LCD_AUTO_SPEED_SEG_B | LCD_AUTO_SPEED_SEG_D |
        LCD_AUTO_SPEED_SEG_E | LCD_AUTO_SPEED_SEG_G,
        LCD_AUTO_SPEED_SEG_A | LCD_AUTO_SPEED_SEG_B | LCD_AUTO_SPEED_SEG_C |
        LCD_AUTO_SPEED_SEG_D | LCD_AUTO_SPEED_SEG_G,
        LCD_AUTO_SPEED_SEG_B | LCD_AUTO_SPEED_SEG_C | LCD_AUTO_SPEED_SEG_F |
        LCD_AUTO_SPEED_SEG_G,
        LCD_AUTO_SPEED_SEG_A | LCD_AUTO_SPEED_SEG_C | LCD_AUTO_SPEED_SEG_D |
        LCD_AUTO_SPEED_SEG_F | LCD_AUTO_SPEED_SEG_G,
        LCD_AUTO_SPEED_SEG_A | LCD_AUTO_SPEED_SEG_C | LCD_AUTO_SPEED_SEG_D |
        LCD_AUTO_SPEED_SEG_E | LCD_AUTO_SPEED_SEG_F | LCD_AUTO_SPEED_SEG_G,
        LCD_AUTO_SPEED_SEG_A | LCD_AUTO_SPEED_SEG_B | LCD_AUTO_SPEED_SEG_C,
        LCD_AUTO_SPEED_SEG_A | LCD_AUTO_SPEED_SEG_B | LCD_AUTO_SPEED_SEG_C |
        LCD_AUTO_SPEED_SEG_D | LCD_AUTO_SPEED_SEG_E | LCD_AUTO_SPEED_SEG_F |
        LCD_AUTO_SPEED_SEG_G,
        LCD_AUTO_SPEED_SEG_A | LCD_AUTO_SPEED_SEG_B | LCD_AUTO_SPEED_SEG_C |
        LCD_AUTO_SPEED_SEG_D | LCD_AUTO_SPEED_SEG_F | LCD_AUTO_SPEED_SEG_G
    };
    uint8_t segments;
    uint16_t margin_x = lcddev.width / 6U;
    uint16_t margin_y = lcddev.height / 5U;
    uint16_t digit_w;
    uint16_t digit_h;
    uint16_t thick;
    uint16_t gap;
    uint16_t x0;
    uint16_t y0;
    uint16_t mid_y;

    if (gear > 9U)
    {
        gear = 9U;
    }
    segments = gear_segments[gear];

    if (margin_x < 20U) margin_x = 20U;
    if (margin_y < 30U) margin_y = 30U;

    digit_w = (lcddev.width > (margin_x * 2U)) ? (lcddev.width - (margin_x * 2U)) : lcddev.width;
    digit_h = (lcddev.height > (margin_y * 2U)) ? (lcddev.height - (margin_y * 2U)) : lcddev.height;
    thick = digit_w / 7U;
    if (thick < 12U) thick = 12U;
    if (thick > (digit_h / 8U)) thick = digit_h / 8U;
    gap = thick / 2U;
    x0 = (lcddev.width - digit_w) / 2U;
    y0 = (lcddev.height - digit_h) / 2U;
    mid_y = y0 + (digit_h / 2U) - (thick / 2U);

    LCD_AutoSpeedDrawSegment((segments & LCD_AUTO_SPEED_SEG_A) != 0U,
                             x0 + thick, y0,
                             x0 + digit_w - thick - 1U, y0 + thick - 1U);
    LCD_AutoSpeedDrawSegment((segments & LCD_AUTO_SPEED_SEG_B) != 0U,
                             x0 + digit_w - thick, y0 + thick + gap,
                             x0 + digit_w - 1U, mid_y - gap);
    LCD_AutoSpeedDrawSegment((segments & LCD_AUTO_SPEED_SEG_C) != 0U,
                             x0 + digit_w - thick, mid_y + thick + gap,
                             x0 + digit_w - 1U, y0 + digit_h - thick - gap);
    LCD_AutoSpeedDrawSegment((segments & LCD_AUTO_SPEED_SEG_D) != 0U,
                             x0 + thick, y0 + digit_h - thick,
                             x0 + digit_w - thick - 1U, y0 + digit_h - 1U);
    LCD_AutoSpeedDrawSegment((segments & LCD_AUTO_SPEED_SEG_E) != 0U,
                             x0, mid_y + thick + gap,
                             x0 + thick - 1U, y0 + digit_h - thick - gap);
    LCD_AutoSpeedDrawSegment((segments & LCD_AUTO_SPEED_SEG_F) != 0U,
                             x0, y0 + thick + gap,
                             x0 + thick - 1U, mid_y - gap);
    LCD_AutoSpeedDrawSegment((segments & LCD_AUTO_SPEED_SEG_G) != 0U,
                             x0 + thick, mid_y,
                             x0 + digit_w - thick - 1U, mid_y + thick - 1U);
}

static uint8_t LCD_AutoSpeedReadModeGear(void)
{
    uint32_t now = HAL_GetTick();
    uint32_t pulse_us;

    if ((now - speed_rc.last_ms) >= RC_TIMEOUT_MS)
    {
        return 0U;
    }

    pulse_us = speed_rc.pulse_us;
    if (pulse_us <= 1300U)
    {
        return 1U;
    }
    if (pulse_us <= 1700U)
    {
        return 2U;
    }
    return 3U;
}

static void LCD_AutoSpeedDrawSmallDigit(uint16_t x, uint16_t y, uint8_t digit)
{
    char text[2];
    uint16_t w = 48U;
    uint16_t h = 32U;

    LCD_AutoSpeedFillRect(x, y, x + w - 1U, y + h - 1U, WHITE);

    if (digit > 9U)
    {
        digit = 9U;
    }
    text[0] = (digit == 0U) ? '-' : (char)('0' + digit);
    text[1] = '\0';
    lcd_show_string(x + 12U, y + 4U, 24U, 24U, 24U, text, BLACK);
}

static void LCD_AutoSpeedDrawTensDigit(uint8_t tens_digit)
{
    uint16_t x;
    uint16_t y;
    uint16_t w = 48U;

    x = (lcddev.width > w) ? ((lcddev.width - w) / 2U) : 0U;
    y = lcddev.height / 10U;
    LCD_AutoSpeedDrawSmallDigit(x, y, tens_digit);
}

static void LCD_AutoSpeedDrawModeGear(uint8_t mode_gear)
{
    uint16_t x;
    uint16_t y = 12U;
    uint16_t w = 48U;

    x = (lcddev.width > (w + 12U)) ? (lcddev.width - w - 12U) : 0U;
    LCD_AutoSpeedDrawSmallDigit(x, y, mode_gear);
}

static void LCD_AutoSpeedDrawPc7Status(uint8_t state, uint16_t pulse_us)
{
    char text[28];
    const char *mode_text;
    uint16_t x;
    uint16_t y = 12U;
    uint16_t w = 220U;

    if (state == 1U)
    {
        mode_text = "AUTO";
    }
    else if (state == 2U)
    {
        mode_text = "MANUAL";
    }
    else if (state == 3U)
    {
        mode_text = "MID";
    }
    else
    {
        mode_text = "----";
    }

    if (pulse_us != 0U)
    {
        snprintf(text, sizeof(text), "PC7 %s %uus", mode_text, pulse_us);
    }
    else
    {
        snprintf(text, sizeof(text), "PC7 %s", mode_text);
    }

    if (w > lcddev.width)
    {
        w = lcddev.width;
    }
    x = (lcddev.width > w) ? ((lcddev.width - w) / 2U) : 0U;
    LCD_AutoSpeedFillRect(x, y, x + w - 1U, y + 22U, WHITE);
    lcd_show_string(x, y, w, 16U, 16U, text, BLACK);
}
#endif

static void LCD_ShowLabel(uint16_t x, uint16_t y, const char *text)
{
    lcd_show_string(x, y, 120, 16, 16, (char *)text, LCD_TEXT_COLOR);
}

void LCD_UI_Init(void)
{
    lcd_ui_ready = 0U;
    lcd_clear(LCD_BG_COLOR);

#if defined(CAR_GPIO_MANUAL_TEST_ONLY) && (CAR_GPIO_MANUAL_TEST_ONLY != 0)
    g_back_color = WHITE;
    lcd_auto_speed_last_gear = 0xFFU;
    lcd_auto_speed_last_mode_gear = 0xFFU;
    lcd_auto_speed_last_tens_digit = 0xFFU;
    lcd_auto_speed_last_pc7 = 0xFFFFU;
    lcd_auto_speed_last_pc7_state = 0xFFU;
    lcd_ui_ready = 1U;
    LCD_UI_Update();
    return;
#endif

#if defined(IMU_TEST_ONLY) && (IMU_TEST_ONLY != 0)
    lcd_show_string(20, 16, 440, 24, 24, "IMU LCD TEST", LCD_TITLE_COLOR);

    lcd_draw_rectangle(18, 58, 462, 250, BLUE);
    lcd_show_string(32, 76, 180, 24, 24, "ANGLE", LCD_TITLE_COLOR);
    LCD_ShowLabel(32, 120, "ROLL");
    LCD_ShowLabel(32, 156, "PITCH");
    LCD_ShowLabel(32, 192, "YAW");

    lcd_draw_rectangle(18, 280, 462, 470, BLUE);
    lcd_show_string(32, 298, 180, 24, 24, "I2C", LCD_TITLE_COLOR);
    LCD_ShowLabel(32, 342, "SCAN");
    LCD_ShowLabel(32, 378, "PIN");
    LCD_ShowLabel(32, 414, "READ");

    lcd_draw_rectangle(18, 500, 462, 650, BLUE);
    lcd_show_string(32, 518, 180, 24, 24, "STATUS", LCD_TITLE_COLOR);
    LCD_ShowLabel(32, 562, "MODE");
    LCD_ShowLabel(32, 598, "TIME");
    lcd_ui_ready = 1U;
    return;
#endif

    lcd_show_string(20, 16, 440, 24, 24, "TREE PLANTER MONITOR M1", LCD_TITLE_COLOR);

    lcd_draw_rectangle(18, 50, 462, 145, BLUE);
    lcd_show_string(32, 62, 180, 24, 24, "WORK", LCD_TITLE_COLOR);
    LCD_ShowLabel(32, 96, "ROUND/GROUP");
    LCD_ShowLabel(32, 122, "STATUS");

    lcd_draw_rectangle(18, 154, 462, 300, BLUE);
    lcd_show_string(32, 166, 180, 24, 24, "CAR RC", LCD_TITLE_COLOR);
    LCD_ShowLabel(32, 198, "CH1 RAW");
    LCD_ShowLabel(32, 224, "CH1 R");
    LCD_ShowLabel(32, 250, "CH2 RAW");
    LCD_ShowLabel(32, 276, "CH2 L");

    lcd_draw_rectangle(18, 310, 462, 405, BLUE);
    lcd_show_string(32, 322, 180, 24, 24, "RADAR", LCD_TITLE_COLOR);
    LCD_ShowLabel(32, 356, "L/F/R");
    LCD_ShowLabel(32, 382, "FRONT");

    lcd_draw_rectangle(18, 414, 462, 555, BLUE);
    lcd_show_string(32, 426, 180, 24, 24, "IMU", LCD_TITLE_COLOR);
    LCD_ShowLabel(32, 460, "ROLL");
    LCD_ShowLabel(32, 486, "PITCH");
    LCD_ShowLabel(32, 512, "YAW");

    lcd_draw_rectangle(18, 564, 462, 680, BLUE);
    lcd_show_string(32, 576, 180, 24, 24, "ULTRASONIC", LCD_TITLE_COLOR);
    LCD_ShowLabel(32, 612, "DISTANCE");
    LCD_ShowLabel(32, 638, "STATE");
    lcd_ui_ready = 1U;
}

void LCD_ShowValue(uint16_t x, uint16_t y, uint16_t width, const char *text, uint16_t color)
{
    if (lcd_ui_ready == 0U)
    {
        return;
    }

    lcd_fill(x, y, x + width - 1U, y + 20U, LCD_BG_COLOR);
    lcd_show_string(x, y, width, 16, 16, (char *)text, color);
}

static void LCD_FormatAngle(char *buf, size_t size, int32_t value_x100)
{
    snprintf(buf, size, "%ld.%02ld deg",
             (long)(value_x100 / 100L), (long)(labs(value_x100) % 100L));
}

void LCD_UI_Update(void)
{
#if defined(CAR_GPIO_MANUAL_TEST_ONLY) && (CAR_GPIO_MANUAL_TEST_ONLY != 0)
    uint32_t duty_percent;
    uint8_t ones_digit;
    uint8_t tens_digit;
    uint8_t mode_gear;
    uint32_t manual_lcd_now;
    uint16_t pc7_pulse = 0U;
    uint8_t pc7_state = 0U;

    if (lcd_ui_ready == 0U)
    {
        return;
    }

    manual_lcd_now = HAL_GetTick();
    duty_percent = Car_Motor_GetDefaultRunDuty();
    mode_gear = LCD_AutoSpeedReadModeGear();
    if (duty_percent > 99U)
    {
        duty_percent = 99U;
    }
    tens_digit = (uint8_t)(duty_percent / 10U);
    ones_digit = (uint8_t)(duty_percent % 10U);

    if ((manual_lcd_now - work_switch_rc.last_ms) < RC_TIMEOUT_MS)
    {
        pc7_pulse = (uint16_t)work_switch_rc.pulse_us;
        if (pc7_pulse >= 1700U)
        {
            pc7_state = 1U;
        }
        else if (pc7_pulse <= 1300U)
        {
            pc7_state = 2U;
        }
        else
        {
            pc7_state = 3U;
        }
    }

    if (ones_digit != lcd_auto_speed_last_gear)
    {
        lcd_clear(WHITE);
        LCD_AutoSpeedDrawGear(ones_digit);
        LCD_AutoSpeedDrawTensDigit(tens_digit);
        LCD_AutoSpeedDrawModeGear(mode_gear);
        LCD_AutoSpeedDrawPc7Status(pc7_state, pc7_pulse);
        lcd_auto_speed_last_gear = ones_digit;
        lcd_auto_speed_last_tens_digit = tens_digit;
        lcd_auto_speed_last_mode_gear = mode_gear;
        lcd_auto_speed_last_pc7 = pc7_pulse;
        lcd_auto_speed_last_pc7_state = pc7_state;
    }
    else if (tens_digit != lcd_auto_speed_last_tens_digit)
    {
        LCD_AutoSpeedDrawTensDigit(tens_digit);
        lcd_auto_speed_last_tens_digit = tens_digit;
    }
    if (mode_gear != lcd_auto_speed_last_mode_gear)
    {
        LCD_AutoSpeedDrawModeGear(mode_gear);
        lcd_auto_speed_last_mode_gear = mode_gear;
    }
    if ((pc7_pulse != lcd_auto_speed_last_pc7) ||
        (pc7_state != lcd_auto_speed_last_pc7_state))
    {
        LCD_AutoSpeedDrawPc7Status(pc7_state, pc7_pulse);
        lcd_auto_speed_last_pc7 = pc7_pulse;
        lcd_auto_speed_last_pc7_state = pc7_state;
    }
    return;
#endif

    static uint32_t lcd_refresh_count;
    char buf[96];
    uint32_t now = HAL_GetTick();
    uint32_t ch1_last_ms_copy;
    uint32_t ch2_last_ms_copy;
    uint32_t pc7_last_ms_copy;
    uint32_t p_last_ms_copy;
    uint32_t pa8_last_ms_copy;
    uint32_t ch1_in;
    uint32_t ch2_in;
    uint32_t pc7_in;
    uint32_t p_in;
    uint32_t pa8_in;
    uint32_t ch1_out;
    uint32_t ch2_out;
    uint32_t radar_front_cm_copy;
    uint32_t radar_left_cm_copy;
    uint32_t radar_right_cm_copy;
    uint32_t radar_last_ms_copy;
    uint32_t radar_rx_count_copy;
    uint32_t ultrasonic_distance_mm_copy;
    uint32_t ultrasonic_last_valid_ms_copy;
    int32_t imu_roll_copy;
    int32_t imu_pitch_copy;
    int32_t imu_yaw_copy;
    uint32_t imu_scan_count_copy;
    uint32_t imu_read_count_copy;
    uint32_t imu_read_error_count_copy;
    uint8_t imu_found_addr_copy;
    uint8_t imu_found_count_copy;
    uint8_t imu_pinset_copy;
    uint8_t imu_scl_level_copy;
    uint8_t imu_sda_level_copy;
    uint8_t round_copy;
    uint8_t group_copy;
    uint8_t paused_copy;
    uint8_t radar_front_valid_copy;
    uint8_t radar_left_valid_copy;
    uint8_t radar_right_valid_copy;
    uint8_t imu_valid_copy;
    uint8_t ultrasonic_valid_copy;

    if (lcd_ui_ready == 0U)
    {
        return;
    }
    lcd_refresh_count++;

#if defined(IMU_TEST_ONLY) && (IMU_TEST_ONLY != 0)
    {
        int32_t imu_roll_copy;
        int32_t imu_pitch_copy;
        int32_t imu_yaw_copy;
        uint32_t imu_scan_count_copy;
        uint32_t imu_read_count_copy;
        uint32_t imu_read_error_count_copy;
        uint8_t imu_found_addr_copy;
        uint8_t imu_found_count_copy;
        uint8_t imu_pinset_copy;
        uint8_t imu_scl_level_copy;
        uint8_t imu_sda_level_copy;
        uint8_t imu_valid_copy;
        uint8_t imu_data_addr_copy;
        uint8_t imu_read_mode_copy;

        __disable_irq();
        imu_roll_copy = imu_roll_x100;
        imu_pitch_copy = imu_pitch_x100;
        imu_yaw_copy = imu_yaw_x100;
        imu_valid_copy = imu_i2c_data_valid;
        imu_scan_count_copy = imu_i2c_scan_count;
        imu_read_count_copy = imu_i2c_read_count;
        imu_read_error_count_copy = imu_i2c_read_error_count;
        imu_found_addr_copy = imu_i2c_found_addr;
        imu_found_count_copy = imu_i2c_found_count;
        imu_pinset_copy = imu_i2c_pinset;
        imu_scl_level_copy = imu_i2c_scl_level;
        imu_sda_level_copy = imu_i2c_sda_level;
        imu_data_addr_copy = imu_i2c_data_addr;
        imu_read_mode_copy = imu_i2c_read_mode;
        __enable_irq();

        if (imu_valid_copy != 0U)
        {
            LCD_FormatAngle(buf, sizeof(buf), imu_roll_copy);
            LCD_ShowValue(150, 120, 220, buf, LCD_OK_COLOR);
            LCD_FormatAngle(buf, sizeof(buf), imu_pitch_copy);
            LCD_ShowValue(150, 156, 220, buf, LCD_OK_COLOR);
            LCD_FormatAngle(buf, sizeof(buf), imu_yaw_copy);
            LCD_ShowValue(150, 192, 220, buf, LCD_OK_COLOR);
        }
        else
        {
            LCD_ShowValue(150, 120, 220, "NO DATA", LCD_WARN_COLOR);
            LCD_ShowValue(150, 156, 220, "NO DATA", LCD_WARN_COLOR);
            LCD_ShowValue(150, 192, 220, "NO DATA", LCD_WARN_COLOR);
        }

        snprintf(buf, sizeof(buf), "cnt%lu n%u addr%02X",
                 (unsigned long)imu_scan_count_copy,
                 (unsigned int)imu_found_count_copy,
                 (unsigned int)imu_found_addr_copy);
        LCD_ShowValue(150, 342, 260, buf, (imu_found_count_copy != 0U) ? LCD_OK_COLOR : LCD_WARN_COLOR);

        snprintf(buf, sizeof(buf), "pin%u SCL%u SDA%u",
                 (unsigned int)imu_pinset_copy,
                 (unsigned int)imu_scl_level_copy,
                 (unsigned int)imu_sda_level_copy);
        LCD_ShowValue(150, 378, 260, buf, LCD_TEXT_COLOR);

        snprintf(buf, sizeof(buf), "ok%lu err%lu",
                 (unsigned long)imu_read_count_copy,
                 (unsigned long)imu_read_error_count_copy);
        LCD_ShowValue(150, 414, 260, buf, (imu_valid_copy != 0U) ? LCD_OK_COLOR : LCD_WARN_COLOR);

        snprintf(buf, sizeof(buf), "%s A%02X M%u",
                 (imu_valid_copy != 0U) ? "OK" : "WAIT",
                 (unsigned int)imu_data_addr_copy,
                 (unsigned int)imu_read_mode_copy);
        LCD_ShowValue(150, 562, 260, buf, (imu_valid_copy != 0U) ? LCD_OK_COLOR : LCD_WARN_COLOR);

        snprintf(buf, sizeof(buf), "T%lus U%lu",
                 (unsigned long)(now / 1000U),
                 (unsigned long)lcd_refresh_count);
        LCD_ShowValue(150, 598, 260, buf, LCD_TITLE_COLOR);
        return;
    }
#endif

    __disable_irq();
    ch1_in = car_rc_ch1.pulse_us;
    ch2_in = car_rc_ch2.pulse_us;
    pc7_in = work_switch_rc.pulse_us;
    p_in = row_continue_rc.pulse_us;
    pa8_in = speed_rc.pulse_us;
    ch1_last_ms_copy = car_rc_ch1.last_ms;
    ch2_last_ms_copy = car_rc_ch2.last_ms;
    pc7_last_ms_copy = work_switch_rc.last_ms;
    p_last_ms_copy = row_continue_rc.last_ms;
    pa8_last_ms_copy = speed_rc.last_ms;
    round_copy = workflow_round_index;
    group_copy = workflow_group_index;
    paused_copy = workflow_paused;
    radar_front_cm_copy = radar_front_distance_cm;
    radar_left_cm_copy = radar_left_distance_cm;
    radar_right_cm_copy = radar_right_distance_cm;
    radar_last_ms_copy = radar_last_ms;
    radar_front_valid_copy = radar_front_valid;
    radar_left_valid_copy = radar_left_valid;
    radar_right_valid_copy = radar_right_valid;
    radar_rx_count_copy = uart1_rx_count;
    imu_roll_copy = imu_roll_x100;
    imu_pitch_copy = imu_pitch_x100;
    imu_yaw_copy = imu_yaw_x100;
    imu_valid_copy = imu_i2c_data_valid;
    imu_scan_count_copy = imu_i2c_scan_count;
    imu_read_count_copy = imu_i2c_read_count;
    imu_read_error_count_copy = imu_i2c_read_error_count;
    imu_found_addr_copy = imu_i2c_found_addr;
    imu_found_count_copy = imu_i2c_found_count;
    imu_pinset_copy = imu_i2c_pinset;
    imu_scl_level_copy = imu_i2c_scl_level;
    imu_sda_level_copy = imu_i2c_sda_level;
    ultrasonic_distance_mm_copy = ultrasonic_distance_mm;
    ultrasonic_last_valid_ms_copy = ultrasonic_last_valid_ms;
    ultrasonic_valid_copy = ultrasonic_data_valid;
    __enable_irq();

    if (paused_copy != 0U)
    {
        snprintf(buf, sizeof(buf), "PAUSED T%lus U%lu",
                 (unsigned long)(now / 1000U),
                 (unsigned long)lcd_refresh_count);
    }
    else if (round_copy == 0U || group_copy == 0U)
    {
        snprintf(buf, sizeof(buf), "Waiting T%lus U%lu",
                 (unsigned long)(now / 1000U),
                 (unsigned long)lcd_refresh_count);
    }
    else
    {
        snprintf(buf, sizeof(buf), "R%u G%u T%lus U%lu",
                 (unsigned int)round_copy,
                 (unsigned int)group_copy,
                 (unsigned long)(now / 1000U),
                 (unsigned long)lcd_refresh_count);
    }
    LCD_ShowValue(150, 96, 300, buf, LCD_OK_COLOR);

    {
        char swa_text[20];
        char swb_text[20];
        char pa8_text[20];
        uint32_t out = RC_NEUTRAL_US;

        if ((now - pc7_last_ms_copy) < RC_TIMEOUT_MS)
        {
            snprintf(swa_text, sizeof(swa_text), "%s%lu",
                     (pc7_in >= WORK_SWITCH_THRESHOLD_US) ? "HI" : "LO",
                     (unsigned long)pc7_in);
        }
        else
        {
            snprintf(swa_text, sizeof(swa_text), "--");
        }

        if ((now - p_last_ms_copy) < RC_TIMEOUT_MS)
        {
            snprintf(swb_text, sizeof(swb_text), "%s%lu",
                     (p_in >= WORK_SWITCH_THRESHOLD_US) ? "HI" : "LO",
                     (unsigned long)p_in);
        }
        else
        {
            snprintf(swb_text, sizeof(swb_text), "--");
        }

        if ((now - pa8_last_ms_copy) < RC_TIMEOUT_MS)
        {
            snprintf(pa8_text, sizeof(pa8_text), "%lu", (unsigned long)pa8_in);
            if (pa8_in < 1500U) pa8_in = 1500U;
            if (pa8_in > 2000U) pa8_in = 2000U;
            out = 1500U + ((pa8_in - 1500U) * (RC_HIGH_LIMIT_US - 1500U)) / 500U;
        }
        else
        {
            snprintf(pa8_text, sizeof(pa8_text), "--");
            out = 1500U;
        }

        snprintf(buf, sizeof(buf), "SWA%s SWB%s THR%s O%lu",
                 swa_text,
                 swb_text,
                 pa8_text,
                 (unsigned long)out);
    }
    LCD_ShowValue(150, 122, 300, buf, LCD_TITLE_COLOR);

    if ((now - ch1_last_ms_copy) < RC_TIMEOUT_MS)
    {
        snprintf(buf, sizeof(buf), "%4lu us", (unsigned long)ch1_in);
        LCD_ShowValue(150, 198, 120, buf, LCD_VALUE_COLOR);
    }
    else
    {
        LCD_ShowValue(150, 198, 120, "---- us", LCD_WARN_COLOR);
    }
    Car_RC_CalcManualOutput(&ch1_out, &ch2_out);
    snprintf(buf, sizeof(buf), "%4lu us", (unsigned long)ch1_out);
    LCD_ShowValue(150, 224, 120, buf, LCD_VALUE_COLOR);

    if ((now - ch2_last_ms_copy) < RC_TIMEOUT_MS)
    {
        snprintf(buf, sizeof(buf), "%4lu us", (unsigned long)ch2_in);
        LCD_ShowValue(150, 250, 120, buf, LCD_VALUE_COLOR);
    }
    else
    {
        LCD_ShowValue(150, 250, 120, "---- us", LCD_WARN_COLOR);
    }
    snprintf(buf, sizeof(buf), "%4lu us", (unsigned long)ch2_out);
    LCD_ShowValue(150, 276, 120, buf, LCD_VALUE_COLOR);

    if ((radar_left_valid_copy != 0U) ||
        (radar_front_valid_copy != 0U) ||
        (radar_right_valid_copy != 0U))
    {
        snprintf(buf, sizeof(buf), "L:%lu F:%lu R:%lu RX%lu",
                 (unsigned long)radar_left_cm_copy,
                 (unsigned long)radar_front_cm_copy,
                 (unsigned long)radar_right_cm_copy,
                 (unsigned long)radar_rx_count_copy);
        LCD_ShowValue(150, 356, 300, buf, LCD_OK_COLOR);
    }
    else
    {
        snprintf(buf, sizeof(buf), "Waiting  RX%lu", (unsigned long)radar_rx_count_copy);
        LCD_ShowValue(150, 356, 300, buf, LCD_WARN_COLOR);
    }

    if (((radar_left_valid_copy != 0U) ||
         (radar_front_valid_copy != 0U) ||
         (radar_right_valid_copy != 0U)) &&
        (now - radar_last_ms_copy) <= 1000U)
    {
        snprintf(buf, sizeof(buf), "OK  RX%lu", (unsigned long)radar_rx_count_copy);
        LCD_ShowValue(150, 382, 180, buf, LCD_OK_COLOR);
    }
    else
    {
        snprintf(buf, sizeof(buf), "OLD RX%lu", (unsigned long)radar_rx_count_copy);
        LCD_ShowValue(150, 382, 180, buf, LCD_WARN_COLOR);
    }

    if (imu_valid_copy != 0U)
    {
        LCD_FormatAngle(buf, sizeof(buf), imu_roll_copy);
        LCD_ShowValue(150, 460, 140, buf, LCD_OK_COLOR);
        LCD_FormatAngle(buf, sizeof(buf), imu_pitch_copy);
        LCD_ShowValue(150, 486, 140, buf, LCD_OK_COLOR);
        LCD_FormatAngle(buf, sizeof(buf), imu_yaw_copy);
        LCD_ShowValue(150, 512, 140, buf, LCD_OK_COLOR);
    }
    else
    {
        snprintf(buf, sizeof(buf), "scan%lu n%u a%02X",
                 (unsigned long)imu_scan_count_copy,
                 (unsigned int)imu_found_count_copy,
                 (unsigned int)imu_found_addr_copy);
        LCD_ShowValue(150, 460, 180, buf, LCD_WARN_COLOR);
        snprintf(buf, sizeof(buf), "pin%u SCL%u SDA%u",
                 (unsigned int)imu_pinset_copy,
                 (unsigned int)imu_scl_level_copy,
                 (unsigned int)imu_sda_level_copy);
        LCD_ShowValue(150, 486, 180, buf, LCD_WARN_COLOR);
        snprintf(buf, sizeof(buf), "rd%lu err%lu",
                 (unsigned long)imu_read_count_copy,
                 (unsigned long)imu_read_error_count_copy);
        LCD_ShowValue(150, 512, 180, buf, LCD_WARN_COLOR);
    }

    if (ultrasonic_valid_copy != 0U || ultrasonic_distance_mm_copy != 0U)
    {
        snprintf(buf, sizeof(buf), "%lu.%lu cm",
                 (unsigned long)(ultrasonic_distance_mm_copy / 10U),
                 (unsigned long)(ultrasonic_distance_mm_copy % 10U));
        LCD_ShowValue(150, 612, 140, buf, LCD_OK_COLOR);
        snprintf(buf, sizeof(buf), "Age%lu R%lu",
                 (unsigned long)(now - ultrasonic_last_valid_ms_copy),
                 (unsigned long)ultrasonic_read_count);
        LCD_ShowValue(150, 638, 140, buf, LCD_TEXT_COLOR);
    }
    else
    {
        LCD_ShowValue(150, 612, 140, "--.- cm", LCD_WARN_COLOR);
        LCD_ShowValue(150, 638, 140, "Waiting", LCD_WARN_COLOR);
    }
}
