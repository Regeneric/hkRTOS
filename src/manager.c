#include <pico/cyw43_arch.h>

// FreeRTOS
#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <event_groups.h>

// hkRTOS
#include <defines.h>

#include <core/logger.h>
#include <comm/network/wifi.h>


extern EventGroupHandle_t xSystemStateEventGroup;
void vSystemManagerTask(__unused void* pvParameters) {
    HTRACE("manager.c -> RTOS:vSystemManagerTask(void*):void");

    UBaseType_t coreID = portGET_CORE_ID();
    HTRACE("vSystemManagerTask(): Running on core {%d}", (u16)coreID);

    HINFO("Checking for WiFi connection...");
    EventBits_t network = xEventGroupWaitBits(
        xSystemStateEventGroup,
        WIFI_CONNECTED_BIT,
        pdFALSE,
        pdFALSE,
        pdMS_TO_TICKS(30000)
    );

    if((network & WIFI_CONNECTED_BIT) != 0) {
        HINFO("WiFi Connected. Starting in NORMAL mode.");
        xEventGroupSetBits(xSystemStateEventGroup, BIT_MODE_NORMAL_OPERATION);
    } else {
        HWARN("vSystemManagerTask(): Failed to connect to WiFi. Starting in CONFIGURATION mode.");
        xEventGroupSetBits(xSystemStateEventGroup, BIT_MODE_CONFIG);
    }

    vTaskDelete(NULL);  // Single shot task
}