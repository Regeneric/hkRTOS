#pragma once
#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>

#include <defines.h>

#include <stdio.h>
#include <math.h>

#include <core/logger.h>
#include <sensors/sensors.h>
#include <sensors/dht11_22/dht11_22.h>


typedef struct DHT20_Config_t {
    u8*    rawData;
    size_t length;
    u8     address;
    QueueHandle_t queue;
} DHT20_Config_t;

typedef struct DHT20_TaskParams_t {
    I2C_Config_t*     i2c;
    DHT20_Config_t*   dht20;
    DHT_DataPacket_t* data;
} DHT20_TaskParams_t;


i32  DHT20_Init(I2C_Config_t* i2c, DHT20_Config_t* config);
i32  DHT20_InitRead(I2C_Config_t* i2c, DHT20_Config_t* config);
i32  DHT20_Read(I2C_Config_t* i2c, DHT20_Config_t* config);
void DHT20_ProcessData(DHT20_Config_t* config, DHT_DataPacket_t* data);

void vDHT20_Task(void* pvParameters);