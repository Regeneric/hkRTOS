#include <stdio.h>
#include <string.h>
    
#include <hardware/pio.h>
#include <hardware/clocks.h>
#include <hardware/dma.h>
#include <hardware/irq.h>

#include <sensors/sensors.h>
#include <sensors/dht11.h>
#include <sensors/dht11.pio.h>
    
#if hkDHT_USE_SENSOR && (hkDHT_USE_PIO && hkDHT_USE_DMA)
static u32 sgDHT11_SM_Offset = 0;
static u32 sgDHT11_DMA_Channel = 0;
static DHT_Config_t* sgDHT11_Config = NULL;

static void DHT11_DMA_ISR();
static void DHT11_ProcessData(DHT_Config_t* config);
static void DHT11_DMA_Init(const DHT_Config_t* config);

void DHT11_Init(DHT_Config_t* config) {
    if(config->status == DHT_INIT) printf("Reinitalizing DHT sensor...\n");

    sgDHT11_SM_Offset = pio_add_program(config->pio, &dht11_program);
    dht11_program_init(config->pio, config->sm, sgDHT11_SM_Offset, config->gpio);
    
    DHT11_DMA_Init(config);
    sgDHT11_Config = config;

    config->status = DHT_READ_SUCCESS;
    return;
}

static void DHT11_DMA_Init(const DHT_Config_t* config) {
    sgDHT11_DMA_Channel = dma_claim_unused_channel(true);
    dma_channel_config cfg = dma_channel_get_default_config(sgDHT11_DMA_Channel);
    
    channel_config_set_read_increment(&cfg, false);
    channel_config_set_write_increment(&cfg, true);
    channel_config_set_transfer_data_size(&cfg, DMA_SIZE_8);    // We need to transfer 5 bytes, 8 bits each 

    u32 dreq = pio_get_dreq(config->pio, config->sm, false);
    channel_config_set_dreq(&cfg, dreq);

    dma_channel_configure(sgDHT11_DMA_Channel, &cfg, NULL, &config->pio->rxf[config->sm], 5, false);
    irq_set_exclusive_handler(hkDHT_DMA_IRQ, DHT11_DMA_ISR);
    irq_set_enabled(hkDHT_DMA_IRQ, true);
    dma_channel_set_irq0_enabled(sgDHT11_DMA_Channel, true);

    return;
}

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
    DHT11_ProcessData(sgDHT11_Config);
    dma_hw->ints0 = (1u << sgDHT11_DMA_Channel);    // Clears the interrupt from the DMA
}
#endif