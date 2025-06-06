#include <stdio.h>
#include <string.h>

#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>

#include <pico/stdlib.h>
#include <hardware/gpio.h>

#include <core/gpio.h>

#include <sensors/sensors.h>
#include <sensors/dht11.h>


#if hkDHT_USE_SENSOR && (!hkDHT_USE_PIO && !hkDHT_USE_DMA)
void DHT11_Init(DHT_Config_t* config) {
    gpio_init(config->gpio);    
    sleep_ms(2000);     // Wait for sensor to boot 
    return;
}

b8 DHT11_Read(DHT_Config_t* config) {
    memset(config->data, 0, config->length);

    // Start signal
    gpio_set_dir(config->gpio, GPIO_OUT);
    gpio_put(config->gpio, GPIO_LOW);
    sleep_ms(20);

    gpio_put(config->gpio, GPIO_HIGH);
    sleep_us(30);

    gpio_set_dir(config->gpio, GPIO_IN);

    // First handshake
    if(!gpio_wait_for_level(config->gpio, GPIO_LOW, 100)) {
        printf("First handshake failed\n"); 
        return false;
    }

    // Second handshake
    if(!gpio_wait_for_level(config->gpio, GPIO_HIGH, 100)) {
        printf("Second handshake failed\n"); 
        return false;
    }

    // Data transmission
    u32 timeout = 0;
    for(u8 bit = 0; bit < 40; ++bit) {
        u8 byteIndex = (u8)(bit/8);     // Counts from 0 to 4
        config->data[byteIndex] <<= 1;  // Shifts 0 as LSB

        // Start bit
        if(!gpio_wait_for_level_count(config->gpio, GPIO_LOW, 70, &timeout)) {
            printf("Start bit %u failed: TIMEOUT\n", bit); 
            return false;
        }

        // Data bit
        if(!gpio_wait_for_level_count(config->gpio, GPIO_HIGH, 90, &timeout)) {
            printf("Data bit %u failed: TIMEOUT\n", bit); 
            return false;
        }

        if(timeout >= 20 && timeout <= 35) continue;
        else if(timeout >= 65 && timeout <= 90) config->data[byteIndex] |= 1;
        else return false;
    }

    u8 checksum = (config->data[0] + config->data[1] + config->data[2] + config->data[3]) & 0xFF; 
    if(checksum != config->data[4]) {
        printf("Data read failed. Invalid checksum: 0x%x  0x%x", checksum, config->data[4]);
        return false;
    } return true;
}


void DHT11_ReadTask(void* pvParameters) {
    while(FOREVER) {
        HPRINT("DHT11_ReadTask(): Task state is: running");
        DHT11_Read((DHT_Config_t*)pvParameters);
        vTaskDelay(1000/ portTICK_PERIOD_MS);
    }
}
#endif