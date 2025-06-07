#include <stdio.h>

#include <sensors/ds18b20/ds18b20.h>

// b8 DS18B20_Address(OneWire_Config_t* ow, DS18B20_Config_t* config) {

// }

static inline b8 DS18B20_MatchRead(OneWire_Config_t* ow, DS18B20_Config_t* config) {
    if(config->length < 9) return false;

    u8 commands[9];
    commands[0] = DS18B20_MATCH_ROM;

    for(u8 byte = 0; byte < 8; ++byte) {
        // Unpack address into single bytes, LSB first
        commands[byte+1] = (config->address >> (byte*8)) & 0xFF;
    }

    if(OneWire_Reset(ow) == false) return false;
    if(OneWire_Write(ow, commands, sizeof(commands)) == false) {
        printf("Failed to send commands to the sensor\n");
        return false;
    }

    if(OneWire_WriteByte(ow, DS18B20_READ_SCRATCHPAD) == false) {
        printf("Failed to send commands to the sensor\n");
        return false;
    }

    OneWire_Read(ow, config->data, config->length);
    return true;
}

static inline  b8 DS18B20_SkipRead(OneWire_Config_t* ow, DS18B20_Config_t* config) {
    if(config->length < 9) return false;

    if(OneWire_Reset(ow) == false) return false;
    if(OneWire_WriteByte(ow, DS18B20_SKIP_ROM) == false) {              
        printf("Failed to send commands to the sensor\n");
        return false;
    }

    if(OneWire_WriteByte(ow, DS18B20_READ_SCRATCHPAD) == false) {
        printf("Failed to send commands to the sensor\n");
        return false;
    }

    OneWire_Read(ow, config->data, config->length);
    return true;
}

b8 DS18B20_Read(OneWire_Config_t* ow, DS18B20_Config_t* config) {
    if(config->address == DS18B20_SKIP_ROM) return DS18B20_SkipRead(ow, config);
    else return DS18B20_MatchRead(ow, config);
}