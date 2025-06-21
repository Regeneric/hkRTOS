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

void vDisplayTask(void* pvParameters) {
    while(1) {
        QueueSetMemberHandle_t xActivatedMember;
        xActivatedMember = xQueueSelectFromSet(xSensorQueueSet, portMAX_DELAY);

        // - BME280 ---
        if(xActivatedMember == xBME280_0_DataQueue) {
            BME280_DataPacket_t hkBME280_0_Data;
            xQueueReceive(xBME280_0_DataQueue, &hkBME280_0_Data, 0);
            HDEBUG("(BME280_0) TEMPERATURE : %.2f *C"  , hkBME280_0_Data.temperature);
            HDEBUG("(BME280_0) DEW POINT   : %.2f *C"  , hkBME280_0_Data.dewPoint);
            HDEBUG("(BME280_0) HUMIDITY    : %.2f %%RH", hkBME280_0_Data.humidity);
            HDEBUG("(BME280_0) ABS HUMIDITY: %.2f %%AH", hkBME280_0_Data.absoluteHumidity);
            HDEBUG("(BME280_0) PRESSURE    : %.1f hPA" , hkBME280_0_Data.pressure);

        }   
        if(xActivatedMember == xBME280_1_DataQueue) {
            BME280_DataPacket_t hkBME280_1_Data;
            xQueueReceive(xBME280_1_DataQueue, &hkBME280_1_Data, 0);
            HDEBUG("(BME280_1) TEMPERATURE : %.2f *C"  , hkBME280_1_Data.temperature);
            HDEBUG("(BME280_1) DEW POINT   : %.2f *C"  , hkBME280_1_Data.dewPoint);
            HDEBUG("(BME280_1) HUMIDITY    : %.2f %%RH", hkBME280_1_Data.humidity);
            HDEBUG("(BME280_1) ABS HUMIDITY: %.2f %%AH", hkBME280_1_Data.absoluteHumidity);
            HDEBUG("(BME280_1) PRESSURE    : %.1f hPA" , hkBME280_1_Data.pressure);
        }
        // ------------

        // - SGP30 ---
        if(xActivatedMember == xSGP30_0_DataQueue) {
            SGP30_DataPacket_t hkSGP30_0_Data;
            xQueueReceive(xSGP30_0_DataQueue, &hkSGP30_0_Data, 0);
            HDEBUG("(SGP30__0) eCO2        : %d ppm", hkSGP30_0_Data.eco2);
            HDEBUG("(SGP30__0) TVOC        : %d ppb", hkSGP30_0_Data.tvoc);
        } 
        // ------------

        // - DS18B20 ---
        if(xActivatedMember == xDS18B20_0_DataQueue) {
            DS18B20_DataPacket_t hkDS18B20_0_Data;
            xQueueReceive(xDS18B20_0_DataQueue, &hkDS18B20_0_Data, 0);
            HDEBUG("(DS18B20 ) TEMPERATURE : %.2f *C", hkDS18B20_0_Data.temperature);
        } 
        // ------------

        // - PMS5003 ---
        if(xActivatedMember == xPMS5003_0_DataQueue) {
            PMS5003_DataPacket_t hkPMS5003_0_Data;
            xQueueReceive(xPMS5003_0_DataQueue, &hkPMS5003_0_Data, 0);
            HDEBUG("(PMS5003 ) PM1         : %d ug/m3", hkPMS5003_0_Data.pm1);
            HDEBUG("(PMS5003 ) PM2.5       : %d ug/m3", hkPMS5003_0_Data.pm2_5);
            HDEBUG("(PMS5003 ) PM10        : %d ug/m3", hkPMS5003_0_Data.pm10);
        } 
        // ------------
    }
}