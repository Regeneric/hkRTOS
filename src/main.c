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



// #include <stdio.h>
// #include "pico/stdlib.h"
// #include "hardware/pio.h"
// #include "hardware/clocks.h"

// // <<< REPLACE THE OLD ARRAY WITH THIS CORRECTED ONE >>>
// static const uint16_t onewire_write_program_instructions[] = {
//     // This PIO program has corrected timings for a 1MHz clock (1 cycle = 1us)
//     0x80a0, //  0: pull   block
//     0xe047, //  1: set    y, 7
//     0x6021, //  2: out    x, 1
//     0x0027, //  3: jmp    !x, 7
//     // --- Send a '1' bit ---
//     0xe581, //  4: set    pindirs, 1             [5]  ; Drive LOW for 6us
//     0xfb80, //  5: set    pindirs, 0             [27] ; Release HIGH and wait for 28us
//     0xa042, //  6: nop                           [32-28-1=3] -> wait for another ~30us
//             // Total slot time for '1' is ~6us (LOW) + ~59us (HIGH) = ~65us. CORRECT.
    
//     // --- Send a '0' bit ---
//     0xff81, //  7: set    pindirs, 1             [31] ; Drive LOW for 32us
//     0xa942, //  8: nop                           [9] ; Drive LOW for another 28us. Total LOW time = 60us. CORRECT.
//     0xe480, //  9: set    pindirs, 0             [4]  ; Release HIGH for 5us recovery time.
    
//     // --- Loop ---
//     0x0082, // 10: jmp    y--, 2
// };

// static const struct pio_program onewire_write_program = {
//     .instructions = onewire_write_program_instructions,
//     .length = 11, // UPDATE LENGTH
//     .origin = -1,
// };

// // The pin for 1-Wire communication
// #define ONEWIRE_PIN 13 // <<< SET THIS TO YOUR ACTUAL GPIO PIN NUMBER

// int main() {
//     stdio_init_all();
//     sleep_ms(2000); // Wait for terminal to connect
//     printf("Starting minimal PIO 1-Wire test...\n");

//     // 1. Setup PIO
//     PIO pio = pio0;
//     uint offset = pio_add_program(pio, &onewire_write_program);
//     uint sm = pio_claim_unused_sm(pio, true);

//     // 2. Configure the State Machine
//     pio_sm_config c = pio_get_default_sm_config();
//     sm_config_set_wrap(&c, offset, offset + 10);
//     sm_config_set_clkdiv(&c, (float)clock_get_hz(clk_sys) / 1000000.0f); // 1MHz clock
//     sm_config_set_out_shift(&c, true, false, 8); // Shift RIGHT (LSB first), NO autopull
//     sm_config_set_out_pins(&c, ONEWIRE_PIN, 1);
//     sm_config_set_set_pins(&c, ONEWIRE_PIN, 1);
    
//     pio_gpio_init(pio, ONEWIRE_PIN); // Initialize the pin for PIO control ONCE
//     pio_sm_set_consecutive_pindirs(pio, sm, ONEWIRE_PIN, 1, true); // Set initial direction to OUT
    
//     pio_sm_init(pio, sm, offset, &c);
//     pio_sm_set_enabled(pio, sm, true); // Enable the SM and leave it running

//     // 3. Main Test Loop
//     while (true) {
//         printf("Sending 0x02...\n");
//         pio_sm_put_blocking(pio, sm, 0x02);
//         sleep_ms(1000); // Wait long enough to see the result clearly

//         printf("Sending 0xCC...\n");
//         pio_sm_put_blocking(pio, sm, 0xCC);
//         sleep_ms(1000);
//     }
// }




void main(void) {
    stdio_init_all();

    // I2C_Config_t i2c = {
    //     .i2c   = hkI2C,
    //     .scl   = hkI2C_SCL,
    //     .sda   = hkI2C_SDA,
    //     .speed = hkI2C_SPEED
    // }; I2C_Init(&i2c);

    // DisplayConfig_t hkSH1107 = {
    //     .width    = 128,
    //     .height   = 128,
    //     .address  = SSD1327_ADDRESS
    // }; Display_Init(&i2c, &hkSH1107);

    OneWire_Config_t ow0 = {
        .gpio   = hkOW_PIN,
        .pio    = hkOW_PIO,
        .sm     = hkOW_PIO_SM,
        .status = ONEW_INIT
    }; OneWire_Init(&ow0);

    static u8 hkDS18B20_Data[9];
    DS18B20_Config_t hkDS18B20 = {
        .address     = ONEW_SKIP_ROM,
        .data        = hkDS18B20_Data,
        .length      = sizeof(hkDS18B20_Data),
        .queue       = NULL,
        .temperature = 0.0f
    }; 
    
    while(FOREVER) {
        DS18B20_Read(&ow0, &hkDS18B20);
        HDEBUG("TEMP: %.2f *C", hkDS18B20.temperature);

        sleep_ms(1000);
    }
}