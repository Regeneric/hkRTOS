#pragma once
#include <defines.h>
#include <comm/i2c.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define hkSGP30_JSON_BUFFER   64

enum {
    hkSGP_INIT          = 0x03,
    hkSGP_RESET         = 0x06,
    hkSGP_START_MEASURE = 0x08,
    hkSGP_GET_BASELINE  = 0x15,
    hkSGP_SET_BASELINE  = 0x1E,
    hkSGP_SET_HUMIDITY  = 0x61,
    hkSGP_READY         = 0x2137,
    hkSGP_BASELINE_READY   = 0x0420,
    hkSGP_READ_IN_PROGRESS = 0x696
};

typedef struct SGP30_Config_t {
    vu16   status;
    u8*    rawData;
    size_t length;
    u16    eco2Baseline;
    u16    tvocBaseline;
} SGP30_Config_t;

// I know it's redundant, I just want to have some universal pattern around my code
typedef char* (*json)(const void* self);
typedef struct SGP30_DataPacket_t {
    u16  eco2;
    u16  tvoc;
    json jsonify;
} SGP30_DataPacket_t;


u32  SGP30_WriteCommand(I2C_Config_t* i2c, SGP30_Config_t* config, u8 command);
u32  SGP30_Init(I2C_Config_t* i2c, SGP30_Config_t* config);
void SGP30_ProcessData(SGP30_Config_t* config, SGP30_DataPacket_t* data);
void SGP30_InitRead(I2C_Config_t* i2c, SGP30_Config_t* config);
void SGP30_Read(I2C_Config_t* i2c, SGP30_Config_t* config);

void SGP30_InitGetBaseline(I2C_Config_t* i2c, SGP30_Config_t* config);
b8   SGP30_GetBaseline(I2C_Config_t* i2c, SGP30_Config_t* config);
void SGP30_ProcessBaseline(SGP30_Config_t* config);


static char* SGP30_Jsonify(const void* self) {   
    const SGP30_DataPacket_t* data = (SGP30_DataPacket_t*)self;
    
    const char* json = "{"
        "\"sensor\": \"sgp30\","
        "\"co2\": %u,"
        "\"tvoc\": %u"
    "}";
    
    static char buffer[hkSGP30_JSON_BUFFER];
    u32 requiredLength = snprintf(buffer, sizeof(buffer), json, data->eco2, data->tvoc);
    if(requiredLength > sizeof(buffer)) {
        HERROR("[SGP30] Buffer size is to small to send this data packet!\n");
        return NULL;
    }

    return buffer;
}