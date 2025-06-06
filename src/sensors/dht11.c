#include <stdio.h>
#include <string.h>

// #include <FreeRTOS.h>
// #include <task.h>
// #include <queue.h>

#include <pico/stdlib.h>
#include <hardware/gpio.h>
#include <hardware/pio.h>
#include <hardware/clocks.h>
#include <hardware/dma.h>
#include <hardware/irq.h>
#include <hardware/sync.h>

#include <core/gpio.h>

#include <sensors/sensors.h>
#include <sensors/dht11.h>
#include <sensors/dht11.pio.h>

#if hkDHT11_USE_PIO
static u32 sgDHT11_SM_Offset = 0;

    #if hkDHT11_USE_DMA
    static u32 sgDHT11_DMA_Channel = 0;
    static DHT_Config_t* sgDHT11_Config = NULL;

    static void DHT11_ProcessData(DHT_Config_t* config) {
        if(config == NULL) {
            printf("DHT was not properly initialized!\n");
            return;
        }

        u8 checksum = (config->data[0] + config->data[1] + config->data[2] + config->data[3]) & 0xFF; 
        if(checksum != config->data[4]) {
            printf("Data read failed. Invalid checksum: 0x%x  0x%x\n", checksum, config->data[4]);
            config->status = DHT_READ_BAD_CHECKSUM;
            return;
        } config->status = DHT_READ_SUCCESS;
    }

    static void DHT11_DMA_ISR() {
        dma_irqn_acknowledge_channel(DMA_IRQ_0, sgDHT11_DMA_Channel);
        DHT11_ProcessData(sgDHT11_Config);
        dma_hw->ints0 = (1u << sgDHT11_DMA_Channel);    // Clears the interrupt from the DMA
    }

    static void DHT11_DMA_Init(const DHT_Config_t* config) {
        sgDHT11_DMA_Channel = dma_claim_unused_channel(true);
        dma_channel_config cfg = dma_channel_get_default_config(sgDHT11_DMA_Channel);
        
        channel_config_set_read_increment(&cfg, false);
        channel_config_set_write_increment(&cfg, true);
        channel_config_set_transfer_data_size(&cfg, DMA_SIZE_8);    

        u32 dreq = pio_get_dreq(config->pio, config->sm, false);
        channel_config_set_dreq(&cfg, dreq);

        dma_channel_configure(sgDHT11_DMA_Channel, &cfg, NULL, &config->pio->rxf[config->sm], 5, false);
        irq_set_exclusive_handler(DMA_IRQ_0, DHT11_DMA_ISR);
        irq_set_enabled(DMA_IRQ_0, true);
        dma_channel_set_irq0_enabled(sgDHT11_DMA_Channel, true);

        return;
    }
    #endif

    void DHT11_Init(DHT_Config_t* config) {
        if(config->status == DHT_INIT) printf("Reinitalizing DHT sensor...\n");

        sgDHT11_SM_Offset = pio_add_program(config->pio, &dht11_program);
        dht11_program_init(config->pio, config->sm, sgDHT11_SM_Offset, config->gpio);
        
        #if hkDHT11_USE_DMA
            DHT11_DMA_Init(config);
            sgDHT11_Config = config;
        #endif

        config->status = DHT_READ_SUCCESS;
        return;
    }

    #if hkDHT11_USE_DMA
    b8 DHT11_Read(DHT_Config_t* config) {
        if(config->length < 5) {
            printf("Data buffer is too small!\n");
            return false;
        }
        if(config->status == DHT_READ_IN_PROGRESS) {
            printf("Another read process in progress!\n");
            return false;
        }

        memset(config->data, 0, config->length);
        config->status = DHT_READ_IN_PROGRESS;

        dma_channel_set_write_addr(sgDHT11_DMA_Channel, config->data, true);
        pio_sm_clear_fifos(config->pio, config->sm);
        pio_sm_set_enabled(config->pio , config->sm, true);
        pio_sm_put_blocking(config->pio, config->sm, 20000U);
        pio_sm_put_blocking(config->pio, config->sm, 30U);

        return true;
    }
    #else
    b8 DHT11_Read(DHT_Config_t* config) {
        if(config->length < 5) return false;
        if(config->status == DHT_READ_IN_PROGRESS) return false;
        
        memset(config->data, 0, config->length);

        pio_sm_clear_fifos(config->pio, config->sm);
        pio_sm_set_enabled(config->pio , config->sm, true);
        pio_sm_put_blocking(config->pio, config->sm, 20000U);
        pio_sm_put_blocking(config->pio, config->sm, 30U);

        u32 val = 0;
        for(u8 byte = 0; byte < 5; ++byte) {
            u32 val = pio_sm_get_blocking(config->pio, config->sm);
            config->data[byte] = (u8)(val & 0xFF);
        } pio_sm_set_enabled(config->pio , config->sm, false);
        
        u8 checksum = (config->data[0] + config->data[1] + config->data[2] + config->data[3]) & 0xFF; 
        if(checksum != config->data[4]) {
            printf("Data read failed. Invalid checksum: 0x%x  0x%x", checksum, config->data[4]);
            return false;
        } return true;
    }
    #endif
#else
void DHT11_Init(const DHT_Config_t* config) {
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
#endif


// void DHT11_ReadTask(void* pvParameters) {
//     while(FOREVER) {
//         HPRINT("DHT11_ReadTask(): Task state is: running");
//         DHT11_Read((DHT_Config_t*)pvParameters);
//         vTaskDelay(1000/ portTICK_PERIOD_MS);
//     }
// }