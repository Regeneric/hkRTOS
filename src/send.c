// FreeRTOS
#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <event_groups.h>

// hkRTOS
#include <defines.h>

#include <comm/network/mqtt.h>
#include <core/logger.h>
#include <sensors/sensors.h>


extern EventGroupHandle_t xMQTTStateEventGroup;
static void mqttClientConnectCallback(mqtt_client_t* client, void* arg, mqtt_connection_status_t status) {
    if(status == MQTT_CONNECT_ACCEPTED) {
        HINFO("MQTT client connected successfully.");
        xEventGroupSetBits(xMQTTStateEventGroup, BIT_MQTT_CONNECTED);
    } else {
        HERROR("mqttClientConnect(): MQTT connect failed (status = %d)", status);
        xEventGroupClearBits(xMQTTStateEventGroup, BIT_MQTT_CONNECTED);
    }
}

static void mqttPubRequestCallback(void* arg, err_t err) {
    char* data = (char*)arg;
    if(err != ERR_OK) HWARN("Publish to topic for %s has encountered an error %d", data, err);
}


extern EventGroupHandle_t xSystemStateEventGroup;
void vDataSendTask(void* pvParameters) {
    HTRACE("send.c -> RTOS:vDataSendTask(void*):void");
    
    xEventGroupWaitBits(
        xSystemStateEventGroup,
        BIT_MODE_NORMAL_OPERATION,
        pdFALSE,
        pdFALSE,
        portMAX_DELAY
    );

    char payload[512];
    MQTT_Config_t* params = (MQTT_Config_t*)pvParameters;
    Sensors_DataPacket_t hkSensors_DataPacket = {0};
    UBaseType_t coreID = portGET_CORE_ID();

    i32 status = MQTT_Init(params);
    if(status == PICO_ERROR_CONNECT_FAILED)  vTaskSuspend(NULL);
    if(status == PICO_ERROR_INVALID_ADDRESS) vTaskSuspend(NULL);

    err_t err = mqtt_client_connect(
        params->mqttInstance,
        &params->mqttServerAddress,
        params->mqttServerPort,
        mqttClientConnectCallback,
        NULL,
        &params->clientInfo
    );

    if(err != ERR_OK) {
        HERROR("vDataSendTask(): Connect error: %d\n", err);
        vTaskSuspend(NULL);
    }

    xEventGroupWaitBits(
        xMQTTStateEventGroup,
        BIT_MQTT_CONNECTED,
        pdFALSE,
        pdFALSE,
        portMAX_DELAY
    );


    while(FOREVER) {
        HTRACE("vDataSendTask(): Running on core {%d}", (u16)coreID);

        char* json = NULL;
        if(xQueueReceive(xMQTTQueue, &hkSensors_DataPacket, portMAX_DELAY) == pdPASS) {
            json = hkSensors_DataPacket.bme280_0.jsonify(&hkSensors_DataPacket.bme280_0);
            if(json) {
                strncpy(payload, json, (sizeof(payload) - 1));
                payload[sizeof(payload) - 1] = '\0';

                err = mqtt_publish(
                    params->mqttInstance,
                    params->topic,
                    payload, strlen(payload),
                    MQTT_QOS0, 0,
                    mqttPubRequestCallback, (void*)("BME280_0")
                ); 
                
                if(err == ERR_OK) HDEBUG("vDataSendTask(): BME280_0 - Data published to topic %s", params->topic);
                else HERROR("vDataSendTask(): BME280_0 - Data could not be published to topic %s (err = %d)", params->topic, err);

            } else HDEBUG("vDataSendTask(): BME280_0 - Data could not be published to topic %s", params->topic);

            json = hkSensors_DataPacket.bme280_1.jsonify(&hkSensors_DataPacket.bme280_1);
            if(json) {
                strncpy(payload, json, (sizeof(payload) - 1));
                payload[sizeof(payload) - 1] = '\0';

                err = mqtt_publish(
                    params->mqttInstance,
                    params->topic,
                    payload, strlen(payload),
                    MQTT_QOS0, 0,
                    mqttPubRequestCallback, (void*)("BME280_1")
                );

                if(err == ERR_OK) HDEBUG("vDataSendTask(): BME280_1 - Data published to topic %s", params->topic);
                else HERROR("vDataSendTask(): BME280_1 - Data could not be published to topic %s (err = %d)", params->topic, err);

            } else HDEBUG("vDataSendTask(): BME280_1 - Data could not be published to topic %s", params->topic);
        }   
    }
}