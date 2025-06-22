#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>

#include <sensors/dht11_22/dht11_22.h>


void vDHT_Task(void* pvParameters) {
    HTRACE("dht11_22.h -> RTOS:vDHT_Task(void*):void");

    DHT_TaskParams_t* params = (DHT_TaskParams_t*)pvParameters;
    UBaseType_t coreID = portGET_CORE_ID();

    DHT_Init(params->dht);
    vTaskDelay(pdMS_TO_TICKS(50));

    while(FOREVER) {
        HTRACE("vDHT_Task(): Running on core {%d}", (u16)coreID);

        DHT_Read(params->dht);
        if(xSemaphoreTake(params->dht->dmaSemaphore, pdMS_TO_TICKS(1000)) == pdTRUE) {
            DHT_ProcessData(params->dht, params->data);
            xQueueSend(params->dht->queue, params->data, 0);
        } else  HWARN("vDHT_Task(): Timed out waiting for DHT DMA completion.");

        vTaskDelay(pdMS_TO_TICKS(1000));    // 1 Hz polling rate
    }
}