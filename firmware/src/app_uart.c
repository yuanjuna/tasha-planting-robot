#include "app.h"

void USART1_UART_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* USART1: radar data receiver, PA9 TX / PA10 RX. */
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_USART1_CLK_ENABLE();

    GPIO_InitStruct.Pin = GPIO_PIN_9 | GPIO_PIN_10;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    huart1.Instance = USART1;
    huart1.Init.BaudRate = UART_BAUDRATE;
    huart1.Init.WordLength = UART_WORDLENGTH_8B;
    huart1.Init.StopBits = UART_STOPBITS_1;
    huart1.Init.Parity = UART_PARITY_NONE;
    huart1.Init.Mode = UART_MODE_TX_RX;
    huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart1.Init.OverSampling = UART_OVERSAMPLING_16;
    if (HAL_UART_Init(&huart1) != HAL_OK) Error_Handler();
    HAL_NVIC_SetPriority(USART1_IRQn, 2U, 0U);
    HAL_NVIC_EnableIRQ(USART1_IRQn);
}

void USART1_StartReceive(void)
{
    if (HAL_UART_Receive_IT(&huart1, &uart1_rx_byte, 1U) != HAL_OK)
    {
        uart1_error_count++;
    }
}

void USART1_ServiceEcho(void)
{
    uint8_t data;
    uint8_t sent = 0U;

    while (sent < 16U)
    {
        __disable_irq();
        if (uart1_tx_tail == uart1_tx_head)
        {
            __enable_irq();
            return;
        }
        data = uart1_tx_buf[uart1_tx_tail];
        uart1_tx_tail++;
        if (uart1_tx_tail >= UART1_TX_BUF_SIZE)
        {
            uart1_tx_tail = 0U;
        }
        __enable_irq();

        if (HAL_UART_Transmit(&huart1, &data, 1U, 20U) == HAL_OK)
        {
            uart1_tx_count++;
        }
        else
        {
            uart1_error_count++;
        }
        sent++;
    }
}

void USART2_UART_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* USART2: host/debug serial port, PA2 TX / PA3 RX. */
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_USART2_CLK_ENABLE();

    GPIO_InitStruct.Pin = GPIO_PIN_2 | GPIO_PIN_3;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    huart2.Instance = USART2;
    huart2.Init.BaudRate = UART2_BAUDRATE;
    huart2.Init.WordLength = UART_WORDLENGTH_8B;
    huart2.Init.StopBits = UART_STOPBITS_1;
    huart2.Init.Parity = UART_PARITY_NONE;
    huart2.Init.Mode = UART_MODE_TX_RX;
    huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart2.Init.OverSampling = UART_OVERSAMPLING_16;
    if (HAL_UART_Init(&huart2) != HAL_OK) Error_Handler();
    HAL_NVIC_SetPriority(USART2_IRQn, 2U, 0U);
    HAL_NVIC_EnableIRQ(USART2_IRQn);
}

void USART2_StartReceive(void)
{
    if (HAL_UART_Receive_IT(&huart2, &uart2_rx_byte, 1U) != HAL_OK)
    {
        uart2_error_count++;
    }
}

void USART2_ServiceEcho(void)
{
    uint8_t data;
    uint8_t sent = 0U;

    while (sent < 16U)
    {
        __disable_irq();
        if (uart2_tx_tail == uart2_tx_head)
        {
            __enable_irq();
            return;
        }
        data = uart2_tx_buf[uart2_tx_tail];
        uart2_tx_tail++;
        if (uart2_tx_tail >= UART2_TX_BUF_SIZE)
        {
            uart2_tx_tail = 0U;
        }
        __enable_irq();

        if (HAL_UART_Transmit(&huart2, &data, 1U, 20U) == HAL_OK)
        {
            uart2_tx_count++;
        }
        else
        {
            uart2_error_count++;
        }
        sent++;
    }
}

void USART3_UART_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* USART3: IMU UART data receiver, PB10 TX / PB11 RX. */
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_USART3_CLK_ENABLE();

    GPIO_InitStruct.Pin = GPIO_PIN_10 | GPIO_PIN_11;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART3;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    huart3.Instance = USART3;
    huart3.Init.BaudRate = UART_BAUDRATE;
    huart3.Init.WordLength = UART_WORDLENGTH_8B;
    huart3.Init.StopBits = UART_STOPBITS_1;
    huart3.Init.Parity = UART_PARITY_NONE;
    huart3.Init.Mode = UART_MODE_TX_RX;
    huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart3.Init.OverSampling = UART_OVERSAMPLING_16;
    if (HAL_UART_Init(&huart3) != HAL_OK) Error_Handler();
    HAL_NVIC_SetPriority(USART3_IRQn, 2U, 0U);
    HAL_NVIC_EnableIRQ(USART3_IRQn);
}

void USART3_StartReceive(void)
{
    if (HAL_UART_Receive_IT(&huart3, &uart3_rx_byte, 1U) != HAL_OK)
    {
        uart3_error_count++;
    }
}

void USART3_ServiceEcho(void)
{
    uint8_t data;

    __disable_irq();
    if (uart3_tx_tail == uart3_tx_head)
    {
        __enable_irq();
        return;
    }
    data = uart3_tx_buf[uart3_tx_tail];
    uart3_tx_tail++;
    if (uart3_tx_tail >= UART3_TX_BUF_SIZE)
    {
        uart3_tx_tail = 0U;
    }
    __enable_irq();

    if (HAL_UART_Transmit(&huart3, &data, 1U, 20U) == HAL_OK)
    {
        uart3_tx_count++;
    }
    else
    {
        uart3_error_count++;
    }
}

void UART1_PushByte(uint8_t data)
{
    static uint8_t parse_state = 0U;
    static uint8_t parse_label = 0U;
    static uint32_t parse_value = 0U;
    static uint8_t parse_digits = 0U;

    uart1_rx_count++;
    uart1_last_byte = data;
    uart1_rx_event = 1U;

    if (parse_state == 0U)
    {
        if (data == (uint8_t)'L' || data == (uint8_t)'F' || data == (uint8_t)'R')
        {
            parse_label = data;
            parse_state = 1U;
        }
    }
    else if (parse_state == 1U)
    {
        parse_state = (data == (uint8_t)':') ? 2U : 0U;
        parse_value = 0U;
        parse_digits = 0U;
    }
    else
    {
        if (data >= (uint8_t)'0' && data <= (uint8_t)'9')
        {
            if (parse_value < 100000U)
            {
                parse_value = (parse_value * 10U) + (uint32_t)(data - (uint8_t)'0');
            }
            parse_digits++;
        }
        else
        {
            if (parse_digits != 0U)
            {
                if (parse_label == (uint8_t)'L')
                {
                    radar_left_distance_cm = parse_value;
                    radar_left_valid = 1U;
                }
                else if (parse_label == (uint8_t)'F')
                {
                    radar_front_distance_cm = parse_value;
                    radar_front_valid = 1U;
                }
                else if (parse_label == (uint8_t)'R')
                {
                    radar_right_distance_cm = parse_value;
                    radar_right_valid = 1U;
                }
                radar_last_ms = HAL_GetTick();
            }
            if (data == (uint8_t)'L' || data == (uint8_t)'F' || data == (uint8_t)'R')
            {
                parse_label = data;
                parse_state = 1U;
            }
            else
            {
                parse_state = 0U;
            }
            parse_value = 0U;
            parse_digits = 0U;
        }
    }

    if (data == '\r' || data == '\n')
    {
        return;
    }

    if (data < 32U || data > 126U)
    {
        data = '?';
    }

    if (uart1_rx_index >= (UART1_RX_BUF_SIZE - 1U))
    {
        uart1_rx_index = 0U;
    }

    uart1_lcd_buf[uart1_rx_index++] = (char)data;
    uart1_lcd_buf[uart1_rx_index] = '\0';
}

void UART1_QueueEchoByte(uint8_t data)
{
    uint16_t next_head = uart1_tx_head + 1U;

    if (next_head >= UART1_TX_BUF_SIZE)
    {
        next_head = 0U;
    }

    if (next_head == uart1_tx_tail)
    {
        uart1_tx_drop_count++;
        return;
    }

    uart1_tx_buf[uart1_tx_head] = data;
    uart1_tx_head = next_head;
}

void UART1_QueueString(const char *text)
{
    while (*text != '\0')
    {
        UART1_QueueEchoByte((uint8_t)*text);
        text++;
    }
}

void UART2_PushByte(uint8_t data)
{
    uart2_rx_count++;
    uart2_last_byte = data;
    uart2_rx_event = 1U;

    if (data == '\r' || data == '\n')
    {
        return;
    }

    if (data < 32U || data > 126U)
    {
        data = '.';
    }

    if (uart2_rx_index >= (UART2_RX_BUF_SIZE - 1U))
    {
        uart2_rx_index = 0U;
    }

    uart2_lcd_buf[uart2_rx_index++] = (char)data;
    uart2_lcd_buf[uart2_rx_index] = '\0';
}

void UART2_QueueEchoByte(uint8_t data)
{
    uint16_t next_head = uart2_tx_head + 1U;
    uint8_t lcd_data = data;

    if (next_head >= UART2_TX_BUF_SIZE)
    {
        next_head = 0U;
    }

    if (next_head == uart2_tx_tail)
    {
        uart2_tx_drop_count++;
        return;
    }

    uart2_tx_buf[uart2_tx_head] = data;
    uart2_tx_head = next_head;

    uart2_tx_event = 1U;
    if (lcd_data == '\r' || lcd_data == '\n')
    {
        return;
    }
    if (lcd_data == (uint8_t)'L')
    {
        uart2_tx_lcd_index = 0U;
    }
    if (lcd_data < 32U || lcd_data > 126U)
    {
        lcd_data = '.';
    }
    if (uart2_tx_lcd_index >= (UART2_RX_BUF_SIZE - 1U))
    {
        uart2_tx_lcd_index = 0U;
    }
    uart2_tx_lcd_buf[uart2_tx_lcd_index++] = (char)lcd_data;
    uart2_tx_lcd_buf[uart2_tx_lcd_index] = '\0';
}

void UART2_QueueString(const char *text)
{
    while (text != NULL && *text != '\0')
    {
        UART2_QueueEchoByte((uint8_t)*text);
        text++;
    }
}

void UART3_PushByte(uint8_t data)
{
    uart3_rx_count++;
    uart3_last_byte = data;
    uart3_rx_event = 1U;

    if (data == '\r' || data == '\n')
    {
        return;
    }

    if (data < 32U || data > 126U)
    {
        data = '?';
    }

    if (uart3_rx_index >= (UART3_RX_BUF_SIZE - 1U))
    {
        uart3_rx_index = 0U;
    }

    uart3_lcd_buf[uart3_rx_index++] = (char)data;
    uart3_lcd_buf[uart3_rx_index] = '\0';
}

void UART3_QueueEchoByte(uint8_t data)
{
    uint16_t next_head = uart3_tx_head + 1U;

    if (next_head >= UART3_TX_BUF_SIZE)
    {
        next_head = 0U;
    }

    if (next_head == uart3_tx_tail)
    {
        uart3_tx_drop_count++;
        return;
    }

    uart3_tx_buf[uart3_tx_head] = data;
    uart3_tx_head = next_head;
}

