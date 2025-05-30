#pragma once
#include <defines.h>

#define GPIO_HIGH   TRUE
#define GPIO_LOW    FALSE

b8  gpio_wait_for_level(u8 pin, u8 level, u32 usTimeout);
b8  gpio_wait_for_level_count(u8 pin, u8 level, u32 usTimeout, u32* count);