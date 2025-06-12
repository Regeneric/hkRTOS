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
#include <comm/onewire/onewire.h>
#include <comm/onewire/onewire.pio.h>

static u8  sgOW_SM = 0;
static u32 sgOW_SM_Offset = 0;
static u32 sgJMP_ResetInstruction = 0;

static u32 sgOW_DMA_RX_Channel = 0;
static u32 sgOW_DMA_TX_Channel = 0;

static OneWire_Config_t* sgOneWire_Config = NULL;

static void OneWire_DMA_ISR();


static void OneWire_DMA_Init(OneWire_Config_t* config) {
    HTRACE("onewire_dma.c -> s:OneWire_DMA_Init(OneWire_Config_t*):void");

    sgOW_DMA_RX_Channel = dma_claim_unused_channel(true);
    sgOW_DMA_TX_Channel = dma_claim_unused_channel(true);

    // RX Channel aka PIO -> Memory
    dma_channel_config rxCfg = dma_channel_get_default_config(sgOW_DMA_RX_Channel);
    channel_config_set_transfer_data_size(&rxCfg, DMA_SIZE_8);
    channel_config_set_read_increment(&rxCfg, false);           // Always read from the PIO FIFO address
    channel_config_set_write_increment(&rxCfg, true);           // But writing to a sequential address in our buffer
    channel_config_set_dreq(&rxCfg, pio_get_dreq(config->pio, config->sm, false));
    dma_channel_configure(sgOW_DMA_RX_Channel, &rxCfg, NULL, &config->pio->rxf[config->sm], 0, false);

    // TX Channel aka Memory -> PIO
    dma_channel_config txCfg = dma_channel_get_default_config(sgOW_DMA_TX_Channel);
    channel_config_set_transfer_data_size(&txCfg, DMA_SIZE_8);
    channel_config_set_read_increment(&txCfg, true);            // Read sequentially from our buffer
    channel_config_set_write_increment(&txCfg, false);          // Always write to the same PIO FIFO address
    channel_config_set_dreq(&txCfg, pio_get_dreq(config->pio, config->sm, true));
    dma_channel_configure(sgOW_DMA_TX_Channel, &txCfg, &config->pio->txf[config->sm], NULL, 0, false);

    irq_set_exclusive_handler(hkOW_DMA_IRQ, OneWire_DMA_ISR);
    irq_set_enabled(hkOW_DMA_IRQ, true);

    dma_channel_set_irq1_enabled(sgOW_DMA_RX_Channel, true);
}

static b8 OneWire_Transfer(u8* txBuffer, u8* rxBuffer, size_t length) {
    HTRACE("onewire_dma.c -> s:OneWire_Transfer(u8*, u8*):b8"); 

    dma_channel_set_write_addr(sgOW_DMA_RX_Channel, rxBuffer, false);
    dma_channel_set_trans_count(sgOW_DMA_RX_Channel, length, false);

    dma_channel_set_read_addr(sgOW_DMA_TX_Channel, txBuffer, false);
    dma_channel_set_trans_count(sgOW_DMA_TX_Channel, length, true);

    return true;
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

b8 OneWire_Write(OneWire_Config_t* config, u8* buffer, size_t length) {
    HTRACE("onewire_dma.c -> OneWire_Write (CPU Send)");
    if(config->status == ONEW_DMA_IN_PROGRESS) {
        return false;
    }
    config->status = ONEW_DMA_IN_PROGRESS;

    static u8 dummy_rx;
    dma_channel_set_write_addr(sgOW_DMA_RX_Channel, &dummy_rx, false);
    dma_channel_set_trans_count(sgOW_DMA_RX_Channel, length, true); // Trigger (i.e., enable) the channel

    for (size_t i = 0; i < length; ++i) pio_sm_put_blocking(config->pio, config->sm, buffer[i]);
    return true;


    // HTRACE("onewire_dma.c -> OneWire_WriteByte(OneWire_Config_t*, u8):b8");
    // if(config->status == ONEW_DMA_IN_PROGRESS) {
    //     HDEBUG("OneWire_WriteByte(): Another read/write in progress!");
    //     return false;
    // }
    // config->status = ONEW_DMA_IN_PROGRESS;

    // static u8 dummyRx;
    // OneWire_Transfer(buffer, &dummyRx, length);

    // return true;
}

u8 OneWire_Read(OneWire_Config_t* config, u8* buffer, size_t length) {
    HTRACE("onewire_dma.c -> OneWire_Read (CPU Send)");
    if(config->status == ONEW_DMA_IN_PROGRESS) {
        return false;
    }
    config->status = ONEW_DMA_IN_PROGRESS;

    dma_channel_set_write_addr(sgOW_DMA_RX_Channel, buffer, false);
    dma_channel_set_trans_count(sgOW_DMA_RX_Channel, length, true); // Trigger the channel

    for (size_t i = 0; i < length; ++i) pio_sm_put_blocking(config->pio, config->sm, 0xFF);
    return true;

    // HTRACE("onewire_dma.c -> OneWire_Read(OneWire_Config_t*, u8*, size_t):u8");
    // if(config->status == ONEW_DMA_IN_PROGRESS) {
    //     HDEBUG("OneWire_Read(): Another read/write in progress!");
    //     return false;
    // }
    // config->status = ONEW_DMA_IN_PROGRESS;

    // static u8 txBuffer[64];
    // if (config->length > sizeof(txBuffer)) {
    //     HERROR("OneWire_Read(): Requested read length is larger than the transfer buffer!");
    //     config->status = ONEW_INIT_SUCCESS;
    //     return false;
    // }

    // memset(txBuffer, 0xFF, config->length);
    // OneWire_Transfer(txBuffer, config->data, config->length);
    // return true;
}

static void OneWire_DMA_ISR() {
    HDEBUG("OneWire_DMA_ISR fired!");

    if(dma_channel_get_irq1_status(sgOW_DMA_RX_Channel)) {
        dma_channel_acknowledge_irq1(sgOW_DMA_RX_Channel);

        if(sgOneWire_Config != NULL) {
            sgOneWire_Config->status = ONEW_DMA_SUCCESS;
        }
    }
}

#endif