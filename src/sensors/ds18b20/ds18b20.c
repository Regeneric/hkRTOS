#include <config/arm.h>
#if !hkOW_USE_DMA

#include <stdio.h>

#include <pico/stdlib.h>

#include <core/logger.h>
#include <sensors/ds18b20/ds18b20.h>


static b8 DS18B20_CRC(const u8* data, size_t len) {
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

static b8 DS18B20_Convert(OneWire_Config_t* ow, u64 address) {
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

    // while(OneWire_Read(ow) == 0);   // Wait for conversion to end
    return true;
}

static inline b8 DS18B20_MatchRead(OneWire_Config_t* ow, DS18B20_Config_t* config) {
    HTRACE("ds18b20.c -> s:DS18B20_MatchRead(OneWire_Config_t*, DS18B20_Config_t*):b8");
    if(config->length < 9) return false;
    HERROR("DS18B20_Convert(): MATCH_ROM is not implemented, yet");
    return true;
}

static inline b8 DS18B20_SkipRead(OneWire_Config_t* ow, DS18B20_Config_t* config) {
    HTRACE("ds18b20.c -> s:DS18B20_SkipRead(OneWire_Config_t*, DS18B20_Config_t*):b8");

    if(config->length < 9) return false;

    if(OneWire_Reset(ow) == false) return false;
    if(OneWire_WriteByte(ow, ONEW_SKIP_ROM) == false) {              
        HWARN("DS18B20_SkipRead(): Failed to send commands to the sensor");
        return false;
    }

    if(OneWire_WriteByte(ow, DS18B20_READ_SCRATCHPAD) == false) {
        HWARN("DS18B20_SkipRead(): Failed to send commands to the sensor");
        return false;
    }

    // Read all 9 bytes from the sensor into the data buffer.
    for (u8 i = 0; i < config->length; i++) config->data[i] = OneWire_Read(ow);
    return true;
}

u8 DS18B20_ReadAndProcess(OneWire_Config_t* ow, DS18B20_Config_t* config, DS18B20_DataPacket_t* data) {
    HTRACE("ds18b20.c -> DS18B20_ReadAndProcess(OneWire_Config_t*, DS18B20_Config_t*, DS18B20_DataPacket_t*):u8");

    if(DS18B20_Convert(ow, config->address) == false) {
        HWARN("DS18B20_Read(): Failed to start conversion");
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

    data->temperature = temp;
    data->address = ONEW_SKIP_ROM ? 0x00 : config->address;
    return result;
}

u8 DS18B20_ProcessData(OneWire_Config_t* ow, DS18B20_Config_t* config, DS18B20_DataPacket_t* data) {
    HTRACE("ds18b20.c -> DS18B20_ProcessData(OneWire_Config_t*, DS18B20_Config_t*, DS18B20_DataPacket_t*):u8");

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

    data->temperature = temp;
    data->address = ONEW_SKIP_ROM ? 0x00 : config->address;
    return result;
}

void DS18B20_TickRead(DS18B20_Config_t* config) {
    HTRACE("ds18b20.c -> DS18B20_Read(DS18B20_Config_t*):void");
    if(config->state == DS18B20_IDLE) config->state = DS18B20_START_CONVERSION;
}

void DS18B20_Tick(OneWire_Config_t* ow, DS18B20_Config_t* config, DS18B20_DataPacket_t* data) {
    HTRACE("ds18b20.c -> DS18B20_Tick(OneWire_Config_t*, DS18B20_Config_t*, DS18B20_DataPacket_t):void");

    switch(config->state) {
        case DS18B20_START_CONVERSION: {
            HTRACE("DS18B20_Tick(): DS18B20_START_CONVERSION");

            if(OneWire_Reset(ow)) {
                OneWire_WriteByte(ow, ONEW_SKIP_ROM);
                OneWire_WriteByte(ow, DS18B20_CONVERT_T);

                config->convertStartTime = get_absolute_time();
                config->state = DS18B20_WAITING_FOR_CONVERSION;
            } else config->state = DS18B20_IDLE;
        } break;

        case DS18B20_WAITING_FOR_CONVERSION: {
            HTRACE("DS18B20_Tick(): DS18B20_WAITING_FOR_CONVERSION");
            if(absolute_time_diff_us(config->convertStartTime, get_absolute_time()) > 750000) config->state = DS18B20_START_READ;
        } break;
        
        case DS18B20_START_READ: {
            HTRACE("DS18B20_Tick(): DS18B20_START_READ");
            if(OneWire_Reset(ow)) {
                OneWire_WriteByte(ow, ONEW_SKIP_ROM);
                OneWire_WriteByte(ow, DS18B20_READ_SCRATCHPAD);

                config->dataCount = 0;
                config->state = DS18B20_READING_SCRATCHPAD;
            } else config->state = DS18B20_IDLE;
        } break;

        case DS18B20_READING_SCRATCHPAD: {
            HTRACE("DS18B20_Tick(): DS18B20_READING_SCRATCHPAD");
            if(config->dataCount < config->length) config->data[config->dataCount++] = OneWire_Read(ow);
            if(config->dataCount >= config->length) config->state = DS18B20_PROCESS_DATA;
        } break;

        case DS18B20_PROCESS_DATA: {
            HTRACE("DS18B20_PROCESS_DATA");
            if(DS18B20_CRC(config->data, config->length)) {
                i16 rawTemp = (i16)((config->data[1] << 8) | config->data[0]);
                data->temperature = (f32)rawTemp / 16.0f;
                data->address = (config->address == ONEW_SKIP_ROM) ? 0x00 : config->address;
                
                data->status = DS18B20_DATA_READY; 
            } else {
                HWARN("DS18B20_Tick(): CRC check failed. Data is corrupted.");
                data->status = DS18B20_DATA_ERROR; 
            }

            config->state = DS18B20_IDLE;
        } break;
        
        case DS18B20_IDLE:
        default: break;
    }
}

b8 DS18B20_SetResolution(OneWire_Config_t* ow, u8 resolution) {
    HTRACE("ds18b20.c -> DS18B20_SetResolution(OneWire_Config_t*, u8):b8");
    u8 configByte;

    switch(resolution) {
        case 9:
            configByte = 0x1F;
            break;
        case 10:
            configByte = 0x3F;
            break;
        case 11:
            configByte = 0x5F;
            break;
        case 12:
        default: // Default to 12-bit for safety
            resolution = 12;
            configByte = 0x7F;
            break;
    }

    HINFO("Setting DS18B20 resolution to %d-bit", resolution);

    if(!OneWire_Reset(ow)) return false;
    OneWire_WriteByte(ow, ONEW_SKIP_ROM);
    OneWire_WriteByte(ow, DS18B20_WRITE_SCRATCHPAD);

    OneWire_WriteByte(ow, 0x00);         // TH Register (dummy value)
    OneWire_WriteByte(ow, 0x00);         // TL Register (dummy value)
    OneWire_WriteByte(ow, configByte);   

    if(!OneWire_Reset(ow)) return false;
    OneWire_WriteByte(ow, ONEW_SKIP_ROM);
    OneWire_WriteByte(ow, DS18B20_COPY_SCRATCHPAD);
    sleep_ms(12);

    HINFO("Resolution change complete.");
    return true;
}
#endif


void vDS18B20_Task(void* pvParameters) {
    HTRACE("ds18b20.c -> vDS18B20_Task(void*):void");

    DS18B20_TaskParams_t* params = (DS18B20_TaskParams_t*)pvParameters;
    UBaseType_t coreID = portGET_CORE_ID();

    // Error loop
    while(DS18B20_SetResolution(params->ow, params->ds18b20->resolution) != true) {
        HFATAL("vDS18B20_Task(): DS18B20 failed to initialize! Retrying in 10 seconds...");
        vTaskDelay(pdMS_TO_TICKS(10000));
    } 

    vTaskDelay(pdMS_TO_TICKS(100));
    while(FOREVER) {
        HTRACE("vDS18B20_Task(): Running on core {%d}", (u16)coreID);

        DS18B20_Convert(params->ow, params->ds18b20->address);
        switch(params->ds18b20->resolution) {
            case  9: vTaskDelay(pdMS_TO_TICKS(95));  break;
            case 10: vTaskDelay(pdMS_TO_TICKS(190)); break;
            case 11: vTaskDelay(pdMS_TO_TICKS(380)); break;
            case 12: vTaskDelay(pdMS_TO_TICKS(755)); break;
            default: vTaskDelay(pdMS_TO_TICKS(800)); break;
        }

        DS18B20_ProcessData(params->ow, params->ds18b20, params->data);
        xQueueSend(params->ds18b20->queue, params->data, 0);
    }
}