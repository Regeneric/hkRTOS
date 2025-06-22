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
extern QueueHandle_t xDHT11_0_DataQueue;
extern QueueHandle_t xDHT22_0_DataQueue;
extern QueueHandle_t xBMP180_0_DataQueue;

extern QueueHandle_t xSnapshotQueue;
extern QueueHandle_t xDisplayQueue;

typedef struct Average_DataPacket_t {
    f32 value;
    u8  weight;
} Average_DataPacket_t;

typedef struct Sensors_DataPacket_t {
    DHT_DataPacket_t     dht_0;     // DHT20
    DHT_DataPacket_t     dht_1;     // DHT20
    DHT_DataPacket_t     dht_2;     // DHT11
    DHT_DataPacket_t     dht_3;     // DHT22
    DHT_DataPacket_t     dht_avg;

    BME280_DataPacket_t  bme280_0;
    BME280_DataPacket_t  bme280_1;
    BME280_DataPacket_t  bme280_avg;

    DS18B20_DataPacket_t ds18b20_0;
    PMS5003_DataPacket_t pms5003_0;
    SGP30_DataPacket_t   sgp30_0; 
    // BMP180_DataPacket_t  bmp180_0;

    // Averages from all sensors
    f32 temperature;
    f32 dewPoint;
    f32 relativeHumidity;
    f32 absoluteHumidity;
    f32 pressure;
} Sensors_DataPacket_t;