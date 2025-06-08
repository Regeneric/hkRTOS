#pragma once
#include <defines.h>
#include <hardware/gpio.h>

#ifndef GPIO_HIGH
    #define GPIO_HIGH   true
#endif
#ifndef GPIO_LOW
    #define GPIO_LOW    false
#endif

b8  gpio_wait_for_level(u8 pin, u8 level, u32 usTimeout);
b8  gpio_wait_for_level_count(u8 pin, u8 level, u32 usTimeout, u32* count);