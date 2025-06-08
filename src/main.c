// Newlib
#include <stdio.h>

// Pico SDK
#include <pico/stdlib.h>
#include <pico/stdio.h>
#include <pico/multicore.h>
#include <pico/cyw43_arch.h>
#include <hardware/pio.h>

// FreeRTOS
#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>

// Segger
#include <SEGGER_SYSVIEW.h>

// hkRTOS
#include <defines.h>

#include <core/logger.h>

#include <comm/i2c.h>
#include <comm/network/wifi.h>
#include <comm/onewire/onewire.h>

#include <storage/storage.h>
#include <storage/eeprom.h>

#include <sensors/sensors.h>
#include <sensors/dht11/dht11.h>
#include <sensors/ds18b20/ds18b20.h>

#include <display/display.h>
#include <display/gfx/gfx.h>
#include <display/ssd1327/ssd1327_config.h>

static DHT_Config_t hkDHT11;

static bool DHT11_Timer_ISR(struct repeating_timer *t) {
    DHT11_Read(&hkDHT11);
    return true;
}

// struct repeating_timer timer;
// add_repeating_timer_ms(-2000, DHT11_Timer_ISR, NULL, &timer);



void main(void) {
    stdio_init_all();

    I2C_Config_t i2c = {
        .i2c   = hkI2C,
        .scl   = hkI2C_SCL,
        .sda   = hkI2C_SDA,
        .speed = hkI2C_SPEED
    }; I2C_Init(&i2c);

    DisplayConfig_t hkSH1107 = {
        .width    = 128,
        .height   = 128,
        .contrast = 0x4F,
        .address  = SSD1327_ADDRESS
    }; Display_Init(&i2c, &hkSH1107);


    hkClearBuffer();
    
    hkDrawChar(10, 10, 'A', 1);
    hkDrawString(10, 20, "IT WORKS!", 1);
    hkDrawRect(0, 0, 128, 128, 1);
    
    hkDisplay();

    sleep_ms(2000);

    while(FOREVER) tight_loop_contents();
}