#include <config/arm.h>
#if hkOW_USE_DMA

#include <stdio.h>

#include <pico/stdlib.h>

#include <core/logger.h>
#include <sensors/ds18b20/ds18b20.h>

static int64_t DS18B20_ConvertCallback(alarm_id_t id, void* userData);

static inline b8 DS18B20_CRC(const u8* data, size_t len) {
    HTRACE("ds18b20.c -> s:DS18B20_CRC(const u8*, size_t):b8");

    u8 crc = 0;
    while(len--) {
        u8 bit = *data++;
        for(u8 i = 8; i; i--) {
            u8 sum = (crc ^ bit) & 0x01;
            crc >>= 1;

            if(sum) crc ^= 0x8C;
            bit >>= 1;
        }
    } return crc == 0;  // If the final CRC is 0, data is valid
}

b8 DS18B20_Convert(OneWire_Config_t* ow, DS18B20_Config_t* config) {
    HTRACE("ds18b20.c -> s:DS18B20_Convert(OneWire_Config_t*):b8");

    if(config->state != DS18B20_STATE_IDLE) {
        HWARN("DS18B20_StartConversion: A conversion is already in progress.");
        return false;
    }

    if(OneWire_Reset(ow) == false) return false;
    
    u8 commands[2] = {ONEW_SKIP_ROM, DS18B20_CONVERT_T};
    OneWire_Write(ow, commands, sizeof(commands));

    config->state = DS18B20_STATE_CONVERT_CMD_SENT;
    return true;
}

void DS18B20_Poll(OneWire_Config_t* ow, DS18B20_Config_t* config) {
    if(ow->status == ONEW_DMA_IN_PROGRESS) return;

    switch(config->state) {
        case DS18B20_STATE_IDLE: break;
        case DS18B20_STATE_CONVERT_CMD_SENT:
            HDEBUG("DS18B20_Poll(): Conversion command sent. Starting 750ms timer.");
            config->state = DS18B20_STATE_WAITING_FOR_CONVERSION;
            add_alarm_in_ms(750, DS18B20_ConvertCallback, config, true);
        break;
        case DS18B20_STATE_WAITING_FOR_CONVERSION: break;
        case DS18B20_STATE_READY_TO_READ:
            HDEBUG("DS18B20_Poll(): Conversion complete. Reading scratchpad.");
            if(!OneWire_Reset(ow)) {
                config->state = DS18B20_STATE_IDLE;
                return;
            }

            u8 commands[2] = {ONEW_SKIP_ROM, DS18B20_READ_SCRATCHPAD};
            OneWire_Write(ow, commands, sizeof(commands));
            config->state = DS18B20_STATE_READ_CMD_SENT;
        break;
        case DS18B20_STATE_READ_CMD_SENT:
            OneWire_Read(ow, config->data, config->length);
            config->state = DS18B20_STATE_READING_SCRATCHPAD;
        break;
        case DS18B20_STATE_READING_SCRATCHPAD:
            if(!DS18B20_CRC(config->data, config->length)) HWARN("DS18B20_Poll(): CRC check failed. Data is corrupted.");
            else {
                i16 rawTemp = (i16)((config->data[1] << 8) | config->data[0]);
                config->temperature = (f32)rawTemp / 16.0f;
            }

            config->state = DS18B20_STATE_IDLE;
        break;
    }
}


static int64_t DS18B20_ConvertCallback(alarm_id_t id, void* userData) {
    DS18B20_Config_t* config = (DS18B20_Config_t*)userData;
    HDEBUG("DS18B20_ConvertCallback(): Alarm fired. Ready to read temperature.");
    
    config->state = DS18B20_STATE_READY_TO_READ;
    return 0;
}

#endif