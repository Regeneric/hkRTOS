#pragma once
#include <defines.h>
#include <hardware/uart.h>

typedef enum {
    UART_INIT,
    UART_DATA_RX_IN_PROGRESS,
    UART_DATA_RX_SUCCESS,
    UART_DATA_TX_IN_PROGRSS,
    UART_DATA_TX_SUCCESS
} UART_Status_t;

typedef struct UART_Config_t {
    u8  tx;
    u8  rx;
    u32 baudrate;
    uart_inst_t* uart;
    u8* data;
    u8  length;
    vu8 status;
    u8  packetSize;
} UART_Config_t;

void UART_Init(UART_Config_t* config);
void UART_Stop(UART_Config_t* config);
void UART_ReadPacket(UART_Config_t* config);