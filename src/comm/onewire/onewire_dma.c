#include <config/arm.h>
#if hkOW_USE_DMA

#include <stdio.h>
#include <string.h>

#include <pico/stdlib.h>
#include <hardware/gpio.h>

#include <hardware/pio.h>
#include <hardware/clocks.h>
#include <hardware/dma.h>

#include <core/logger.h>
#include <comm/dma_irq_handler.h>
#include <comm/onewire/onewire.h>
#include <comm/onewire/onewire.pio.h>

static u8  sgOW_SM = 0;
static u32 sgOW_SM_Offset = 0;
static u32 sgJMP_ResetInstruction = 0;

static u32 sgOW_DMA_RX_Channel = 0;
static u32 sgOW_DMA_TX_Channel = 0;

static OneWire_Config_t* sgOneWire_Config = NULL;

static void OneWire_DMA_Init(OneWire_Config_t* config) {
    HTRACE("onewire_dma.c -> s:OneWire_DMA_Init(OneWire_Config_t*):void");

    sgOW_DMA_RX_Channel = dma_claim_unused_channel(true);
    dma_channel_config rxcfg = dma_channel_get_default_config(sgOW_DMA_RX_Channel);

    channel_config_set_read_increment(&rxcfg, false);
    channel_config_set_write_increment(&rxcfg, true);
    channel_config_set_transfer_data_size(&rxcfg, DMA_SIZE_8);
    channel_config_set_dreq(&rxcfg, pio_get_dreq(config->pio, config->sm, false));

    dma_channel_configure(sgOW_DMA_RX_Channel, &rxcfg, NULL, &config->pio->rxf[config->sm], 0, false);
    DMA_OneWire_RX_Register(config, sgOW_DMA_RX_Channel);
    
    if(hkOW_DMA_IRQ == DMA_IRQ_0) dma_channel_set_irq0_enabled(sgOW_DMA_RX_Channel, true);
    else dma_channel_set_irq1_enabled(sgOW_DMA_RX_Channel, true);


    sgOW_DMA_TX_Channel = dma_claim_unused_channel(true);
    dma_channel_config txcfg = dma_channel_get_default_config(sgOW_DMA_TX_Channel);

    channel_config_set_read_increment(&txcfg, true);
    channel_config_set_write_increment(&txcfg, false);
    channel_config_set_transfer_data_size(&txcfg, DMA_SIZE_8);
    channel_config_set_dreq(&txcfg, pio_get_dreq(config->pio, config->sm, false));

    dma_channel_configure(sgOW_DMA_TX_Channel, &txcfg, &config->pio->txf[config->sm], NULL, 0, false);
    DMA_OneWire_TX_Register(config, sgOW_DMA_TX_Channel);

    if(hkOW_DMA_IRQ == DMA_IRQ_0) dma_channel_set_irq0_enabled(sgOW_DMA_TX_Channel, true);
    else dma_channel_set_irq1_enabled(sgOW_DMA_TX_Channel, true);

    return;
}


void OneWire_Init(OneWire_Config_t* config) {
    HTRACE("onewire_dma.c -> OneWire_Init(OneWire_Config_t*):void");
    if(config->status == ONEW_INIT) HINFO("Reinitalizing 1-Wire interface...");

    sgOW_SM = pio_claim_unused_sm(config->pio, true);
    if(sgOW_SM == -1) {
        HWARN("OneWire_Init(): Failed to claim free SM!");
        return;
    }

    gpio_init(config->gpio);
    pio_gpio_init(config->pio, config->gpio); 

    sgOW_SM_Offset = pio_add_program(config->pio, &onewire_program);
    onewire_sm_init(config->pio, config->sm, sgOW_SM_Offset, config->gpio, config->bitMode);
    sgJMP_ResetInstruction = onewire_reset_instr(sgOW_SM_Offset);

    OneWire_DMA_Init(config);

    sgOneWire_Config = config;
    config->status = ONEW_INIT_SUCCESS;
    return;
}

b8 OneWire_Reset(OneWire_Config_t* config) {
    HTRACE("onewire_dma.c -> OneWire_Reset(OneWire_Config_t*):b8");

    pio_sm_exec_wait_blocking(config->pio, config->sm, sgJMP_ResetInstruction);
    if((pio_sm_get_blocking(config->pio, config->sm) & 1) == 0) return true;
    else {
        HDEBUG("OneWire_Reset(): No devices found!");
        return false;
    }
}

b8 OneWire_WriteByte(OneWire_Config_t* config, u32 data) {
    HTRACE("onewire.c -> OneWire_WriteByte(OneWire_Config_t*, u8):b8");
    if(config->status == ONEW_READ_IN_PROGRESS || config->status == ONEW_WRITE_IN_PROGRESS) {
        HDEBUG("OneWire_WriteByte(): Another read/write in progress!");
        return false;
    }
    config->status = ONEW_WRITE_IN_PROGRESS;

    pio_sm_put_blocking(config->pio, config->sm, (u32)data);
    pio_sm_get_blocking(config->pio, config->sm);

    config->status = ONEW_WRITE_SUCCESS;
    return true;
}

void OneWire_Write(OneWire_Config_t* config, u8* data, size_t len) {
    if(config->status == ONEW_DMA_IN_PROGRESS) return;
    config->status = ONEW_DMA_IN_PROGRESS;

    dma_channel_set_read_addr(sgOW_DMA_TX_Channel, data, len);
    dma_channel_set_trans_count(sgOW_DMA_TX_Channel, len, true);
}

void OneWire_Read(OneWire_Config_t* config) {
    if(config->status == ONEW_DMA_IN_PROGRESS) return;
    config->status = ONEW_DMA_IN_PROGRESS;

    static const u8 dummy = 0xFF;

    dma_channel_set_write_addr(sgOW_DMA_RX_Channel, config->data, false);
    dma_channel_set_trans_count(sgOW_DMA_RX_Channel, config->length, true);

    dma_channel_config txcfg = dma_channel_get_default_config(sgOW_DMA_TX_Channel);
    dma_channel_set_read_addr(sgOW_DMA_TX_Channel, &dummy, false);
    channel_config_set_read_increment(&txcfg, false);
    dma_channel_set_trans_count(sgOW_DMA_TX_Channel, config->length, true);
    channel_config_set_read_increment(&txcfg, true);
}

#endif