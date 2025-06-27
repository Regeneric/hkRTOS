#pragma once
#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include <event_groups.h>

#include <defines.h>
#include <stdio.h>
#include <pico/sync.h>

#include <comm/i2c.h>

#define hkBM280_JSON_BUFFER 128

#define hkBME280_BIT_ENABLED (1 << hkBME280_USE_SENSOR)


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
    BME280_REG_CONFIG    = 0xF5,

    BME280_MODE_SLEEP    = 0x00,
    BME280_MODE_FORCED   = 0x01,
    BME280_MODE_NORMAL   = 0x03,

    BME280_HUMID_OVERSAMPLING_SKIPPED = 0x00,
    BME280_HUMID_OVERSAMPLING_X1      = 0x01,
    BME280_HUMID_OVERSAMPLING_X2      = 0x02,
    BME280_HUMID_OVERSAMPLING_X4      = 0x03,
    BME280_HUMID_OVERSAMPLING_X8      = 0x04,
    BME280_HUMID_OVERSAMPLING_X16     = 0x05,

    BME280_OVERSAMPLING_SKIPPED       = 0x00,
    BME280_OVERSAMPLING_X1            = 0x01,
    BME280_OVERSAMPLING_X2            = 0x02,
    BME280_OVERSAMPLING_X4            = 0x03,
    BME280_OVERSAMPLING_X8            = 0x04,
    BME280_OVERSAMPLING_X16           = 0x05,

    BME280_FILTER_COEFF_OFF           = (0x00 << 2),
    BME280_FILTER_COEFF_2             = (0x01 << 2),
    BME280_FILTER_COEFF_4             = (0x02 << 2),
    BME280_FILTER_COEFF_8             = (0x03 << 2),
    BME280_FILTER_COEFF_16            = (0x04 << 2),

    BME280_STANDBY_TIME_0_5_MS        = (0x00),
    BME280_STANDBY_TIME_62_5_MS       = (0x01),
    BME280_STANDBY_TIME_125_MS        = (0x02),
    BME280_STANDBY_TIME_250_MS        = (0x03),
    BME280_STANDBY_TIME_500_MS        = (0x04),
    BME280_STANDBY_TIME_1000_MS       = (0x05),
    BME280_STANDBY_TIME_10_MS         = (0x06),
    BME280_STANDBY_TIME_20_MS         = (0x07)
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