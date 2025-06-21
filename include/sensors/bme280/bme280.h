#pragma once
#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>

#include <defines.h>
#include <stdio.h>
#include <pico/sync.h>

#include <comm/i2c.h>

#define hkBM280_JSON_BUFFER 128

enum {
    BME280_INIT,
    BME280_READ_READY,
    BME280_READ_SUCCESS,
    BME280_READ_IN_PROGRESS,
    BME280_READ_BAD_CHECKSUM,
    BME280_REG_CALIB_00  = 0x88,
    BME280_REG_CALIB_26  = 0xE1,
    BME280_REG_CTRL_HUM  = 0xF2,
    BME280_REG_CTRL_MEAS = 0xF4,
    BME280_REG_CONFIG    = 0xF5
};

// It's all defined by the datasheet
typedef struct BME280_CalibrationParams_t {
    u16 dig_T1;
    i16 dig_T2;
    i16 dig_T3;

    u16 dig_P1;
    i16 dig_P2;
    i16 dig_P3;
    i16 dig_P4;
    i16 dig_P5;
    i16 dig_P6;
    i16 dig_P7;
    i16 dig_P8;
    i16 dig_P9;

    u8  dig_H1;
    i16 dig_H2;
    u8  dig_H3;
    i16 dig_H4;
    i16 dig_H5;
    i8  dig_H6;
} BME280_CalibrationParams_t;

typedef struct BME280_Config_t {
    u8*    rawData;
    size_t length;
    u8     address;
    vu16   status;
    u8     humiditySampling;
    u8     iirCoefficient;
    u8     tempAndPressureMode;
    QueueHandle_t queue;
    BME280_CalibrationParams_t params;
} BME280_Config_t;

// I know it's redundant, I just want to have some universal pattern around my code
typedef char* (*json)(const void* self);
typedef struct BME280_DataPacket_t {
    f32  pressure;
    f32  temperature;
    f32  dewPoint;
    f32  humidity;
    f32  absoluteHumidity;
    json jsonify;
} BME280_DataPacket_t;

typedef struct BME280_TaskParams_t {
    I2C_Config_t*        i2c;
    BME280_Config_t*     bme280;
    BME280_DataPacket_t* data;
} BME280_TaskParams_t;


void vBME280_Task(void* pvParameters);

i32 BME280_Init(I2C_Config_t* i2c, BME280_Config_t* config);
i32 BME280_InitRead(I2C_Config_t* i2c, BME280_Config_t* config);
i32 BME280_Read(I2C_Config_t* i2c, BME280_Config_t* config);
i32 BME280_WriteCommand(I2C_Config_t* i2c, BME280_Config_t* config, u8 command);

void BME280_ProcessData(BME280_Config_t* config, BME280_DataPacket_t* data);
BME280_DataPacket_t BME280_AverageData(BME280_DataPacket_t* data, size_t len);


static char* BME280_Jsonify(const void* self) {   
    const BME280_DataPacket_t* data = (BME280_DataPacket_t*)self;
    
    const char* json = "{"
        "\"sensor\": \"bme280\","
        "\"pressure\": %.2f,"
        "\"temperature\": %.2f,"
        "\"humidity\": %.2f,"
        "\"absoluteHumidity\": %.2f"
    "}";
    
    static char buffer[hkBM280_JSON_BUFFER];
    u32 requiredLength = snprintf(buffer, sizeof(buffer), json, data->pressure, data->temperature, data->humidity);
    if(requiredLength > sizeof(buffer)) {
        HERROR("BME280_Jsonify(): [BME280] Buffer size is to small to send this data packet!\n");
        return NULL;
    }

    return buffer;
}