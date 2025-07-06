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
void vDataCollectTask(void* pvParameters) {
    HTRACE("collect.c -> RTOS:vDataCollectTask(void*):void");

    xEventGroupWaitBits(
        xSystemStateEventGroup,      // The event group to wait on
        BIT_MODE_NORMAL_OPERATION,   // The bit to wait for
        pdFALSE,                     // Don't clear the bit on exit
        pdFALSE,                     // Wait for ALL bits (we only have one)
        portMAX_DELAY                // Wait forever
    );


    Sensors_DataPacket_t hkSensors_DataPacket = {0};
    UBaseType_t coreID = portGET_CORE_ID();

    // This timer will trigger sending the Sensors_DataPacket every 2 seconds
    TickType_t xLastSendTime = xTaskGetTickCount();
    const TickType_t xSendInterval = pdMS_TO_TICKS(2000);

    while(FOREVER) {
        HTRACE("vDataCollectTask(): Running on core {%d}", (u16)coreID);
        
        QueueSetMemberHandle_t xActivatedMember;
        xActivatedMember = xQueueSelectFromSet(xSensorQueueSet, xSendInterval);

        if(xActivatedMember != NULL) {
            // - BME280 ----
            if(xActivatedMember == xBME280_0_DataQueue) {
                xQueueReceive(xActivatedMember, &hkSensors_DataPacket.bme280_0, 0);
            }
            if(xActivatedMember == xBME280_1_DataQueue) {
                xQueueReceive(xActivatedMember, &hkSensors_DataPacket.bme280_1, 0);
            }
            // -------------

            // - DHT20 -----
            if(xActivatedMember == xDHT20_0_DataQueue) {
                xQueueReceive(xActivatedMember, &hkSensors_DataPacket.dht_0, 0);
            }
            if(xActivatedMember == xDHT20_1_DataQueue) {
                xQueueReceive(xActivatedMember, &hkSensors_DataPacket.dht_1, 0);
            }
            // -------------

            // - DHT11 -----
            if(xActivatedMember == xDHT11_0_DataQueue) {
                xQueueReceive(xActivatedMember, &hkSensors_DataPacket.dht_2, 0);
            }
            // -------------

            // - DHT22 -----
            if(xActivatedMember == xDHT22_0_DataQueue) {
                xQueueReceive(xActivatedMember, &hkSensors_DataPacket.dht_3, 0);
            }
            // -------------

            // - SGP30 -----
            if(xActivatedMember == xSGP30_0_DataQueue) {
                xQueueReceive(xActivatedMember, &hkSensors_DataPacket.sgp30_0, 0);
            }
            // -------------

            // - PMS5003 ---
            if(xActivatedMember == xPMS5003_0_DataQueue) {
                xQueueReceive(xActivatedMember, &hkSensors_DataPacket.pms5003_0, 0);
            }
            // -------------

            // - DS18B20 ---
            if(xActivatedMember == xDS18B20_0_DataQueue) {
                xQueueReceive(xActivatedMember, &hkSensors_DataPacket.ds18b20_0, 0);
            }
            // -------------

            // if(xActivatedMember == xBMP180_0_DataQueue) {
            //     xQueueReceive(xActivatedMember, &hkSensors_DataPacket.bmp180_0, 0);
            // }
        }

        if((xTaskGetTickCount() - xLastSendTime) >= xSendInterval) {
            xQueueSend(xSnapshotQueue, &hkSensors_DataPacket, 0);
            xQueueSend(xMQTTQueue, &hkSensors_DataPacket, 0);
            xLastSendTime = xTaskGetTickCount();
        }
    }
}