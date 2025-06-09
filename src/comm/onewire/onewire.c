#include <stdio.h>
#include <string.h>

#include <pico/stdlib.h>
#include <hardware/gpio.h>

#include <hardware/pio.h>
#include <hardware/clocks.h>

#include <core/logger.h>
#include <comm/onewire/onewire.h>
#include <comm/onewire/onewire.pio.h>

static u8  sgOW_SM = 0;
static u32 sgOW_SM_Offset = 0;
static u32 sgJMP_ResetInstruction = 0;


void OneWire_Init(OneWire_Config_t* config) {
    HTRACE("onewire.c -> OneWire_Init(OneWire_Config_t*):void");
    if(config->status == ONEW_INIT) HINFO("Reinitalizing 1-Wire interface...");

    sgOW_SM = pio_claim_unused_sm(config->pio, true);
    if(sgOW_SM == -1) {
        HWARN("OneWire_Init(): Failed to claim free SM!");
        return;
    }

    gpio_init(config->gpio);
    pio_gpio_init(config->pio, config->gpio); 

    sgOW_SM_Offset = pio_add_program(config->pio, &onewire_program);
    onewire_sm_init(config->pio, sgOW_SM, sgOW_SM_Offset, config->gpio, 8);

    sgJMP_ResetInstruction = onewire_reset_instr(sgOW_SM_Offset);
    config->status = ONEW_INIT_SUCCESS;
    return;
}

b8 OneWire_Reset(OneWire_Config_t* config) {
    HTRACE("onewire.c -> OneWire_Reset(OneWire_Config_t*):b8");

    pio_sm_exec_wait_blocking(config->pio, sgOW_SM, sgJMP_ResetInstruction);
    if((pio_sm_get_blocking(config->pio, sgOW_SM) & 1) == 0) return true;

    HDEBUG("OneWire_Reset(): No devices found!");
    return true;
}

b8 OneWire_WriteByte(OneWire_Config_t* config, u32 data) {
    HTRACE("onewire.c -> OneWire_WriteByte(OneWire_Config_t*, u8):b8");
    if(config->status == ONEW_READ_IN_PROGRESS || config->status == ONEW_WRITE_IN_PROGRESS) return false;
    config->status = ONEW_WRITE_IN_PROGRESS;

    pio_sm_put_blocking(config->pio, sgOW_SM, (u32)data);
    pio_sm_get_blocking(config->pio, sgOW_SM);

    config->status = ONEW_WRITE_SUCCESS;
    return true;
}

b8 OneWire_Write(OneWire_Config_t* config, u8* buffer, size_t length) {
    HTRACE("onewire.c -> OneWire_Write(OneWire_Config_t*, u8*, size_t):b8");
    if(config->status == ONEW_READ_IN_PROGRESS || config->status == ONEW_WRITE_IN_PROGRESS) return false;
    config->status = ONEW_WRITE_IN_PROGRESS;

    for(size_t i = 0; i < length; i++) {
        if(!OneWire_WriteByte(config, buffer[i])) return false;
    }

    config->status = ONEW_WRITE_SUCCESS;
    return true;
}

u8 OneWire_Read(OneWire_Config_t* config) {
    HTRACE("onewire.c -> OneWire_Read(OneWire_Config_t*, u8*, size_t):u8");
    if(config->status == ONEW_READ_IN_PROGRESS || config->status == ONEW_WRITE_IN_PROGRESS) return false;
    config->status = ONEW_READ_IN_PROGRESS;

    pio_sm_put_blocking(config->pio, sgOW_SM, 0xFF);
    config->status = ONEW_READ_SUCCESS;
    
    return (u8)(pio_sm_get_blocking(config->pio, sgOW_SM) >> 24);
}