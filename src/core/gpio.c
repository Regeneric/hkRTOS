#include <core/gpio.h>

#include <pico/stdlib.h>
#include <hardware/gpio.h>

b8 gpio_wait_for_level(u8 pin, u8 level, u32 usTimeout) {
    u32 startTime = time_us_32();
    while(gpio_get(pin) == level) {
        if(time_us_32()-startTime > usTimeout) return false;
    }
}

b8 gpio_wait_for_level_count(u8 pin, u8 level, u32 usTimeout, u32* count) {
    u32 startTime = time_us_32();
    while(gpio_get(pin) == level) {
        if(time_us_32()-startTime > usTimeout) return false;
    } u32 endTime = time_us_32()-startTime;
    
    *count = endTime;
    return true;
}