#include "app.h"

#define IMU_YAW_DEADBAND_X100 20L

static volatile uint8_t imu_yaw_base_valid;
static volatile int32_t imu_yaw_base_x100;
static volatile uint8_t imu_yaw_display_valid;
static volatile int32_t imu_yaw_display_x100;

static int32_t IMU_NormalizeYawDeltaX100(int32_t delta_x100)
{
    while (delta_x100 > 18000L)
    {
        delta_x100 -= 36000L;
    }
    while (delta_x100 < -18000L)
    {
        delta_x100 += 36000L;
    }
    return delta_x100;
}

static int32_t IMU_NormalizeYaw0To360X100(int32_t yaw_x100)
{
    while (yaw_x100 < 0L)
    {
        yaw_x100 += 36000L;
    }
    while (yaw_x100 >= 36000L)
    {
        yaw_x100 -= 36000L;
    }
    return yaw_x100;
}

static int32_t IMU_MapYawMeasuredToActualX100(int32_t measured_x100)
{
    static const int32_t measured_points[] = {0L, 4000L, 9400L, 18300L, 36000L};
    static const int32_t actual_points[] = {0L, 9000L, 18000L, 27000L, 36000L};
    uint8_t i;

    measured_x100 = IMU_NormalizeYaw0To360X100(measured_x100);

    for (i = 0U; i < 4U; i++)
    {
        int32_t in0 = measured_points[i];
        int32_t in1 = measured_points[i + 1U];
        int32_t out0 = actual_points[i];
        int32_t out1 = actual_points[i + 1U];

        if (measured_x100 >= in0 && measured_x100 <= in1)
        {
            return out0 + ((measured_x100 - in0) * (out1 - out0)) / (in1 - in0);
        }
    }

    return measured_x100;
}

static int32_t IMU_CalibrateYawX100(int32_t raw_yaw_x100)
{
    int32_t yaw_x100;
    int32_t mapped_yaw_x100;
    int32_t delta_x100;

    imu_yaw_raw_x100 = IMU_NormalizeYaw0To360X100(raw_yaw_x100);

    if (imu_yaw_base_valid == 0U)
    {
        imu_yaw_base_x100 = raw_yaw_x100;
        imu_yaw_base_valid = 1U;
        imu_yaw_display_x100 = 0L;
        imu_yaw_display_valid = 1U;
        imu_yaw_delta_x100 = 0L;
        return 0L;
    }

    if (imu_yaw_display_valid == 0U)
    {
        imu_yaw_display_x100 = 0L;
        imu_yaw_display_valid = 1U;
        return 0L;
    }

    yaw_x100 = IMU_NormalizeYaw0To360X100(imu_yaw_base_x100 - raw_yaw_x100);
    mapped_yaw_x100 = IMU_MapYawMeasuredToActualX100(yaw_x100);
    mapped_yaw_x100 = IMU_NormalizeYaw0To360X100(mapped_yaw_x100);
    delta_x100 = IMU_NormalizeYawDeltaX100(mapped_yaw_x100 - imu_yaw_display_x100);
    imu_yaw_delta_x100 = delta_x100;
    if (delta_x100 > -IMU_YAW_DEADBAND_X100 && delta_x100 < IMU_YAW_DEADBAND_X100)
    {
        return imu_yaw_display_x100;
    }

    imu_yaw_display_x100 = mapped_yaw_x100;
    return imu_yaw_display_x100;
}

void I2C1_Init(void)
{
    SoftI2C_SelectPins(10U);
}

void I2C1_SelectPins(uint8_t pinset)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    uint32_t pins;

    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_I2C1_CLK_ENABLE();

    if (pinset == 10U)
    {
        pins = GPIO_PIN_10 | GPIO_PIN_11;
    }
    else if (pinset == 67U)
    {
        pins = GPIO_PIN_6 | GPIO_PIN_7;
    }
    else
    {
        pinset = 89U;
        pins = GPIO_PIN_8 | GPIO_PIN_9;
    }

    HAL_I2C_DeInit(&hi2c1);
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9 |
                           GPIO_PIN_10 | GPIO_PIN_11);

    GPIO_InitStruct.Pin = pins;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF4_I2C1;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    hi2c1.Instance = I2C1;
    hi2c1.Init.ClockSpeed = 100000U;
    hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
    hi2c1.Init.OwnAddress1 = 0U;
    hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c1.Init.OwnAddress2 = 0U;
    hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
    if (HAL_I2C_Init(&hi2c1) != HAL_OK) Error_Handler();

    imu_i2c_pinset = pinset;
}

void IMU_I2C_Scan(void)
{
    uint8_t found = 0U;
    uint8_t first_addr = 0U;
    uint8_t i;

    for (i = 0U; i < 4U; i++)
    {
        imu_i2c_addr_list[i] = 0U;
    }

    SoftI2C_SelectPins(10U);
    found = IMU_I2C_ScanCurrentPins(&first_addr);
    if (SoftI2C_IsReady(IMU_I2C_ADDR) == 0U)
    {
        SoftI2C_SelectPins(89U);
        found = IMU_I2C_ScanCurrentPins(&first_addr);
    }
    if (SoftI2C_IsReady(IMU_I2C_ADDR) == 0U)
    {
        SoftI2C_SelectPins(67U);
        found = IMU_I2C_ScanCurrentPins(&first_addr);
    }
    if (SoftI2C_IsReady(IMU_I2C_ADDR) == 0U)
    {
        SoftI2C_SelectPins(10U);
        found = IMU_I2C_ScanCurrentPins(&first_addr);
    }

    imu_i2c_found_addr = first_addr;
    imu_i2c_found_count = found;
    imu_i2c_scan_count++;
}

uint8_t IMU_I2C_ScanCurrentPins(uint8_t *first_addr)
{
    uint8_t addr;
    uint8_t found = 0U;

    *first_addr = 0U;
    imu_i2c_scl_level = SoftI2C_ReadSCL();
    imu_i2c_sda_level = SoftI2C_ReadSDA();

    for (addr = 1U; addr < 0x7FU; addr++)
    {
        if (SoftI2C_IsReady(addr) != 0U)
        {
            if (found == 0U)
            {
                *first_addr = addr;
            }
            if (found < 4U)
            {
                imu_i2c_addr_list[found] = addr;
            }
            found++;
        }
    }

    return found;
}

void SoftI2C_SelectPins(uint8_t pinset)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    uint32_t pins;

    __HAL_RCC_GPIOB_CLK_ENABLE();
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9 |
                           GPIO_PIN_10 | GPIO_PIN_11);

    if (pinset == 10U)
    {
        pins = GPIO_PIN_10 | GPIO_PIN_11;
    }
    else if (pinset == 67U)
    {
        pins = GPIO_PIN_6 | GPIO_PIN_7;
    }
    else
    {
        pinset = 89U;
        pins = GPIO_PIN_8 | GPIO_PIN_9;
    }

    GPIO_InitStruct.Pin = pins;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    imu_i2c_pinset = pinset;
    SoftI2C_SetSCL(1U);
    SoftI2C_SetSDA(1U);
    SoftI2C_Stop();
}

void SoftI2C_SetSCL(uint8_t level)
{
    uint16_t pin = GPIO_PIN_8;
    if (imu_i2c_pinset == 67U) pin = GPIO_PIN_6;
    else if (imu_i2c_pinset == 10U) pin = GPIO_PIN_10;
    HAL_GPIO_WritePin(GPIOB, pin, level ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

void SoftI2C_SetSDA(uint8_t level)
{
    uint16_t pin = GPIO_PIN_9;
    if (imu_i2c_pinset == 67U) pin = GPIO_PIN_7;
    else if (imu_i2c_pinset == 10U) pin = GPIO_PIN_11;
    HAL_GPIO_WritePin(GPIOB, pin, level ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

uint8_t SoftI2C_ReadSDA(void)
{
    uint16_t pin = GPIO_PIN_9;
    if (imu_i2c_pinset == 67U) pin = GPIO_PIN_7;
    else if (imu_i2c_pinset == 10U) pin = GPIO_PIN_11;
    return (HAL_GPIO_ReadPin(GPIOB, pin) == GPIO_PIN_SET) ? 1U : 0U;
}

uint8_t SoftI2C_ReadSCL(void)
{
    uint16_t pin = GPIO_PIN_8;
    if (imu_i2c_pinset == 67U) pin = GPIO_PIN_6;
    else if (imu_i2c_pinset == 10U) pin = GPIO_PIN_10;
    return (HAL_GPIO_ReadPin(GPIOB, pin) == GPIO_PIN_SET) ? 1U : 0U;
}

void SoftI2C_Delay(void)
{
    delay_us(5U);
}

void SoftI2C_Start(void)
{
    SoftI2C_SetSDA(1U);
    SoftI2C_SetSCL(1U);
    SoftI2C_Delay();
    SoftI2C_SetSDA(0U);
    SoftI2C_Delay();
    SoftI2C_SetSCL(0U);
    SoftI2C_Delay();
}

void SoftI2C_Stop(void)
{
    SoftI2C_SetSDA(0U);
    SoftI2C_Delay();
    SoftI2C_SetSCL(1U);
    SoftI2C_Delay();
    SoftI2C_SetSDA(1U);
    SoftI2C_Delay();
}

uint8_t SoftI2C_WaitAck(void)
{
    uint16_t wait = 0U;

    SoftI2C_SetSDA(1U);
    SoftI2C_Delay();
    SoftI2C_SetSCL(1U);
    SoftI2C_Delay();
    while (SoftI2C_ReadSDA() != 0U)
    {
        wait++;
        if (wait > 250U)
        {
            SoftI2C_Stop();
            return 1U;
        }
    }
    SoftI2C_SetSCL(0U);
    SoftI2C_Delay();
    return 0U;
}

void SoftI2C_Ack(void)
{
    SoftI2C_SetSDA(0U);
    SoftI2C_Delay();
    SoftI2C_SetSCL(1U);
    SoftI2C_Delay();
    SoftI2C_SetSCL(0U);
    SoftI2C_Delay();
    SoftI2C_SetSDA(1U);
}

void SoftI2C_Nack(void)
{
    SoftI2C_SetSDA(1U);
    SoftI2C_Delay();
    SoftI2C_SetSCL(1U);
    SoftI2C_Delay();
    SoftI2C_SetSCL(0U);
    SoftI2C_Delay();
}

void SoftI2C_SendByte(uint8_t data)
{
    uint8_t i;

    for (i = 0U; i < 8U; i++)
    {
        SoftI2C_SetSDA((data & 0x80U) ? 1U : 0U);
        SoftI2C_Delay();
        SoftI2C_SetSCL(1U);
        SoftI2C_Delay();
        SoftI2C_SetSCL(0U);
        data <<= 1;
    }
    SoftI2C_SetSDA(1U);
}

uint8_t SoftI2C_ReadByte(uint8_t ack)
{
    uint8_t i;
    uint8_t data = 0U;

    SoftI2C_SetSDA(1U);
    for (i = 0U; i < 8U; i++)
    {
        data <<= 1;
        SoftI2C_SetSCL(1U);
        SoftI2C_Delay();
        if (SoftI2C_ReadSDA() != 0U)
        {
            data++;
        }
        SoftI2C_SetSCL(0U);
        SoftI2C_Delay();
    }

    if (ack != 0U) SoftI2C_Ack();
    else SoftI2C_Nack();
    return data;
}

uint8_t SoftI2C_IsReady(uint8_t addr)
{
    uint8_t ack;

    SoftI2C_Start();
    SoftI2C_SendByte((uint8_t)(addr << 1));
    ack = SoftI2C_WaitAck();
    SoftI2C_Stop();
    return (ack == 0U) ? 1U : 0U;
}

HAL_StatusTypeDef SoftI2C_WriteRead(uint8_t addr, const uint8_t *tx, uint16_t tx_len,
                                           uint8_t *rx, uint16_t rx_len)
{
    uint16_t i;

    SoftI2C_Start();
    SoftI2C_SendByte((uint8_t)(addr << 1));
    if (SoftI2C_WaitAck() != 0U)
    {
        SoftI2C_Stop();
        return HAL_ERROR;
    }

    for (i = 0U; i < tx_len; i++)
    {
        SoftI2C_SendByte(tx[i]);
        if (SoftI2C_WaitAck() != 0U)
        {
            SoftI2C_Stop();
            return HAL_ERROR;
        }
    }

    /* The Yahboom IMU sample code uses a repeated-start after the register byte. */
    SoftI2C_Start();
    SoftI2C_SendByte((uint8_t)((addr << 1) | 0x01U));
    if (SoftI2C_WaitAck() != 0U)
    {
        SoftI2C_Stop();
        return HAL_ERROR;
    }

    for (i = 0U; i < rx_len; i++)
    {
        rx[i] = SoftI2C_ReadByte((i + 1U) < rx_len);
    }
    SoftI2C_Stop();
    return HAL_OK;
}

HAL_StatusTypeDef SoftI2C_WriteBytes(uint8_t addr, const uint8_t *tx, uint16_t tx_len)
{
    uint16_t i;

    SoftI2C_Start();
    SoftI2C_SendByte((uint8_t)(addr << 1));
    if (SoftI2C_WaitAck() != 0U)
    {
        SoftI2C_Stop();
        return HAL_ERROR;
    }

    for (i = 0U; i < tx_len; i++)
    {
        SoftI2C_SendByte(tx[i]);
        if (SoftI2C_WaitAck() != 0U)
        {
            SoftI2C_Stop();
            return HAL_ERROR;
        }
    }

    SoftI2C_Stop();
    return HAL_OK;
}

HAL_StatusTypeDef IMU_I2C_ReadRegister(uint8_t addr, uint8_t reg, uint8_t *data, uint16_t len)
{
    return SoftI2C_WriteRead(addr, &reg, 1U, data, len);
}

HAL_StatusTypeDef IMU_I2C_ReadEulerOnPins(uint8_t pinset)
{
    uint8_t version[3] = {0};
    uint8_t euler[12] = {0};
    float roll;
    float pitch;
    float yaw;

    SoftI2C_SelectPins(pinset);
    if (IMU_I2C_ReadRegister(IMU_I2C_ADDR, IMU_REG_VERSION, version, sizeof(version)) != HAL_OK)
    {
        imu_i2c_dbg0 = 0xE1U;
        imu_i2c_dbg1 = 0xE1U;
        imu_i2c_dbg2 = 0xE1U;
        return HAL_ERROR;
    }

    if (IMU_I2C_ReadRegister(IMU_I2C_ADDR, IMU_REG_EULER, euler, sizeof(euler)) != HAL_OK)
    {
        imu_i2c_dbg0 = version[0];
        imu_i2c_dbg1 = version[1];
        imu_i2c_dbg2 = version[2];
        return HAL_ERROR;
    }

    roll = IMU_MakeFloatLE(&euler[0]) * 57.2957795f;
    pitch = IMU_MakeFloatLE(&euler[4]) * 57.2957795f;
    yaw = IMU_MakeFloatLE(&euler[8]) * 57.2957795f;

    imu_i2c_dbg0 = version[0];
    imu_i2c_dbg1 = version[1];
    imu_i2c_dbg2 = version[2];
    imu_roll_x100 = IMU_DegToX100(roll);
    imu_pitch_x100 = IMU_DegToX100(pitch);
    imu_yaw_x100 = IMU_CalibrateYawX100(IMU_DegToX100(yaw));
    imu_i2c_data_addr = IMU_I2C_ADDR;
    imu_i2c_read_mode = 2U;
    imu_i2c_data_valid = 1U;
    imu_i2c_read_count++;
    return HAL_OK;
}

HAL_StatusTypeDef IMU_I2C_ReadWitAngleOnPins(uint8_t pinset)
{
    uint8_t data[6] = {0};
    int16_t raw_roll;
    int16_t raw_pitch;
    int16_t raw_yaw;

    SoftI2C_SelectPins(pinset);
    if (IMU_I2C_ReadRegister(IMU_WIT_I2C_ADDR, IMU_WIT_REG_ANGLE, data, sizeof(data)) != HAL_OK)
    {
        return HAL_ERROR;
    }

    raw_roll = IMU_MakeInt16(data[0], data[1]);
    raw_pitch = IMU_MakeInt16(data[2], data[3]);
    raw_yaw = IMU_MakeInt16(data[4], data[5]);

    imu_i2c_dbg0 = data[0];
    imu_i2c_dbg1 = data[1];
    imu_i2c_dbg2 = data[2];
    imu_roll_x100 = ((int32_t)raw_roll * 18000L) / 32768L;
    imu_pitch_x100 = ((int32_t)raw_pitch * 18000L) / 32768L;
    imu_yaw_x100 = IMU_CalibrateYawX100(((int32_t)raw_yaw * 18000L) / 32768L);
    imu_i2c_data_addr = IMU_WIT_I2C_ADDR;
    imu_i2c_read_mode = 5U;
    imu_i2c_data_valid = 1U;
    imu_i2c_read_count++;
    return HAL_OK;
}

void IMU_I2C_ReadAngles(void)
{
    imu_raw_valid = 0U;

    if (IMU_I2C_ReadEulerOnPins(10U) == HAL_OK)
    {
        return;
    }

    imu_i2c_data_valid = 0U;
    imu_i2c_read_error_count++;
}

HAL_StatusTypeDef IMU_I2C_ReadRegs(uint8_t addr, uint16_t reg, uint8_t *data, uint16_t len)
{
    HAL_StatusTypeDef status;

    status = HAL_I2C_Mem_Read(&hi2c1, (uint16_t)(addr << 1), reg, I2C_MEMADD_SIZE_16BIT,
                              data, len, 20U);
    if (status == HAL_OK)
    {
        return status;
    }

    return HAL_I2C_Mem_Read(&hi2c1, (uint16_t)(addr << 1), (uint16_t)(reg & 0xFFU),
                            I2C_MEMADD_SIZE_8BIT, data, len, 20U);
}

HAL_StatusTypeDef IMU_I2C_ReadAddrFrame(uint8_t addr, uint8_t reg,
                                               int16_t *value1, int16_t *value2, int16_t *value3)
{
    uint8_t tx[5] = {0xFFU, 0xAAU, 0x27U, 0x00U, 0x00U};
    uint8_t rx[11] = {0};
    uint8_t sum = 0U;
    uint8_t i;
    HAL_StatusTypeDef status;

    tx[3] = reg;
    status = SoftI2C_WriteRead(addr, tx, sizeof(tx), rx, sizeof(rx));
    if (status != HAL_OK)
    {
        return status;
    }

    if (rx[0] != 0x55U || rx[1] != 0x5FU)
    {
        return HAL_ERROR;
    }

    for (i = 0U; i < 10U; i++)
    {
        sum = (uint8_t)(sum + rx[i]);
    }
    if (sum != rx[10])
    {
        return HAL_ERROR;
    }

    *value1 = IMU_MakeInt16(rx[2], rx[3]);
    *value2 = IMU_MakeInt16(rx[4], rx[5]);
    *value3 = IMU_MakeInt16(rx[6], rx[7]);
    return HAL_OK;
}

HAL_StatusTypeDef IMU_I2C_ReadShortFrame(uint8_t addr, uint8_t reg,
                                                int16_t *value1, int16_t *value2, int16_t *value3)
{
    uint8_t tx[4];
    uint8_t rx[11] = {0};
    uint8_t *payload;

    tx[0] = addr;
    tx[1] = 0x03U;
    tx[2] = 0x00U;
    tx[3] = reg;

    if (SoftI2C_WriteRead(addr, tx, sizeof(tx), rx, sizeof(rx)) != HAL_OK)
    {
        imu_i2c_dbg0 = 0xEEU;
        imu_i2c_dbg1 = 0xEEU;
        imu_i2c_dbg2 = 0xEEU;
        return HAL_ERROR;
    }

    imu_i2c_dbg0 = rx[0];
    imu_i2c_dbg1 = rx[1];
    imu_i2c_dbg2 = rx[2];

    if (rx[0] == addr && rx[1] == 0x03U && rx[2] == 0x06U)
    {
        payload = &rx[3];
    }
    else if (rx[0] == 0x03U && rx[1] == 0x06U)
    {
        payload = &rx[2];
    }
    else
    {
        return HAL_ERROR;
    }

    *value1 = IMU_MakeInt16BE(payload[0], payload[1]);
    *value2 = IMU_MakeInt16BE(payload[2], payload[3]);
    *value3 = IMU_MakeInt16BE(payload[4], payload[5]);
    return HAL_OK;
}

HAL_StatusTypeDef IMU_I2C_ReadModbusAngles(uint8_t addr, uint8_t include_addr,
                                                  int16_t *roll, int16_t *pitch, int16_t *yaw)
{
    uint8_t tx_with_addr[8] = {IMU_I2C_ADDR, 0x03U, 0x00U, 0x3DU, 0x00U, 0x03U, 0x99U, 0x86U};
    uint8_t tx_no_addr[7] = {0x03U, 0x00U, 0x3DU, 0x00U, 0x03U, 0x99U, 0x86U};
    uint8_t rx[11] = {0};
    uint8_t *payload;
    HAL_StatusTypeDef status;

    tx_with_addr[0] = addr;
    status = HAL_I2C_Master_Transmit(&hi2c1, (uint16_t)(addr << 1),
                                     include_addr ? tx_with_addr : tx_no_addr,
                                     include_addr ? sizeof(tx_with_addr) : sizeof(tx_no_addr),
                                     20U);
    if (status != HAL_OK)
    {
        return status;
    }

    HAL_Delay(2U);

    status = HAL_I2C_Master_Receive(&hi2c1, (uint16_t)(addr << 1), rx, 8U, 20U);
    if (status == HAL_OK && rx[0] == 0x03U && rx[1] == 0x06U)
    {
        payload = &rx[2];
    }
    else
    {
        status = HAL_I2C_Master_Transmit(&hi2c1, (uint16_t)(addr << 1),
                                         include_addr ? tx_with_addr : tx_no_addr,
                                         include_addr ? sizeof(tx_with_addr) : sizeof(tx_no_addr),
                                         20U);
        if (status != HAL_OK)
        {
            return status;
        }
        HAL_Delay(2U);
        status = HAL_I2C_Master_Receive(&hi2c1, (uint16_t)(addr << 1), rx, 11U, 20U);
        if (status != HAL_OK || rx[0] != addr || rx[1] != 0x03U || rx[2] != 0x06U)
        {
            return HAL_ERROR;
        }
        payload = &rx[3];
    }

    *roll = IMU_MakeInt16BE(payload[0], payload[1]);
    *pitch = IMU_MakeInt16BE(payload[2], payload[3]);
    *yaw = IMU_MakeInt16BE(payload[4], payload[5]);
    return HAL_OK;
}

int16_t IMU_MakeInt16(uint8_t low, uint8_t high)
{
    return (int16_t)((uint16_t)low | ((uint16_t)high << 8));
}

int16_t IMU_MakeInt16BE(uint8_t high, uint8_t low)
{
    return (int16_t)(((uint16_t)high << 8) | (uint16_t)low);
}

float IMU_MakeFloatLE(const uint8_t *bytes)
{
    float value;
    memcpy(&value, bytes, sizeof(value));
    return value;
}

int32_t IMU_DegToX100(float value)
{
    if (value >= 0.0f)
    {
        return (int32_t)((value * 100.0f) + 0.5f);
    }
    return (int32_t)((value * 100.0f) - 0.5f);
}

