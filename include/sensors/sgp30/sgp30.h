#pragma once
#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include <event_groups.h>

#include <defines.h>
#include <comm/i2c.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define hkSGP30_JSON_BUFFER   64

enum {
    SGP30_READ_INIT,
    SGP30_READ_READY,
    SGP30_READ_BASELINE_INIT,
    SGP30_READ_BASELINE_READY,
    SGP30_INIT          = 0x03,
    SGP30_RESET         = 0x06,
    SGP30_START_MEASURE = 0x08,
    SGP30_GET_BASELINE  = 0x15,
    SGP30_SET_BASELINE  = 0x1E,
    SGP30_SET_HUMIDITY  = 0x61,
    SGP30_READY         = 0x2137,
    SGP30_BASELINE_READY   = 0x0420,
    SGP30_READ_IN_PROGRESS = 0x696
};

typedef struct SGP30_Config_t {
    u8     address;
    vu16   status;
    u8*    rawData;
    size_t length;
    u16    eco2Baseline;
    u16    tvocBaseline;
    QueueHandle_t queue;
} SGP30_Config_t;

typedef struct SGP30_DataPacket_t {
    u16  eco2;
    u16  tvoc;
    json jsonify;
} SGP30_DataPacket_t;

typedef struct SGP30_TaskParams_t {
    I2C_Config_t*       i2c;
    SGP30_Config_t*     sgp30;
    SGP30_DataPacket_t* data;
    void* humidSensor;
} SGP30_TaskParams_t;


void vSGP30_Task(void* pvParameters);

i32  SGP30_Init(I2C_Config_t* i2c, SGP30_Config_t* config);
void SGP30_InitRead(I2C_Config_t* i2c, SGP30_Config_t* config);
void SGP30_InitReadHumidCompensation(I2C_Config_t* i2c, SGP30_Config_t* config, void* sensorData);
i32  SGP30_Read(I2C_Config_t* i2c, SGP30_Config_t* config);
i32  SGP30_ReadBlocking(I2C_Config_t* i2c, SGP30_Config_t* config);
i32  SGP30_ReadBlockingHumidComp(I2C_Config_t* i2c, SGP30_Config_t* config, void* sensorData);
void SGP30_ProcessData(SGP30_Config_t* config, SGP30_DataPacket_t* data);


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