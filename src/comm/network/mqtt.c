#include <comm/network/mqtt.h>


i32 MQTT_Init(MQTT_Config_t* config) {
    HTRACE("mqtt.c -> MQTT_Init(MQTT_Config_t*):void");
    HINFO("Initializing MQTT client...");

    static char clientID[32];
    hkUniqueID("pico", clientID, sizeof(clientID));

    config->mqttInstance = mqtt_client_new();
    if(!config->mqttInstance) {
        HFATAL("MQTT_Init(): Could not create an MQTT client!");
        return PICO_ERROR_CONNECT_FAILED;        
    }

    memset(&config->clientInfo, 0, sizeof(config->clientInfo));
    config->clientInfo.client_id = clientID;
    config->clientInfo.keep_alive = hkMQTT_KEEPALIVE;

    if(!ipaddr_aton(hkMQTT_BROKER_ADDR, &config->mqttServerAddress)) {
        HFATAL("MQTT_Init(): MQTT broker address invalid!");
        return PICO_ERROR_INVALID_ADDRESS;
    }

    config->mqttServerPort = hkMQTT_BROKER_PORT;
    config->topic = hkMQTT_TOPIC;

    HINFO("MQTT client has been initialized with ID: %s", clientID);
    return PICO_OK;
}