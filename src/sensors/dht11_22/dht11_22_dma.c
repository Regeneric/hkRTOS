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

static void DHT_DMA_Init(DHT_Config_t* config) {
    HTRACE("dht11_22_dma.c -> s:DHT_DMA_Init(DHT_Config_t*):void");

    config->dmaChannel = dma_claim_unused_channel(true);
    dma_channel_config cfg = dma_channel_get_default_config(config->dmaChannel);
    
    channel_config_set_read_increment(&cfg, false);
    channel_config_set_write_increment(&cfg, true);
    channel_config_set_transfer_data_size(&cfg, DMA_SIZE_8);    // We need to transfer 5 bytes, 8 bits each 

    u32 dreq = pio_get_dreq(config->pio, config->sm, false);
    channel_config_set_dreq(&cfg, dreq);

    dma_channel_configure(config->dmaChannel, &cfg, NULL, &config->pio->rxf[config->sm], 5, false);
    DMA_DHT_Register(config, config->dmaChannel);

    if(hkDHT_DMA_IRQ == DMA_IRQ_0) dma_channel_set_irq0_enabled(config->dmaChannel, true);
    else dma_channel_set_irq1_enabled(config->dmaChannel, true);

    return;
}


void DHT_Init(DHT_Config_t* config) {
    HTRACE("dht11_22_dma.c -> DHT_Init(DHT_Config_t*):void");

    if(config->status == DHT_INIT) HINFO("Initalizing DHT sensor...");

    config->smOffset = pio_add_program(config->pio, &dht11_program);
    dht11_program_init(config->pio, config->sm, config->smOffset, config->gpio);
    
    DHT_DMA_Init(config);

    config->status = DHT_READ_SUCCESS;
    HINFO("DHT sensor has been initialized.");
    return;
}

b8 DHT_Read(DHT_Config_t* config) {
    HTRACE("dht11_22_dma.c -> DHT_Read(DHT_Config_t*):b8");

    if(config->length < 5) {
        HERROR("DHT_Read(): Data buffer is too small! You need at least 5 bytes!");
        return false;
    }
    // if(config->status == DHT_READ_IN_PROGRESS) {
        // HTRACE("DHT_Read(): Another read process in progress!");
        // return false;
    // }

    memset(config->rawData, 0, config->length);
    config->status = DHT_READ_IN_PROGRESS;

    dma_channel_set_write_addr(config->dmaChannel, config->rawData, true);
    pio_sm_clear_fifos(config->pio, config->sm);
    pio_sm_set_enabled(config->pio , config->sm, true);
    pio_sm_put_blocking(config->pio, config->sm, 20000U);   // 20ms delay
    pio_sm_put_blocking(config->pio, config->sm, 30U);      // 30us delay

    return true;
}
#endif