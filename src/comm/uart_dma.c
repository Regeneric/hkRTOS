#include <pico/stdlib.h>
#include <hardware/gpio.h>
#include <hardware/uart.h>
#include <hardware/dma.h>
#include <hardware/irq.h>

#include <core/logger.h>
#include <comm/uart.h>


static u32 sgUART_DMA_Channel = 0;
static UART_Config_t* sgUART_Config;

static void UART_DMA_ISR();


static void UART_DMA_Init(const UART_Config_t* uart) {
    sgUART_DMA_Channel = dma_claim_unused_channel(true);
    dma_channel_config cfg = dma_channel_get_default_config(sgUART_DMA_Channel);

    channel_config_set_read_increment(&cfg, false);
    channel_config_set_write_increment(&cfg, true);
    channel_config_set_transfer_data_size(&cfg, DMA_SIZE_8);    // Each data packet is 8 bit
    channel_config_set_dreq(&cfg, uart_get_dreq_num(uart->uart, false));

    dma_channel_configure(sgUART_DMA_Channel, &cfg, NULL, &uart_get_hw(uart->uart)->dr, uart->packetSize, false);
    irq_set_exclusive_handler(DMA_IRQ_1, UART_DMA_ISR);
    irq_set_enabled(DMA_IRQ_1, true);
    dma_channel_set_irq1_enabled(sgUART_DMA_Channel, true);

    return;
}


void UART_Init(UART_Config_t* config) {
    HTRACE("uart.c -> UART_Init(const UART_Config_t*):void");

    uart_init(config->uart, config->baudrate);

    gpio_set_function(config->tx, GPIO_FUNC_UART);
    gpio_set_function(config->rx, GPIO_FUNC_UART);

    uart_set_fifo_enabled(config->uart, true);

    UART_DMA_Init(config);
    sgUART_Config = config;
}

void UART_Stop(UART_Config_t* config) {
    HTRACE("uart.c -> UART_Stop(const UART_Config_t*):void");

    uart_deinit(config->uart);

    gpio_deinit(config->tx);
    gpio_deinit(config->rx);
}

void UART_ReadPacket(UART_Config_t* config) {
    sgUART_Config->status = UART_DATA_RX_IN_PROGRESS;
    dma_channel_set_write_addr(sgUART_DMA_Channel, config->data, true);
    return;
}

static void UART_DMA_ISR() {
    sgUART_Config->status = UART_DATA_RX_SUCCESS;
    dma_hw->ints1 = (1u << sgUART_DMA_Channel);
}