#include <stdio.h>
#include <string.h>
    
#include <hardware/pio.h>
#include <hardware/clocks.h>
#include <hardware/dma.h>
#include <hardware/irq.h>

#include <comm/dma_irq_handler.h>
#include <core/logger.h>
#include <sensors/sensors.h>
#include <sensors/dht11_22/dht11_22.h>
#include <sensors/dht11_22/dht11_22.pio.h>
    
#if hkDHT_USE_SENSOR && (hkDHT_USE_PIO && hkDHT_USE_DMA)
static u32 sgDHT11_SM_Offset = 0;
static u32 sgDHT11_DMA_Channel = 0;
static DHT_Config_t* sgDHT11_Config = NULL;

static void DHT_DMA_Init(const DHT_Config_t* config) {
    HTRACE("dht11_22_dma.c -> s:DHT_DMA_Init(DHT_Config_t*):void");

    sgDHT11_DMA_Channel = dma_claim_unused_channel(true);
    dma_channel_config cfg = dma_channel_get_default_config(sgDHT11_DMA_Channel);
    
    channel_config_set_read_increment(&cfg, false);
    channel_config_set_write_increment(&cfg, true);
    channel_config_set_transfer_data_size(&cfg, DMA_SIZE_8);    // We need to transfer 5 bytes, 8 bits each 

    u32 dreq = pio_get_dreq(config->pio, config->sm, false);
    channel_config_set_dreq(&cfg, dreq);

    dma_channel_configure(sgDHT11_DMA_Channel, &cfg, NULL, &config->pio->rxf[config->sm], 5, false);
    DMA_DHT_Register(config, sgDHT11_DMA_Channel);

    if(hkDHT_DMA_IRQ == DMA_IRQ_0) dma_channel_set_irq0_enabled(sgDHT11_DMA_Channel, true);
    else dma_channel_set_irq1_enabled(sgDHT11_DMA_Channel, true);

    return;
}


void DHT_Init(DHT_Config_t* config) {
    HTRACE("dht11_22_dma.c -> DHT_Init(DHT_Config_t*):void");

    if(config->status == DHT_INIT) HINFO("Reinitalizing DHT sensor...");

    sgDHT11_SM_Offset = pio_add_program(config->pio, &dht11_program);
    dht11_program_init(config->pio, config->sm, sgDHT11_SM_Offset, config->gpio);
    
    DHT_DMA_Init(config);
    sgDHT11_Config = config;

    config->status = DHT_READ_SUCCESS;
    return;
}

b8 DHT_Read(DHT_Config_t* config) {
    HTRACE("dht11_22_dma.c -> DHT_Read(DHT_Config_t*):b8");

    if(config->length < 5) {
        HERROR("DHT_Read(): Data buffer is too small!");
        return false;
    }
    if(config->status == DHT_READ_IN_PROGRESS) {
        HDEBUG("DHT_Read(): Another read process in progress!");
        return false;
    }

    memset(config->data, 0, config->length);
    config->status = DHT_READ_IN_PROGRESS;

    dma_channel_set_write_addr(sgDHT11_DMA_Channel, config->data, true);
    pio_sm_clear_fifos(config->pio, config->sm);
    pio_sm_set_enabled(config->pio , config->sm, true);
    pio_sm_put_blocking(config->pio, config->sm, 20000U);   // 20ms delay
    pio_sm_put_blocking(config->pio, config->sm, 30U);      // 30us delay

    return true;
}
#endif