// FreeRTOS
#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>

// hkRTOS
#include <defines.h>

#include <core/logger.h>

#include <sensors/sensors.h>
#include <sensors/dht11_22/dht11_22.h>
#include <sensors/ds18b20/ds18b20.h>
#include <sensors/pms5003/pms5003.h>
#include <sensors/sgp30/sgp30.h>
#include <sensors/bme280/bme280.h>


extern EventGroupHandle_t xSystemStateEventGroup;
void vDataDisplayTask(void* pvParameters) {
    HTRACE("display.c -> RTOS:vDataDisplayTask(void*):void");

    xEventGroupWaitBits(
        xSystemStateEventGroup,      // The event group to wait on
        BIT_MODE_NORMAL_OPERATION,   // The bit to wait for
        pdFALSE,                     // Don't clear the bit on exit
        pdFALSE,                     // Wait for ALL bits (we only have one)
        portMAX_DELAY                // Wait forever
    );


    Sensors_DataPacket_t hkSensors_DataPacket = {0};
    UBaseType_t coreID = portGET_CORE_ID();

    while(FOREVER) {
        HTRACE("vDataDisplayTask(): Running on core {%d}", (u16)coreID);

        if(xQueueReceive(xDisplayQueue, &hkSensors_DataPacket, portMAX_DELAY) == pdPASS) {
            // - BME280 ----------------------
            HDEBUG("(BME280_0) TEMPERATURE   : %.2f *C"  , hkSensors_DataPacket.bme280_0.temperature);
            HDEBUG("(BME280_0) DEW POINT     : %.2f *C"  , hkSensors_DataPacket.bme280_0.dewPoint);
            HDEBUG("(BME280_0) HUMIDITY      : %.2f %%RH", hkSensors_DataPacket.bme280_0.humidity);
            HDEBUG("(BME280_0) ABS HUMIDITY  : %.2f %%AH", hkSensors_DataPacket.bme280_0.absoluteHumidity);
            HDEBUG("(BME280_0) PRESSURE      : %.1f hPA" , hkSensors_DataPacket.bme280_0.pressure);
            
            HDEBUG("(BME280_1) TEMPERATURE   : %.2f *C"  , hkSensors_DataPacket.bme280_1.temperature);
            HDEBUG("(BME280_1) DEW POINT     : %.2f *C"  , hkSensors_DataPacket.bme280_1.dewPoint);
            HDEBUG("(BME280_1) HUMIDITY      : %.2f %%RH", hkSensors_DataPacket.bme280_1.humidity);
            HDEBUG("(BME280_1) ABS HUMIDITY  : %.2f %%AH", hkSensors_DataPacket.bme280_1.absoluteHumidity);
            HDEBUG("(BME280_1) PRESSURE      : %.1f hPA" , hkSensors_DataPacket.bme280_1.pressure);

            HDEBUG("(BME__AVG) TEMPERATURE   : %.2f *C"  , hkSensors_DataPacket.bme280_avg.temperature);
            HDEBUG("(BME__AVG) DEW POINT     : %.2f *C"  , hkSensors_DataPacket.bme280_avg.dewPoint);
            HDEBUG("(BME__AVG) HUMIDITY      : %.2f %%RH", hkSensors_DataPacket.bme280_avg.humidity);
            HDEBUG("(BME__AVG) ABS HUMIDITY  : %.2f %%AH", hkSensors_DataPacket.bme280_avg.absoluteHumidity);
            HDEBUG("(BME__AVG) PRESSURE      : %.1f hPA" , hkSensors_DataPacket.bme280_avg.pressure);
            // -------------------------------

            // - DHT20 -----------------------
            HDEBUG("(DHT20__0) TEMPERATURE   : %.2f *C"  , hkSensors_DataPacket.dht_0.temperature);
            HDEBUG("(DHT20__0) DEW POINT     : %.2f *C"  , hkSensors_DataPacket.dht_0.dewPoint);
            HDEBUG("(DHT20__0) HUMIDITY      : %.2f %%RH", hkSensors_DataPacket.dht_0.humidity);
            HDEBUG("(DHT20__0) ABS HUMIDITY  : %.2f %%AH", hkSensors_DataPacket.dht_0.absoluteHumidity);

            HDEBUG("(DHT20__1) TEMPERATURE   : %.2f *C"  , hkSensors_DataPacket.dht_1.temperature);
            HDEBUG("(DHT20__1) DEW POINT     : %.2f *C"  , hkSensors_DataPacket.dht_1.dewPoint);
            HDEBUG("(DHT20__1) HUMIDITY      : %.2f %%RH", hkSensors_DataPacket.dht_1.humidity);
            HDEBUG("(DHT20__1) ABS HUMIDITY  : %.2f %%AH", hkSensors_DataPacket.dht_1.absoluteHumidity);

            HDEBUG("(DHT__AVG) TEMPERATURE   : %.2f *C"  , hkSensors_DataPacket.dht_avg.temperature);
            HDEBUG("(DHT__AVG) DEW POINT     : %.2f *C"  , hkSensors_DataPacket.dht_avg.dewPoint);
            HDEBUG("(DHT__AVG) HUMIDITY      : %.2f %%RH", hkSensors_DataPacket.dht_avg.humidity);
            HDEBUG("(DHT__AVG) ABS HUMIDITY  : %.2f %%AH", hkSensors_DataPacket.dht_avg.absoluteHumidity);
            // -------------------------------

            // - DHT11 -----------------------
            HDEBUG("(DHT11__0) TEMPERATURE   : %.2f *C"  , hkSensors_DataPacket.dht_2.temperature);
            HDEBUG("(DHT11__0) DEW POINT     : %.2f *C"  , hkSensors_DataPacket.dht_2.dewPoint);
            HDEBUG("(DHT11__0) HUMIDITY      : %.2f %%RH", hkSensors_DataPacket.dht_2.humidity);
            HDEBUG("(DHT11__0) ABS HUMIDITY  : %.2f %%AH", hkSensors_DataPacket.dht_2.absoluteHumidity);
            // -------------------------------

            // - DHT22 -----------------------
            HDEBUG("(DHT22__0) TEMPERATURE   : %.2f *C"  , hkSensors_DataPacket.dht_3.temperature);
            HDEBUG("(DHT22__0) DEW POINT     : %.2f *C"  , hkSensors_DataPacket.dht_3.dewPoint);
            HDEBUG("(DHT22__0) HUMIDITY      : %.2f %%RH", hkSensors_DataPacket.dht_3.humidity);
            HDEBUG("(DHT22__0) ABS HUMIDITY  : %.2f %%AH", hkSensors_DataPacket.dht_3.absoluteHumidity);
            // -------------------------------

            // - SGP30 -----------------------
            HDEBUG("(SGP30__0) eCO2          : %d ppm", hkSensors_DataPacket.sgp30_0.eco2);
            HDEBUG("(SGP30__0) TVOC          : %d ppb", hkSensors_DataPacket.sgp30_0.tvoc);
            // -------------------------------

            // - DS18B20 ---------------------
            HDEBUG("(DS18B20 ) TEMPERATURE   : %.2f *C", hkSensors_DataPacket.ds18b20_0.temperature);
            // -------------------------------

            // - PMS5003 ---------------------
            HDEBUG("(PMS_5003) PM1           : %d ug/m3", hkSensors_DataPacket.pms5003_0.pm1);
            HDEBUG("(PMS_5003) PM2.5         : %d ug/m3", hkSensors_DataPacket.pms5003_0.pm2_5);
            HDEBUG("(PMS_5003) PM10          : %d ug/m3", hkSensors_DataPacket.pms5003_0.pm10);
            // -------------------------------


            // - Average Temperature ---------
            HDEBUG("AVERAGE TEMPERATURE      : %.2f *C", hkSensors_DataPacket.temperature);
            // -------------------------------

            // - Average Dew Point -----------
            HDEBUG("AVERAGE DEW POINT        : %.2f *C", hkSensors_DataPacket.dewPoint);
            // -------------------------------

            // - Average Relative Humidity ---
            HDEBUG("AVERAGE RELATIVE HUMIDITY: %.2f %%RH", hkSensors_DataPacket.relativeHumidity);
            // -------------------------------

            // - Average Absolute Humidity ---
            HDEBUG("AVERAGE ABSOLUTE HUMIDITY: %.2f %%AH", hkSensors_DataPacket.absoluteHumidity);
            // -------------------------------
        
            // - Average Pressure ------------
            HDEBUG("AVERAGE PRESSURE         : %.2f hPA", hkSensors_DataPacket.pressure);
            // -------------------------------
        }
    }
}