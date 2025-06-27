#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <event_groups.h>

#include <sensors/dht11_22/dht11_22.h>


extern EventGroupHandle_t xSystemStateEventGroup;
void vDHT_Task(void* pvParameters) {
    HTRACE("dht11_22.h -> RTOS:vDHT_Task(void*):void");

    xEventGroupWaitBits(
        xSystemStateEventGroup,      // The event group to wait on
        BIT_MODE_NORMAL_OPERATION,   // The bit to wait for
        pdFALSE,                     // Don't clear the bit on exit
        pdFALSE,                     // Wait for ALL bits (we only have one)
        portMAX_DELAY                // Wait forever
    );


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

        vTaskDelay(pdMS_TO_TICKS(2000));    // 2 Hz polling rate
    }
}