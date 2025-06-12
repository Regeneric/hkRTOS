#include <config/arm.h>
#if !hkOW_USE_DMA

#include <stdio.h>

#include <pico/stdlib.h>

#include <core/logger.h>
#include <sensors/ds18b20/ds18b20.h>


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

static inline b8 DS18B20_Convert(OneWire_Config_t* ow, u64 address) {
    HTRACE("ds18b20.c -> s:DS18B20_Convert(OneWire_Config_t*):b8");
    
    if(OneWire_Reset(ow) == false) return false;

    if(address == ONEW_SKIP_ROM) {
        if(OneWire_WriteByte(ow, ONEW_SKIP_ROM) == false) return false;
    } else {
        u8 commands[2];
        commands[0] = ONEW_MATCH_ROM;
        commands[1] = address;

        // Not implemented, yet
        HERROR("DS18B20_Convert(): MATCH_ROM is not implemented, yet");
        return false;
        
        if(OneWire_Write(ow, commands, sizeof(commands)) == false) return false;
    }
    if(OneWire_WriteByte(ow, DS18B20_CONVERT_T) == false) return false;

    while(OneWire_Read(ow) == 0);   // Wait for conversion to end
    return true;
}

static inline b8 DS18B20_MatchRead(OneWire_Config_t* ow, DS18B20_Config_t* config) {
    HTRACE("ds18b20.c: s:DS18B20_MatchRead(OneWire_Config_t*, DS18B20_Config_t*):b8");

    if(config->length < 9) return false;

    u8 commands[2];
    commands[0] = ONEW_MATCH_ROM;
    commands[1] = config->address;

    // Not implemented, yet
    HERROR("DS18B20_Convert(): MATCH_ROM is not implemented, yet");
    return false;

    if(OneWire_Reset(ow) == false) return false;
    if(OneWire_Write(ow, commands, sizeof(commands)) == false) {
        HDEBUG("DS18B20_MatchRead(): Failed to send commands to the sensor");
        return false;
    }

    if(OneWire_WriteByte(ow, DS18B20_READ_SCRATCHPAD) == false) {
        HDEBUG("DS18B20_MatchRead(): Failed to send commands to the sensor");
        return false;
    }

    // Read all 9 bytes from the sensor into the data buffer.
    for (u8 i = 0; i < config->length; i++) config->data[i] = OneWire_Read(ow);
    return true;
}

static inline b8 DS18B20_SkipRead(OneWire_Config_t* ow, DS18B20_Config_t* config) {
    HTRACE("ds18b20.c: s:DS18B20_SkipRead(OneWire_Config_t*, DS18B20_Config_t*):b8");

    if(config->length < 9) return false;

    if(OneWire_Reset(ow) == false) return false;
    if(OneWire_WriteByte(ow, ONEW_SKIP_ROM) == false) {              
        HDEBUG("DS18B20_SkipRead(): Failed to send commands to the sensor");
        return false;
    }

    if(OneWire_WriteByte(ow, DS18B20_READ_SCRATCHPAD) == false) {
        HDEBUG("DS18B20_SkipRead(): Failed to send commands to the sensor");
        return false;
    }

    // Read all 9 bytes from the sensor into the data buffer.
    for (u8 i = 0; i < config->length; i++) config->data[i] = OneWire_Read(ow);
    return true;
}

u8 DS18B20_Read(OneWire_Config_t* ow, DS18B20_Config_t* config) {
    HTRACE("ds18b20.c: DS18B20_Read(OneWire_Config_t*, DS18B20_Config_t*):u8");

    if(DS18B20_Convert(ow, config->address) == false) {
        HDEBUG("DS18B20_Read(): Failed to start conversion");
        return false;
    }

    u8 result = 0;
    if(config->address == ONEW_SKIP_ROM) result = DS18B20_SkipRead(ow, config);
    else result = DS18B20_MatchRead(ow, config);

    if(!DS18B20_CRC(config->data, config->length)) {
        HWARN("DS18B20_Read(): CRC check failed. Data is corrupted.");
        return false;
    }

    if(!result) return result;  // Something went wrong, early return

    i16 rawTemp = (i16)((config->data[1] << 8) | config->data[0]);
    f32 temp = (f32)rawTemp/16.0f;

    config->temperature = temp;
    return result;
}

#endif