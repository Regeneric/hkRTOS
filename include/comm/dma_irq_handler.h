#pragma once
#include <defines.h>

void DMA_Master_Init();
void DMA_UART_Register(void* config, u32 channel);
void DMA_OneWire_RX_Register(void* config, u32 channel);
void DMA_OneWire_TX_Register(void* config, u32 channel);
void DMA_DHT_Register(void* config, u32 channel);

void DMA_UART_ISR();
void DMA_OneWire_ISR();
void DMA_DHT_ISR();