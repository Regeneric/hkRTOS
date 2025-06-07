#include <stdio.h>
#include <string.h>

#include <pico/stdlib.h>
#include <hardware/gpio.h>

#include <hardware/pio.h>
#include <hardware/clocks.h>

#include <comm/onewire/onewire.h>
#include <comm/onewire/onewire_reset.pio.h>
#include <comm/onewire/onewire_write.pio.h>
#include <comm/onewire/onewire_read.pio.h>

static u32 sgOW_Reset_SM_Offset = 0;
static u32 sgOW_Write_SM_Offset = 0;
static u32 sgOW_Read_SM_Offset  = 0;

static u8 sgOW_Reset_SM = 0;
static u8 sgOW_Write_SM = 0;
static u8 sgOW_Read_SM  = 0;

void OneWire_Init(OneWire_Config_t* config) {
    if(config->status == ONEW_INIT) printf("Reinitalizing 1-Wire interface...\n");

    sgOW_Reset_SM_Offset = pio_add_program(config->pio, &onewire_reset_program);
    sgOW_Write_SM_Offset = pio_add_program(config->pio, &onewire_write_program);
    sgOW_Read_SM_Offset  = pio_add_program(config->pio, &onewire_read_program);
    
    sgOW_Reset_SM = config->sm;
    sgOW_Write_SM = config->sm+1;
    sgOW_Read_SM  = config->sm+2;

    onewire_reset_program_init(config->pio, sgOW_Reset_SM, sgOW_Reset_SM_Offset, config->gpio);
    onewire_write_program_init(config->pio, sgOW_Write_SM, sgOW_Write_SM_Offset, config->gpio);
    onewire_read_program_init(config->pio , sgOW_Read_SM , sgOW_Read_SM_Offset , config->gpio);

    config->status = ONEW_INIT_SUCCESS;
    return;
}

b8 OneWire_WriteByte(OneWire_Config_t* config, u8 data) {
    if(config->status == ONEW_READ_IN_PROGRESS || config->status == ONEW_WRITE_IN_PROGRESS) return false;
    config->status = ONEW_WRITE_IN_PROGRESS;

    pio_sm_clear_fifos(config->pio, sgOW_Reset_SM);
    pio_sm_clear_fifos(config->pio, sgOW_Write_SM);

    // 1-Wire reset sequence
    pio_sm_set_enabled(config->pio, sgOW_Reset_SM, false);
    pio_sm_put_blocking(config->pio, sgOW_Reset_SM, 480);
    pio_sm_set_enabled(config->pio, sgOW_Reset_SM, true);

    if(pio_sm_get_blocking(config->pio, sgOW_Reset_SM) != 0) {
        pio_sm_set_enabled(config->pio, sgOW_Reset_SM, false);
        printf("No devices found!\n");
        return false;
    }

    // 1-Wire write byte
    pio_sm_set_enabled(config->pio, sgOW_Reset_SM, false);
    pio_sm_put_blocking(config->pio, sgOW_Write_SM, data);
    pio_sm_set_enabled(config->pio, sgOW_Write_SM, true);

    while(!pio_sm_is_tx_fifo_empty(config->pio, sgOW_Reset_SM)) tight_loop_contents();
    config->status = ONEW_WRITE_SUCCESS;

    return true;
}

b8 OneWire_Write(OneWire_Config_t* config, u8* buffer, size_t length) {
    if(config->status == ONEW_READ_IN_PROGRESS || config->status == ONEW_WRITE_IN_PROGRESS) return false;
    config->status = ONEW_WRITE_IN_PROGRESS;

    pio_sm_clear_fifos(config->pio, sgOW_Reset_SM);
    pio_sm_clear_fifos(config->pio, sgOW_Write_SM);

    // 1-Wire reset sequence
    pio_sm_set_enabled(config->pio, sgOW_Reset_SM, false);
    pio_sm_put_blocking(config->pio, sgOW_Reset_SM, 480);
    pio_sm_set_enabled(config->pio, sgOW_Reset_SM, true);

    if(pio_sm_get_blocking(config->pio, sgOW_Reset_SM) != 0) {
        pio_sm_set_enabled(config->pio, sgOW_Reset_SM, false);
        printf("No devices found!\n");
        return false;
    }

    // 1-Wire write buffer
    pio_sm_set_enabled(config->pio, sgOW_Reset_SM, false);
    pio_sm_set_enabled(config->pio, sgOW_Write_SM, true);

    for(size_t byte = 0; byte < length; ++byte) {
        pio_sm_put_blocking(config->pio, sgOW_Write_SM, buffer[byte]);
    }

    while(!pio_sm_is_tx_fifo_empty(config->pio, sgOW_Reset_SM)) tight_loop_contents();
    config->status = ONEW_WRITE_SUCCESS;

    return true;
}