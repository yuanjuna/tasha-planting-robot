#ifndef __DELAY_H
#define __DELAY_H
#include "stm32f4xx_hal.h"
#include <stdint.h>
static inline void delay_ms(uint32_t ms)
{
    HAL_Delay(ms);
}
static inline void delay_us(uint32_t us)
{
    volatile uint32_t count = (SystemCoreClock / 1000000U / 5U) * us;
    if (count == 0) count = us * 4U;
    while (count--) { __NOP(); }
}
#endif
