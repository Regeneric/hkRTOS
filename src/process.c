// FreeRTOS
#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <event_groups.h>

// hkRTOS
#include <defines.h>

#include <core/logger.h>
#include <timing/ds1307/ds1307.h>

#include <sensors/sensors.h>
#include <sensors/dht11_22/dht11_22.h>
#include <sensors/ds18b20/ds18b20.h>
#include <sensors/pms5003/pms5003.h>
#include <sensors/sgp30/sgp30.h>
#include <sensors/bme280/bme280.h>

static f32 hkWeightedAverage(Average_DataPacket_t* data, size_t len) {
    HTRACE("process.c -> s:hkWeightedAverage(Average_DataPacket_t*, size_t):f32");

    // Temperature and humidity
    // BME280  - weight 10
    // DS18B20 - weight 10
    // AHT21   - weight 10
    // AHT20/DHT20 - weight 9
    // DHT22  - weight 7
    // BMP180 - weight 3
    // DHT11  - weight 2   

    // Pressure
    // BME280 - weight 8
    // BMP180 - weight 2

    // CO2
    // ENS160 - weight 9
    // SGP30  - weight 4

    if(data == NULL) {
        HERROR("hkWeightedAverage(): No data to process!");
        return -1.0f;
    }
    if(len < 1)  {
        HERROR("hkWeightedAverage(): Data length is less than 1!");
        return data[0].value;
    }
    if(len == 1) {
        HWARN("hkWeightedAverage(): Data length is 1, weighted average will not be calculated.");
        return data[0].value;
    }

    f32 sum = 0.0f;
    u16 divider = 0.0f;

    for(size_t i = 0; i < len; ++i) {
        sum     += (data[i].value * data[i].weight);
        divider += data[i].weight;
    }

    return sum/divider;
}


extern EventGroupHandle_t xSystemStateEventGroup;
void vDataProcessTask(void* pvParameters) {
    HTRACE("collect.c -> RTOS:vDataProcessTask(void*):void");
    
    xEventGroupWaitBits(
        xSystemStateEventGroup,      // The event group to wait on
        BIT_MODE_NORMAL_OPERATION,   // The bit to wait for
        pdFALSE,                     // Don't clear the bit on exit
        pdFALSE,                     // Wait for ALL bits (we only have one)
        portMAX_DELAY                // Wait forever
    );


    // // ************************************************************************
    // // = DS1307 ===
    // // ------------------------------------------------------------------------
    // // TODO: set datetime from web interface
    // I2C_Config_t* hkI2Cx = (I2C_Config_t*)pvParameters;
    // static DS1307_Config_t hkDS1307_0 = {
    //     .address   = hkDS1307_ADDRESS,
    //     .reset     = false,
    //     .utcOffset = 2
    // };

    // static DS1307_DataPacket_t hkDS1307_0_Data = {
    //     .year    = 2025,
    //     .month   = 7,
    //     .day     = 4,
    //     .dayName = 6,   // 1 is Sunday
    //     .hour    = 0,
    //     .min     = 20,
    //     .sec     = 0
    // }; DS1307_Init(hkI2Cx, &hkDS1307_0, &hkDS1307_0_Data);
    // // ------------------------------------------------------------------------


    Sensors_DataPacket_t hkSensors_DataPacket = {0};
    UBaseType_t coreID = portGET_CORE_ID();

    while(FOREVER) {
        HTRACE("vDataProcessTask(): Running on core {%d}", (u16)coreID);

        if(xQueueReceive(xSnapshotQueue, &hkSensors_DataPacket, portMAX_DELAY) == pdPASS) {
            Average_DataPacket_t temparatureAverage[] = {
                // VALUE, WEIGHT
                {.value = hkSensors_DataPacket.bme280_0.temperature , .weight = 10},
                {.value = hkSensors_DataPacket.bme280_1.temperature , .weight = 10},
                {.value = hkSensors_DataPacket.ds18b20_0.temperature, .weight = 10},
                {.value = hkSensors_DataPacket.dht_0.temperature    , .weight =  9},        // DHT20
                {.value = hkSensors_DataPacket.dht_1.temperature    , .weight =  9},        // DHT20
                {.value = hkSensors_DataPacket.dht_2.temperature    , .weight =  2},        // DHT11
                {.value = hkSensors_DataPacket.dht_3.temperature    , .weight =  7},        // DHT22
            }; size_t temparatureAverageLength = sizeof(temparatureAverage) / sizeof(temparatureAverage[0]);

            Average_DataPacket_t dewPointAverage[] = {
                // VALUE, WEIGHT
                {.value = hkSensors_DataPacket.bme280_0.dewPoint, .weight = 10},
                {.value = hkSensors_DataPacket.bme280_1.dewPoint, .weight = 10},
                {.value = hkSensors_DataPacket.dht_0.dewPoint   , .weight =  9},            // DHT20
                {.value = hkSensors_DataPacket.dht_1.dewPoint   , .weight =  9},            // DHT20
                {.value = hkSensors_DataPacket.dht_2.dewPoint   , .weight =  2},            // DHT11
                {.value = hkSensors_DataPacket.dht_3.dewPoint   , .weight =  7},            // DHT22
            }; size_t dewPointAverageLength = sizeof(dewPointAverage) / sizeof(dewPointAverage[0]);

            Average_DataPacket_t relativeHumidityAverage[] = {
                // VALUE, WEIGHT
                {.value = hkSensors_DataPacket.bme280_0.humidity, .weight = 10},
                {.value = hkSensors_DataPacket.bme280_1.humidity, .weight = 10},
                {.value = hkSensors_DataPacket.dht_0.humidity   , .weight =  9},            // DHT20
                {.value = hkSensors_DataPacket.dht_1.humidity   , .weight =  9},            // DHT20
                {.value = hkSensors_DataPacket.dht_2.humidity   , .weight =  2},            // DHT11
                {.value = hkSensors_DataPacket.dht_3.humidity   , .weight =  7},            // DHT22
            }; size_t relativeHumidityAverageLength = sizeof(relativeHumidityAverage) / sizeof(relativeHumidityAverage[0]);

            Average_DataPacket_t absoluteHumidityAverage[] = {
                // VALUE, WEIGHT
                {.value = hkSensors_DataPacket.bme280_0.absoluteHumidity, .weight = 10},
                {.value = hkSensors_DataPacket.bme280_1.absoluteHumidity, .weight = 10},
                {.value = hkSensors_DataPacket.dht_0.absoluteHumidity   , .weight =  9},    // DHT20
                {.value = hkSensors_DataPacket.dht_1.absoluteHumidity   , .weight =  9},    // DHT20
                {.value = hkSensors_DataPacket.dht_2.absoluteHumidity   , .weight =  2},    // DHT11
                {.value = hkSensors_DataPacket.dht_3.absoluteHumidity   , .weight =  7},    // DHT22
            }; size_t absoluteHumidityAverageLength = sizeof(absoluteHumidityAverage) / sizeof(absoluteHumidityAverage[0]);

            Average_DataPacket_t pressureAverage[] = {
                // VALUE, WEIGHT
                {.value = hkSensors_DataPacket.bme280_0.pressure, .weight = 8},
                {.value = hkSensors_DataPacket.bme280_1.pressure, .weight = 8},
                // {.value = hkSensors_DataPacket.bmp180_0.pressure, .weight = 2},
            }; size_t pressureAverageLength = sizeof(pressureAverage) / sizeof(pressureAverage[0]);


            // - BME280 Average --------------
            BME280_DataPacket_t BME280_Sensors[] = {hkSensors_DataPacket.bme280_0, hkSensors_DataPacket.bme280_1};
            hkSensors_DataPacket.bme280_avg = BME280_AverageData(BME280_Sensors, 2);
            // -------------------------------

            // - DHT20 Average ---------------
            DHT_DataPacket_t DHT20_Sensors[] = {hkSensors_DataPacket.dht_0, hkSensors_DataPacket.dht_1};
            hkSensors_DataPacket.dht_avg = DHT_AverageData(DHT20_Sensors, 2);
            // -------------------------------
        
            // - Average Temperature ---------
            hkSensors_DataPacket.temperature = hkWeightedAverage(temparatureAverage, temparatureAverageLength);
            // -------------------------------

            // - Average Dew Point -----------
            hkSensors_DataPacket.dewPoint = hkWeightedAverage(dewPointAverage, temparatureAverageLength);
            // -------------------------------

            // - Average Relative Humidity ---
            hkSensors_DataPacket.relativeHumidity = hkWeightedAverage(relativeHumidityAverage, relativeHumidityAverageLength);
            // -------------------------------

            // - Average Absolute Humidity ---
            hkSensors_DataPacket.absoluteHumidity = hkWeightedAverage(absoluteHumidityAverage, relativeHumidityAverageLength);
            // -------------------------------
        
            // - Average Pressure ------------
            hkSensors_DataPacket.pressure = hkWeightedAverage(pressureAverage, pressureAverageLength);
            // -------------------------------


            // - Timestamp -------------------
            // hkSensors_DataPacket.timestamp = DS1307_ReadDateTime(hkI2Cx, &hkDS1307_0, &hkDS1307_0_Data);
            // -------------------------------

            xQueueSend(xDisplayQueue, &hkSensors_DataPacket, 0);
            xQueueSend(xMQTTQueue, &hkSensors_DataPacket, 0);
        }
    }
}