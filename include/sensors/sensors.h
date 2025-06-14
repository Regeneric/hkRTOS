#pragma once
#include <defines.h>

#include <sensors/dht11/dht11.h>
#include <sensors/ds18b20/ds18b20.h>
#include <sensors/pms5003/pms5003.h>
#include <sensors/sgp30/sgp30.h>
#include <sensors/bme280/bme280.h>

typedef struct Sensors_DataPacket_t {
    DHT_DataPacket_t     dht;
    DS18B20_DataPacket_t ds18b20;
    PMS5003_DataPacket_t pms5003;
    SGP30_DataPacket_t   sgp30;
    BME280_DataPacket_t  bme280; 
} Sensors_DataPacket_t;