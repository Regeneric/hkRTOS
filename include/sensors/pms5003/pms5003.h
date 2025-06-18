#pragma once
#include <defines.h>
#include <comm/uart.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define hkPMS5003_JSON_BUFFER   64

enum {
    PMS5003_INIT,
    PMS5003_READ_INIT,
    PMS5003_START_BYTE_1  = 0x42,
    PMS5003_START_BYTE_2  = 0x4D,
    PMS5003_PACKET_LENGTH = 32
};

typedef struct PMS5003_Config_t {
    u8*    rawData;
    size_t length;
    u8     status;
} PMS5003_Config_t;

// I know it's redundant, I just want to have some universal pattern around my code
typedef char* (*json)(const void* self);
typedef struct PMS5003_DataPacket_t {
    u16  pm1;
    u16  pm2_5;
    u16  pm10;
    json jsonify;
} PMS5003_DataPacket_t;


void PMS5003_ProcessData(PMS5003_Config_t* config, PMS5003_DataPacket_t* data);
void PMS5003_ProcessDataHumidCompensation(PMS5003_Config_t* config, PMS5003_DataPacket_t* data, void* humidityData);

b8 PMS5003_Read(UART_Config_t* uart, PMS5003_Config_t* config);


static char* PMS5003_Jsonify(const void* self) {   
    const PMS5003_DataPacket_t* data = (PMS5003_DataPacket_t*)self;
    
    const char* json = "{"
        "\"sensor\": \"pms5003\","
        "\"pm1\": %u,"
        "\"pm2_5\": %u,"
        "\"pm10\": %u"
    "}";
    
    static char buffer[hkPMS5003_JSON_BUFFER];
    u32 requiredLength = snprintf(buffer, sizeof(buffer), json, data->pm1, data->pm2_5, data->pm10);
    if(requiredLength > sizeof(buffer)) {
        HERROR("[PMS5003] Buffer size is to small to send this data packet!\n");
        return NULL;
    }

    return buffer;
}