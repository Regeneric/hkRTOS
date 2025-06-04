#include <stdio.h>
#include <string.h>

#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>

#include <pico/stdlib.h>
#include <hardware/gpio.h>
#include <hardware/pio.h>
#include <hardware/clocks.h>

#include <core/gpio.h>

#include <sensors/sensors.h>
#include <sensors/dht11.h>
#include <sensors/dht11.pio.h>

static u32 sgDHT11_SM_InitOffset = 0;
static u32 sgDHT11_SM_ReadOffset = 0;
static const DHT_Config_t* sgConfig = NULL;

static inline void DHT11_ISR(void) {
    pio_interrupt_clear(sgConfig->pio, 0);
    pio_sm_set_enabled(sgConfig->pio, sgConfig->sm, false);
}

void DHT11_Init(const DHT_Config_t* config) {
    sgDHT11_SM_InitOffset = pio_add_program(config->pio, &dht11_init_program);
    dht11_init_program_init(config->pio, config->sm, sgDHT11_SM_InitOffset, config->gpio);

    irq_set_exclusive_handler(0, DHT11_ISR);
    irq_set_enabled(0, true);
    pio_interrupt_clear(config->pio, 0);

    sgConfig = config;
    return;
}

b8 DHT11_Read(DHT_Config_t* config) {
    if(config->length < 5) return false;
    memset(config->data, 0, config->length);

    sgDHT11_SM_ReadOffset = pio_add_program(config->pio, &dht11_read_program);
    dht11_read_program_init(config->pio, config->sm+1, sgDHT11_SM_ReadOffset, config->gpio);
    pio_sm_clear_fifos(config->pio, config->sm+1);

    pio_sm_set_enabled(config->pio , config->sm, true);
    pio_sm_put_blocking(config->pio, config->sm, 20000U);
    pio_sm_put_blocking(config->pio, config->sm, 30U);

    // Wait for all 5 bytes
    while(pio_sm_get_rx_fifo_level(config->pio, config->sm+1) < 5) {
        tight_loop_contents();
        printf("waiting\n");
    }

    for(u8 byte = 0; byte < 5; ++byte) {
        u32 val = pio_sm_get(config->pio, config->sm+1);
        config->data[byte] = (u8)(val & 0xFF);
    }

    pio_sm_set_enabled(config->pio , config->sm, false);
    return true;
}



// void DHT11_Init(const DHT_Config_t* config) {
//     sgDHT11_SM_Offset = pio_add_program(config->pio, &dht11_program);
//     pio_sm_config PIO_SM_Config = dht11_program_get_default_config(sgDHT11_SM_Offset);

//     sm_config_set_in_pins(&PIO_SM_Config, config->gpio);
//     sm_config_set_sideset_pins(&PIO_SM_Config, hkDHT11_LED_PIN);

//     // 1 PIO cycle = 1us
//     f32 clockDiv = (f32)clock_get_hz(clk_sys)/1000000.0f;
//     sm_config_set_clkdiv(&PIO_SM_Config, clockDiv);

//     pio_gpio_init(config->pio, config->gpio);
//     pio_sm_set_consecutive_pindirs(config->pio, config->sm, config->gpio, 1, GPIO_IN);

//     pio_gpio_init(config->pio, hkDHT11_LED_PIN);
//     pio_sm_set_consecutive_pindirs(config->pio, config->sm, hkDHT11_LED_PIN, 1, GPIO_OUT);

//     pio_sm_init(config->pio, config->sm, sgDHT11_SM_Offset, &PIO_SM_Config);
//     pio_sm_set_enabled(config->pio, config->sm, false);      // Don't start SM, yet

//     return;
// }   

// b8 DHT11_Read(DHT_Config_t* config) {
//     if(config->length < 5) return false;
//     memset(config->data, 0, config->length);

//     gpio_init(config->gpio);

//     // Start signal
//     gpio_set_dir(config->gpio, GPIO_OUT);
//     gpio_put(config->gpio, GPIO_LOW);
//     sleep_ms(20);

//     gpio_put(config->gpio, GPIO_HIGH);
//     sleep_us(30);

//     gpio_set_dir(config->gpio, GPIO_IN);

//     // First handshake
//     if(!gpio_wait_for_level(config->gpio, GPIO_LOW, 100)) {
//         printf("First handshake failed\n"); 
//         return false;
//     }

//     // Second handshake
//     if(!gpio_wait_for_level(config->gpio, GPIO_HIGH, 100)) {
//         printf("Second handshake failed\n"); 
//         return false;
//     }

//     pio_gpio_init(config->pio, config->gpio);
//     pio_sm_set_consecutive_pindirs(config->pio, config->sm, config->gpio, 1, GPIO_IN);
//     pio_sm_clear_fifos(config->pio, config->sm);
//     pio_sm_set_enabled(config->pio, config->sm, true);

//     for(u8 i = 0; i < 5; ++i) {
//         u32 start = to_us_since_boot(get_absolute_time());
//         while(pio_sm_is_rx_fifo_empty(config->pio, config->sm)) {
//             if(to_us_since_boot(get_absolute_time()) - start > 5000) {
//                 pio_sm_set_enabled(config->pio, config->sm, false);
//                 return false;
//             }
//         }

//         u32 word = pio_sm_get_blocking(config->pio, config->sm);
//         config->data[i] = (u8)(word & 0xFF);    // We only care for 8 LSB bits
//     }

//     for(u8 i = 0; i < 5; ++i) {
//         printf("data[%u]: %u\n", i, config->data[i]);
//     }

//     pio_sm_set_enabled(config->pio, config->sm, false);

//     u8 checksum = (config->data[0] + config->data[1] + config->data[2] + config->data[3]) & 0xFF; 
//     if(checksum != config->data[4]) {
//         printf("Data read failed. Invalid checksum: 0x%x  0x%x", checksum, config->data[4]);
//         return false;
//     } return true;
// }







// void DHT11_Init(const DHT_Config_t* config) {
//     gpio_init(config->gpio);    
//     sleep_ms(2000);     // Wait for sensor to boot 
//     return;
// }

// b8 DHT11_Read(DHT_Config_t* config) {
//     memset(config->data, 0, config->length);

//     // Start signal
//     gpio_set_dir(config->gpio, GPIO_OUT);
//     gpio_put(config->gpio, GPIO_LOW);
//     sleep_ms(20);

//     gpio_put(config->gpio, GPIO_HIGH);
//     sleep_us(30);

//     gpio_set_dir(config->gpio, GPIO_IN);

//     // First handshake
//     if(!gpio_wait_for_level(config->gpio, GPIO_LOW, 100)) {
//         printf("First handshake failed\n"); 
//         return false;
//     }

//     // Second handshake
//     if(!gpio_wait_for_level(config->gpio, GPIO_HIGH, 100)) {
//         printf("Second handshake failed\n"); 
//         return false;
//     }

//     // Data transmission
//     u32 timeout = 0;
//     for(u8 bit = 0; bit < 40; ++bit) {
//         u8 byteIndex = (u8)(bit/8);     // Counts from 0 to 4
//         config->data[byteIndex] <<= 1;  // Shifts 0 as LSB

//         // Start bit
//         if(!gpio_wait_for_level_count(config->gpio, GPIO_LOW, 70, &timeout)) {
//             printf("Start bit %u failed: TIMEOUT\n", bit); 
//             return false;
//         }

//         // Data bit
//         if(!gpio_wait_for_level_count(config->gpio, GPIO_HIGH, 90, &timeout)) {
//             printf("Data bit %u failed: TIMEOUT\n", bit); 
//             return false;
//         }

//         if(timeout >= 20 && timeout <= 35) continue;
//         else if(timeout >= 65 && timeout <= 90) config->data[byteIndex] |= 1;
//         else return false;
//     }

//     u8 checksum = (config->data[0] + config->data[1] + config->data[2] + config->data[3]) & 0xFF; 
//     if(checksum != config->data[4]) {
//         printf("Data read failed. Invalid checksum: 0x%x  0x%x", checksum, config->data[4]);
//         return false;
//     } return true;
// }


// void DHT11_ReadTask(void* pvParameters) {
//     while(FOREVER) {
//         HPRINT("DHT11_ReadTask(): Task state is: running");
//         DHT11_Read((DHT_Config_t*)pvParameters);
//         vTaskDelay(1000/ portTICK_PERIOD_MS);
//     }
// }