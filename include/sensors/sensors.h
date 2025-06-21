#pragma once

#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>

#include <defines.h>

#include <sensors/dht11_22/dht11_22.h>
#include <sensors/ds18b20/ds18b20.h>
#include <sensors/pms5003/pms5003.h>
#include <sensors/sgp30/sgp30.h>
#include <sensors/bme280/bme280.h>


extern QueueSetHandle_t xSensorQueueSet;
extern QueueHandle_t xBME280_0_DataQueue;
extern QueueHandle_t xBME280_1_DataQueue;
extern QueueHandle_t xSGP30_0_DataQueue;
extern QueueHandle_t xPMS5003_0_DataQueue;
extern QueueHandle_t xDS18B20_0_DataQueue;
extern QueueHandle_t xDHT20_0_DataQueue;
extern QueueHandle_t xDHT20_1_DataQueue;
extern QueueHandle_t xBMP180_0_DataQueue;


typedef struct Average_DataPacket_t {
    f32 value;
    u8  weight;
} Average_DataPacket_t;

typedef struct Sensors_DataPacket_t {
    DHT_DataPacket_t     dht;
    DS18B20_DataPacket_t ds18b20;
    PMS5003_DataPacket_t pms5003;
    SGP30_DataPacket_t   sgp30;
    BME280_DataPacket_t  bme280; 
    
    f32 temperature;
    f32 humidity;
    f32 dewPoint;
    f32 absHumidity;
} Sensors_DataPacket_t;

static f32 hkWeightedAverage(Average_DataPacket_t* data, size_t len) {
    // BME280  - weight 10
    // DS18B20 - weight 10
    // AHT20/DHT20 - weight 9
    // DHT22 - weight 7
    // DHT11 - weight 2   

    if(len < 1) return -2.137f;

    f32 sum = 0.0f;
    u16 divider = 0.0f;

    for(size_t i = 0; i < len; ++i) {
        sum     += (data[i].value * data[i].weight);
        divider += data[i].weight;
    }

    return sum/divider;
}