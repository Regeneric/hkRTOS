#include <config/arm.h>
#if !hkOW_USE_DMA

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
    if(config->status == ONEW_INIT) HINFO("Initalizing 1-Wire interface...");

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
    config->status = ONEW_INIT_SUCCESS;

    HINFO("1-Wire interface has be initialized.");
    return;
}

b8 OneWire_Reset(OneWire_Config_t* config) {
    HTRACE("onewire.c -> OneWire_Reset(OneWire_Config_t*):b8");

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

b8 OneWire_Write(OneWire_Config_t* config, u8* buffer, size_t length) {
    HTRACE("onewire.c -> OneWire_Write(OneWire_Config_t*, u8*, size_t):b8");
    if(config->status == ONEW_READ_IN_PROGRESS || config->status == ONEW_WRITE_IN_PROGRESS) {
        HDEBUG("OneWire_Write(): Another read/write in progress!");
        return false;
    }
    config->status = ONEW_WRITE_IN_PROGRESS;

    for(size_t i = 0; i < length; i++) {
        if(!OneWire_WriteByte(config, buffer[i])) return false;
    }

    config->status = ONEW_WRITE_SUCCESS;
    return true;
}

u8 OneWire_Read(OneWire_Config_t* config) {
    HTRACE("onewire.c -> OneWire_Read(OneWire_Config_t*, u8*, size_t):u8");
    if(config->status == ONEW_READ_IN_PROGRESS || config->status == ONEW_WRITE_IN_PROGRESS) {
        HDEBUG("OneWire_Read(): Another read/write in progress!");
        return false;
    }
    config->status = ONEW_READ_IN_PROGRESS;

    pio_sm_put_blocking(config->pio, config->sm, 0xFF);
    config->status = ONEW_READ_SUCCESS;
    
    return (u8)(pio_sm_get_blocking(config->pio, config->sm) >> 24);
}

u8 OneWire_SearchROM(OneWire_Config_t* config, u64* devices, u8 maxDevices, u8 command) {
    HTRACE("onewire.c -> OneWire_SearchROM(OneWire_Config_t*, u64*, u8, u16):u64");

    u8 bitMode = config->bitMode;
    config->bitMode = 1;
    OneWire_Init(config);   // Driver in 1 bit mode

    b8 finished = false;
    u8 foundDevices = 0;
    u8 index = 0;

    u32 branchPoint = 0;
    u32 nextBranchPoint = -1;

    u64 address = 0ULL;
    
    while(finished == false && (maxDevices > 0 || foundDevices < maxDevices)) {
        finished = true;
        branchPoint = nextBranchPoint;
        if(OneWire_Reset(config) == false) {
            foundDevices = 0;
            finished = true;
            break;
        }
        
        // Search command as single bits
        for(u8 i = 0; i < 8; i++) OneWire_WriteByte(config, command >> i);
        
        // ROM address bits
        for(index = 0; index < 64; index++) {
            u32 a = OneWire_Read(config);
            u32 b = OneWire_Read(config);

            if(a == 0 && b == 0) {                  // Devices present
                if(index == branchPoint) {
                    OneWire_WriteByte(config, 1);
                    address |= (1ULL << index);
                } else {
                    if(index > branchPoint || (address & (1ULL << index)) == 0) {
                        OneWire_WriteByte(config, 0);
                        finished = false;
                        address &= ~(1ULL << index);
                        nextBranchPoint = index;
                    } else OneWire_WriteByte(config, 1);
                }
            } else if(a != 0 && b != 0) {           // Devices disconnected
                foundDevices = -2;
                finished = true;
                
                HDEBUG("OneWire_SearchROM(): No connected device found");
                break;
            } else {
                if(a == 0) {
                    OneWire_WriteByte(config, 0);
                    address &= ~(1ULL << index);
                } else {
                    OneWire_WriteByte(config, 1);
                    address |= (1ULL << index);
                }
            }
        }

        // if(devices != NULL) devices[foundDevices] = __builtin_bswap64(address);
        if(devices != NULL) devices[foundDevices] = address;
        foundDevices++;
    }

    config->bitMode = bitMode;
    OneWire_Init(config);
}

#endif